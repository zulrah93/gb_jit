#ifndef GAMEBOY_CPU_GPU_STRUCTS_HPP
#define GAMEBOY_CPU_GPU_STRUCTS_HPP

#include <gameboy_rom_structures.hpp>
#include <gameboy_render_utils.hpp>
#include <functional>

struct gameboy_cpu_state_t {
    uint8_t register_a;
    uint8_t register_b;
    uint8_t register_c;
    uint8_t register_d;
    uint8_t register_e;
    uint8_t register_h;
    uint8_t register_l;
    uint8_t flag_register;
    uint16_t program_counter;
    uint16_t stack_pointer;
    int8_t* ram;
};

struct gameboy_save_state_t {
    gameboy_cpu_state_t cpu_state;
    int8_t* ram;
};

namespace flag_register_helper {
    void set_zero_on_flag_register(gameboy_cpu_state_t& state) {
        state.flag_register |= (1 << 8);
    }

    void unset_zero_on_flag_register(gameboy_cpu_state_t& state) {
        state.flag_register &= 0b01111111;
    }

    void set_negative_on_flag_register(gameboy_cpu_state_t& state) {
        state.flag_register |= (1 << 7);
    }

    void unset_negative_on_flag_register(gameboy_cpu_state_t& state) {
        state.flag_register &= 0b10111111;
    }

    void set_half_carry_on_flag_register(gameboy_cpu_state_t& state) {
        state.flag_register |= (1 << 6);
    }

    void unset_half_carry_on_flag_register(gameboy_cpu_state_t& state) {
        state.flag_register &= 0b11011111;
    }

    void set_carry_on_flag_register(gameboy_cpu_state_t& state) {
        state.flag_register |= (1 << 5);
    }

    void unset_carry_on_flag_register(gameboy_cpu_state_t& state) {
        state.flag_register &= 0b11101111;
    }
};

struct gameboy_opcodes_t {
    std::function<void(gameboy_cpu_state_t&)> opcodes[0xff];
};

class gameboy_t {
public:
    gameboy_t(const gameboy_rom_t& rom)  {
            if (rom) {
                m_ram = new int8_t[0xffffff];
               memset(m_ram, 0, sizeof(m_ram));
                m_cpu_state.ram = m_ram;
               memcpy(m_ram,  rom.get_rom_data(), rom.get_rom_size());
               initialize_opcodes();
            }
    }

    ~gameboy_t() {
        delete[] m_ram;
    }

    bool start_system() {
        m_cpu_state = {};
        m_cpu_state.program_counter = 0x100;
    }

    void on_update() {

    }
    void on_render() {

        size_t x = 0;
        size_t y = 0;
        palete_t palete{};
        palete.color1 = GB_BACKGROUND;
        palete.color2 = Color{0x49, 0x6b, 0x22, 0xff};
        palete.color3 = Color{0x0e, 0x45, 0x0b, 0xff};
        palete.color4 = Color{0x1b, 0x2a, 0x09, 0xff};
        size_t bytes_added{};
         std::vector<uint8_t> temp_bytes;
        for (size_t index = 0x18000; index <  0x23FFF; index++) {
            if (bytes_added == 16) {
                draw_gameboy_8_x_8_tile(temp_bytes, x, y, palete);
                x += 8;
                if (x >= 160) {
                    x = 0;
                    y += 8;
                }
                bytes_added = 0;
                temp_bytes.clear();
            }
            temp_bytes.push_back(m_ram[index]);
            bytes_added++;
        }

     /*   for (size_t index = 0x323F; index <  0x3E8E; index++) {
            if (bytes_added == 16) {
                draw_gameboy_8_x_8_tile(temp_bytes, x, y, palete);
                x += 8;
                if (x >= 160) {
                    x = 0;
                    y += 8;
                }
                bytes_added = 0;
                temp_bytes.clear();
            }
            temp_bytes.push_back(m_ram[index]);
            bytes_added++;
        }

        bytes_added = 0;
        temp_bytes.clear();

       for (size_t index = 0x55AC; index <  0x62AB; index++) {
            if (bytes_added == 16) {
                draw_gameboy_8_x_8_tile(temp_bytes, x, y, palete);
                x += 8;
                if (x >= 160) {
                    x = 0;
                    y += 8;
                }
                bytes_added = 0;
                temp_bytes.clear();
            }
            temp_bytes.push_back(m_ram[index]);
            bytes_added++;
        }*/
    }
private:
    void initialize_opcodes() {
        //Source: https://izik1.github.io/gbops/
        m_opcodes.opcodes[0x0] = [](gameboy_cpu_state_t& state) {
            //NOP
            state.program_counter++;
        };

        m_opcodes.opcodes[0x1] = [](gameboy_cpu_state_t& state) {
            int16_t u16 = *reinterpret_cast<int16_t*>(&state.ram[state.program_counter+1]);
            state.register_c = u16 & 0xff;
            state.register_b = (u16 >> 8) & 0xff;
            state.program_counter += 3;
        };

        m_opcodes.opcodes[0x2] = [](gameboy_cpu_state_t& state) {
            uint16_t address = (static_cast<uint16_t>(state.register_b) << 8) | state.register_c;
            state.ram[address] = state.register_a;
            state.program_counter++;
        };

        m_opcodes.opcodes[0x3] = [](gameboy_cpu_state_t& state) {
            uint16_t register_bc = (static_cast<uint16_t>(state.register_b) << 8) | state.register_c;
            register_bc++;
            state.register_c = register_bc & 0xff;
            state.register_b = (register_bc >> 8) & 0xff;
            state.program_counter++;
        };

        m_opcodes.opcodes[0x4] = [](gameboy_cpu_state_t& state) {
            state.register_b++;
            if (state.register_b == 0) {
                flag_register_helper::set_zero_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_zero_on_flag_register(state);
            }
            if (state.register_b > 0b1111) {
                flag_register_helper::set_half_carry_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_half_carry_on_flag_register(state);
            }
            flag_register_helper::unset_negative_on_flag_register(state);
            state.program_counter++;
        };

        m_opcodes.opcodes[0x5] = [](gameboy_cpu_state_t& state) {
            state.register_b--;
            if (state.register_b == 0) {
                flag_register_helper::set_zero_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_zero_on_flag_register(state);
            }
            if (state.register_b > 0b1111) {
                flag_register_helper::set_half_carry_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_half_carry_on_flag_register(state);
            }
            flag_register_helper::set_negative_on_flag_register(state);
            state.program_counter++;
        };

        m_opcodes.opcodes[0x6] = [](gameboy_cpu_state_t& state) {
            uint8_t u8 = state.ram[state.program_counter+1];
            state.register_b = u8;
            state.program_counter += 2;
        };

        m_opcodes.opcodes[0x7] = [](gameboy_cpu_state_t& state) {
            uint16_t register_a = state.register_a;
            register_a <<= 1;
            bool has_carry = register_a & 0b100000000;
            flag_register_helper::unset_zero_on_flag_register(state);
            flag_register_helper::unset_negative_on_flag_register(state);
            flag_register_helper::unset_carry_on_flag_register(state);
            flag_register_helper::unset_half_carry_on_flag_register(state);
            if (has_carry) {
                flag_register_helper::set_carry_on_flag_register(state);
            }
            state.register_a = static_cast<uint8_t>(register_a & 0xff);
            state.program_counter++;
        };

        m_opcodes.opcodes[0x8] = [](gameboy_cpu_state_t& state) {
            uint16_t u16 = (state.ram[state.program_counter+2] << 1) | state.ram[state.program_counter+1];
            state.ram[u16] = state.stack_pointer & 0xff;
            state.ram[u16+1] = (state.stack_pointer >> 8) & 0xff;
            state.program_counter += 3;
        };

        m_opcodes.opcodes[0x9] = [](gameboy_cpu_state_t& state) {
            uint32_t hl = (static_cast<uint16_t>(state.register_h) << 8)  | state.register_l;
            uint16_t bc = (static_cast<uint16_t>(state.register_b) << 8)  | state.register_c;
            hl = hl + bc;
            if (hl > 0xffff) {
                flag_register_helper::set_carry_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_carry_on_flag_register(state);
            }
            if (hl > 0xfff) {
                flag_register_helper::set_half_carry_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_half_carry_on_flag_register(state);
            }
            state.register_h = (hl >> 8) & 0xff;
            state.register_l = hl & 0xff;
            flag_register_helper::unset_negative_on_flag_register(state);
            state.program_counter++;
        };

        m_opcodes.opcodes[0xa] = [](gameboy_cpu_state_t& state) {
            uint16_t address = (static_cast<uint16_t>(state.register_b) << 8) | state.register_c;
            state.register_a = state.ram[address];
            state.program_counter++;
        };

        m_opcodes.opcodes[0xb] = [](gameboy_cpu_state_t& state) {
            uint16_t register_bc = (static_cast<uint16_t>(state.register_b) << 8) | state.register_c;
            register_bc--;
            state.register_b = (register_bc >> 8) & 0xff;
            state.register_c = register_bc & 0xff;
            state.program_counter++;
        };

        m_opcodes.opcodes[0xc] = [](gameboy_cpu_state_t& state) {
            state.register_c++;
            if (0 == state.register_c) {
                flag_register_helper::set_zero_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_zero_on_flag_register(state);
            }

            if (state.register_c > 0b1111) {
                flag_register_helper::set_half_carry_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_half_carry_on_flag_register(state);
            }

            flag_register_helper::unset_negative_on_flag_register(state);
            state.program_counter++;
        };

        m_opcodes.opcodes[0xd] = [](gameboy_cpu_state_t& state) {
            state.register_c--;
            if (0 == state.register_c) {
                flag_register_helper::set_zero_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_zero_on_flag_register(state);
            }

            if (state.register_c > 0b1111) {
                flag_register_helper::set_half_carry_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_half_carry_on_flag_register(state);
            }

            flag_register_helper::set_negative_on_flag_register(state);
            state.program_counter++;
        };

        m_opcodes.opcodes[0xe] = [](gameboy_cpu_state_t& state) {
            uint8_t u8 = state.ram[state.program_counter+1];
            state.register_c = u8;
            state.program_counter += 2;
        };
    }
    gameboy_cpu_state_t m_cpu_state;
    int8_t* m_ram;
    gameboy_opcodes_t m_opcodes;
};

#endif