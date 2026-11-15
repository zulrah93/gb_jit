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

struct gameboy_opcodes_t {
    std::function<void(gameboy_cpu_state_t&)> opcodes[0xff];
};

class gameboy_t {
public:
    gameboy_t(const gameboy_rom_t& rom)  {
            if (rom) {
                m_ram = new int8_t[0xffff];
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
private:
    void initialize_opcodes() {
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
            uint16_t address = (state.register_b << 8) | state.register_c;
            state.ram[address] = state.register_a;
            state.program_counter++;
        };
        m_opcodes.opcodes[0x3] = [](gameboy_cpu_state_t& state) {
            uint16_t register_bc = (state.register_b << 8) | state.register_c;
            register_bc++;
            state.register_c = register_bc & 0xff;
            state.register_b = (register_bc >> 8) & 0xff;
            state.program_counter++;
        }; // Missing flags status
        m_opcodes.opcodes[0x4] = [](gameboy_cpu_state_t& state) {
            state.register_b++;
            state.program_counter++;
        };
    }
    gameboy_cpu_state_t m_cpu_state;
    int8_t* m_ram;
    gameboy_opcodes_t m_opcodes;
};

#endif