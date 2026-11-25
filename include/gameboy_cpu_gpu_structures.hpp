#ifndef GAMEBOY_CPU_GPU_STRUCTS_HPP
#define GAMEBOY_CPU_GPU_STRUCTS_HPP

#include <gameboy_rom_structures.hpp>
#include <gameboy_render_utils.hpp>
#include <functional>

enum gameboy_flags_t : uint8_t {
    carry = 4,
    half_carry = 5,
    negative = 6,
    zero = 7
};

//Source: https://gbdev.io/pandocs/LCDC.html
struct gameboy_lcd_control_register_t {
    uint8_t lcd_enable : 1;
    uint8_t window_tile_map_area : 1;
    uint8_t enable_window : 1;
    uint8_t window_tile_data_area : 1;
    uint8_t background_tile_map_area : 1;
    uint8_t object_type : 1;
    uint8_t enable_objects : 1;
    uint8_t background_and_window_enable : 1;
};

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

static constexpr size_t OAM_START_ADDRESS{0xFE00};
static constexpr size_t OAM_END_ADDRESS{0xFE9F};
static constexpr size_t OAM_DMA_ADDRESS{0xFF46};
static constexpr size_t LCD_CONTROL_REGISTER{0xFF40};

class gameboy_t {
public:
    gameboy_t(const gameboy_rom_t& rom) : m_cpu_halted{true}, m_dma_register_hit{false}  {
            if (rom) {
                m_rom_title = rom.get_rom_title();
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
        m_cpu_state.program_counter = 0x0;
        m_cpu_halted = false;
    }

    void on_update() {
        while (!m_cpu_halted) {
          m_functors.opcodes[m_cpu_state.ram[m_cpu_state.program_counter]](m_cpu_state);
        }
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

        if (m_rom_title == "METROID2") {
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
        }

        if (m_rom_title == "TETRIS") {
            for (size_t index = 0x323F; index <  0x3E8E; index++) {
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
            }
        }
    }
private:
    void initialize_opcodes() {
        //Source: https://izik1.github.io/gbops/
        m_functors.opcodes[0x0] = [](gameboy_cpu_state_t& state) {
            //NOP
            state.program_counter++;
        };

        m_functors.opcodes[0x1] = [](gameboy_cpu_state_t& state) {
            uint16_t u16 = *reinterpret_cast<uint16_t*>(&state.ram[state.program_counter+1]);
            state.register_c = u16 & 0xff;
            state.register_b = (u16 >> 8) & 0xff;
            state.program_counter += 3;
        };

        m_functors.opcodes[0x2] = [](gameboy_cpu_state_t& state) {
            uint16_t address = (static_cast<uint16_t>(state.register_b) << 8) | state.register_c;
            state.ram[address] = state.register_a;
            state.program_counter++;
        };

        m_functors.opcodes[0x3] = [](gameboy_cpu_state_t& state) {
            uint16_t register_bc = (static_cast<uint16_t>(state.register_b) << 8) | state.register_c;
            register_bc++;
            state.register_c = register_bc & 0xff;
            state.register_b = (register_bc >> 8) & 0xff;
            state.program_counter++;
        };

        m_functors.opcodes[0x4] = [](gameboy_cpu_state_t& state) {
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

        m_functors.opcodes[0x5] = [](gameboy_cpu_state_t& state) {
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

        m_functors.opcodes[0x6] = [](gameboy_cpu_state_t& state) {
            uint8_t u8 = state.ram[state.program_counter+1];
            state.register_b = u8;
            state.program_counter += 2;
        };

        m_functors.opcodes[0x7] = [](gameboy_cpu_state_t& state) {
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

        m_functors.opcodes[0x8] = [](gameboy_cpu_state_t& state) {
            uint16_t u16 = (state.ram[state.program_counter+2] << 1) | state.ram[state.program_counter+1];
            state.ram[u16] = state.stack_pointer & 0xff;
            state.ram[u16+1] = (state.stack_pointer >> 8) & 0xff;
            state.program_counter += 3;
        };

        m_functors.opcodes[0x9] = [](gameboy_cpu_state_t& state) {
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

        m_functors.opcodes[0xa] = [](gameboy_cpu_state_t& state) {
            uint16_t address = (static_cast<uint16_t>(state.register_b) << 8) | state.register_c;
            state.register_a = state.ram[address];
            state.program_counter++;
        };

        m_functors.opcodes[0xb] = [](gameboy_cpu_state_t& state) {
            uint16_t register_bc = (static_cast<uint16_t>(state.register_b) << 8) | state.register_c;
            register_bc--;
            state.register_b = (register_bc >> 8) & 0xff;
            state.register_c = register_bc & 0xff;
            state.program_counter++;
        };

        m_functors.opcodes[0xc] = [](gameboy_cpu_state_t& state) {
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

        m_functors.opcodes[0xd] = [](gameboy_cpu_state_t& state) {
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

        m_functors.opcodes[0xe] = [](gameboy_cpu_state_t& state) {
            uint8_t u8 = state.ram[state.program_counter+1];
            state.register_c = u8;
            state.program_counter += 2;
        };

        m_functors.opcodes[0xf] = [](gameboy_cpu_state_t& state) {
            state.register_a >>= 1;
            if (state.register_a & 0b10000000) {
                flag_register_helper::set_carry_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_carry_on_flag_register(state);
            }
            flag_register_helper::unset_zero_on_flag_register(state);
            flag_register_helper::unset_half_carry_on_flag_register(state);
            flag_register_helper::unset_negative_on_flag_register(state);
            state.program_counter++;
        };

        m_functors.opcodes[0x10] = [](gameboy_cpu_state_t& state) {
            state.program_counter += 2; //NOTE: STOP instruction skips the next byte but does not care what value that byte is
        };

        m_functors.opcodes[0x11] = [](gameboy_cpu_state_t& state) {
            uint16_t u16 = *reinterpret_cast<uint16_t*>(&state.ram[state.program_counter+1]);
            state.register_e = u16 & 0xff;
            state.register_d = (u16 >> 8) & 0xff;
            state.program_counter += 3;
        };

        m_functors.opcodes[0x12] = [](gameboy_cpu_state_t& state) {
            uint16_t address = (static_cast<uint16_t>(state.register_d) << 8)  | state.register_e;
            state.ram[address] = state.register_a;
            state.program_counter++;
        };

        m_functors.opcodes[0x13] = [](gameboy_cpu_state_t& state) {
            uint16_t de = (static_cast<uint16_t>(state.register_d) << 8)  | state.register_e;
            de++;
            state.register_d  = (de >> 8) & 0xff;
            state.register_e = de & 0xff;
            state.program_counter++;
        };

        m_functors.opcodes[0x14] = [](gameboy_cpu_state_t& state) {
            state.register_d++;
            if (state.register_d == 0) {
                flag_register_helper::set_zero_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_zero_on_flag_register(state);
            }
            if (state.register_d > 0b1111) {
                flag_register_helper::set_half_carry_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_half_carry_on_flag_register(state);
            }
            flag_register_helper::unset_negative_on_flag_register(state);
            state.program_counter++;
        };

        m_functors.opcodes[0x15] = [](gameboy_cpu_state_t& state) {
            state.register_d--;
            if (state.register_d == 0) {
                flag_register_helper::set_zero_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_zero_on_flag_register(state);
            }
            if (state.register_d > 0b1111) {
                flag_register_helper::set_half_carry_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_half_carry_on_flag_register(state);
            }
            flag_register_helper::set_negative_on_flag_register(state);
            state.program_counter++;
        };

        m_functors.opcodes[0x16] = [](gameboy_cpu_state_t& state) {
            uint8_t u8 = state.ram[state.program_counter+1];
            state.register_d = u8;
            state.program_counter += 2;
        };

        m_functors.opcodes[0x17] = [](gameboy_cpu_state_t& state) {
              uint8_t previous_carry_value = (state.flag_register >> gameboy_flags_t::carry) & 1;
              uint8_t new_carry_value =  (state.register_a >> gameboy_flags_t::zero) & 1;
              if (1 == new_carry_value) {
                  flag_register_helper::set_carry_on_flag_register(state);
              }
              else {
                flag_register_helper::unset_carry_on_flag_register(state);
              }
              state.register_a <<= 1;
              state.register_a |= previous_carry_value;
              flag_register_helper::unset_zero_on_flag_register(state);
              flag_register_helper::unset_negative_on_flag_register(state);
              flag_register_helper::unset_half_carry_on_flag_register(state);
              state.program_counter++;
        };

        m_functors.opcodes[0x18] = [](gameboy_cpu_state_t& state) {
            uint8_t relative_offset = state.ram[state.program_counter+1];
            state.program_counter = state.program_counter+relative_offset+1;
        };

        m_functors.opcodes[0x19] = [](gameboy_cpu_state_t& state) {
            uint32_t hl = (static_cast<uint16_t>(state.register_h) << 8)  | state.register_l;
            uint16_t de = (static_cast<uint16_t>(state.register_d) << 8)  | state.register_e;
            hl = hl + de;
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

        m_functors.opcodes[0x1a] = [](gameboy_cpu_state_t& state) {
            uint16_t address = (static_cast<uint16_t>(state.register_d) << 8) | state.register_e;
            state.register_a = state.ram[address];
            state.program_counter++;
        };

        m_functors.opcodes[0x1b] = [](gameboy_cpu_state_t& state) {
            uint16_t register_de = (static_cast<uint16_t>(state.register_d) << 8) | state.register_e;
            register_de--;
            state.register_d = (register_de >> 8) & 0xff;
            state.register_e = register_de & 0xff;
            state.program_counter++;
        };

        m_functors.opcodes[0x1c] = [](gameboy_cpu_state_t& state) {
            state.register_e++;
            if (0 == state.register_e) {
                flag_register_helper::set_zero_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_zero_on_flag_register(state);
            }

            if (state.register_e > 0b1111) {
                flag_register_helper::set_half_carry_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_half_carry_on_flag_register(state);
            }

            flag_register_helper::unset_negative_on_flag_register(state);
            state.program_counter++;
        };

        m_functors.opcodes[0x1d] = [](gameboy_cpu_state_t& state) {
            state.register_e--;
            if (0 == state.register_e) {
                flag_register_helper::set_zero_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_zero_on_flag_register(state);
            }

            if (state.register_e > 0b1111) {
                flag_register_helper::set_half_carry_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_half_carry_on_flag_register(state);
            }

            flag_register_helper::set_negative_on_flag_register(state);
            state.program_counter++;
        };

        m_functors.opcodes[0x1e] = [](gameboy_cpu_state_t& state) {
            uint8_t u8 = state.ram[state.program_counter+1];
            state.register_e = u8;
            state.program_counter += 2;
        };

        m_functors.opcodes[0x1f] = [](gameboy_cpu_state_t& state) {
            uint8_t previous_carry_value = (state.flag_register >> gameboy_flags_t::carry) & 1;
            uint8_t new_carry_value =  state.register_a  & 1;
            if (1 == new_carry_value) {
                flag_register_helper::set_carry_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_carry_on_flag_register(state);
            }
            state.register_a >>= 1;
            state.register_a |= (previous_carry_value << 7);
            flag_register_helper::unset_zero_on_flag_register(state);
            flag_register_helper::unset_negative_on_flag_register(state);
            flag_register_helper::unset_half_carry_on_flag_register(state);
            state.program_counter++;
        };

        m_functors.opcodes[0x20] = [](gameboy_cpu_state_t& state) {
            bool is_zero = (state.flag_register >> gameboy_flags_t::zero) & 1;
            if (!is_zero) { //  NZ
                uint8_t relative_address = state.ram[state.program_counter+1];
                state.program_counter = state.program_counter + relative_address + 1;
            }
        };

        m_functors.opcodes[0x21] = [](gameboy_cpu_state_t& state) {
            uint16_t u16 = *reinterpret_cast<uint16_t*>(&state.ram[state.program_counter+1]);
            state.register_l = u16 & 0xff;
            state.register_h = (u16 >> 8) & 0xff;
            state.program_counter += 3;
        };

        m_functors.opcodes[0x22] = [](gameboy_cpu_state_t& state) {
            uint16_t address = (static_cast<uint16_t>(state.register_h) << 8) | state.register_l;
            state.ram[address] = state.register_a;
            const uint16_t hl = address+1;
            state.register_l = hl & 0xff;
            state.register_h = (hl >> 0x8) & 0xff;
            state.program_counter++;
        };

        m_functors.opcodes[0x23] = [](gameboy_cpu_state_t& state) {
            uint16_t hl = (static_cast<uint16_t>(state.register_h) << 8)  | state.register_l;
            hl++;
            state.register_l  = (hl >> 8) & 0xff;
            state.register_h = hl & 0xff;
            state.program_counter++;
        };

        m_functors.opcodes[0x24] = [](gameboy_cpu_state_t& state) {
            state.register_h++;
            if (state.register_h == 0) {
                flag_register_helper::set_zero_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_zero_on_flag_register(state);
            }
            if (state.register_h > 0b1111) {
                flag_register_helper::set_half_carry_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_half_carry_on_flag_register(state);
            }
            flag_register_helper::unset_negative_on_flag_register(state);
            state.program_counter++;
        };

        m_functors.opcodes[0x25] = [](gameboy_cpu_state_t& state) {
            state.register_h--;
            if (state.register_h == 0) {
                flag_register_helper::set_zero_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_zero_on_flag_register(state);
            }
            if (state.register_h > 0b1111) {
                flag_register_helper::set_half_carry_on_flag_register(state);
            }
            else {
                flag_register_helper::unset_half_carry_on_flag_register(state);
            }
            flag_register_helper::set_negative_on_flag_register(state);
            state.program_counter++;
        };

        m_functors.opcodes[0x26] = [](gameboy_cpu_state_t& state) {
            uint8_t u8 = state.ram[state.program_counter+1];
            state.register_h = u8;
            state.program_counter += 2;
        };

        m_functors.opcodes[0x27] = [](gameboy_cpu_state_t& state) {
            bool half_carry = ((state.flag_register >> gameboy_flags_t::half_carry) & 1) == 1;
            bool carry = ((state.flag_register >> gameboy_flags_t::carry) & 1) == 1;
            if ((state.register_a & 0xf) > 0x09 || half_carry) {
                state.register_a |= 0x06;
            }

            if ((state.register_a > 0x99) || carry){
                state.register_a |= 0x60;
            }
            state.program_counter++;
        };

        m_functors.opcodes[0x28] = [](gameboy_cpu_state_t& state) {
            bool is_zero = (state.flag_register >> gameboy_flags_t::zero) & 1;
            if (is_zero) { //  NZ
                uint8_t relative_address = state.ram[state.program_counter+1];
                state.program_counter = state.program_counter + relative_address + 1;
            }
        };
    }

    void oam_search() {
    }

    bool check_oam_dma_transfer() {
        if (m_dma_register_hit) {
            m_dma_register_hit = false;
            return true;
        }
    }

    void do_oam_dma_transfer() {
    }

    gameboy_lcd_control_register_t get_current_lcd_control_register() {
        return *reinterpret_cast<gameboy_lcd_control_register_t*>(&m_ram[LCD_CONTROL_REGISTER]);
    }

    void update_lcd_control_status_register(const gameboy_lcd_control_register_t& lcdc) {
        m_ram[LCD_CONTROL_REGISTER] = *reinterpret_cast<const uint8_t*>(&lcdc);
    }

    gameboy_cpu_state_t m_cpu_state;
    int8_t* m_ram;
    gameboy_opcodes_t m_functors;
    bool m_cpu_halted;
    std::string m_rom_title;
    bool m_dma_register_hit;
};

#endif