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
static constexpr size_t LCD_Y_REGISTER{0xFF44};
static constexpr size_t LCD_Y_COMPARE_REGISTER{0xFF45};
static constexpr size_t LCD_STATUS_REGISTER{0xFF41};
static constexpr size_t SCX_REGISTER{0xFF42};
static constexpr size_t SCY_REGISTER{0xFF43};

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

//Source: https://gbdev.io/pandocs/STAT.html#ff44--ly-lcd-y-coordinate-read-only
struct gameboy_lcd_status_register_t {
    uint8_t current_ppu_mode : 2;
    uint8_t ly_equals_lyc : 1;
    uint8_t mode0_interrupt : 1;
    uint8_t mode1_interrupt : 1;
    uint8_t mode2_interrupt : 1;
    uint8_t lyc_interrupt : 1;
    uint8_t reserved : 1;
};

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
    void initialize_opcodes();

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

    uint8_t get_scx_register() {
        return m_ram[SCX_REGISTER];
    }

    uint8_t get_scy_register() {
        return m_ram[SCY_REGISTER];
    }

    gameboy_lcd_control_register_t get_current_lcd_control_register() {
        return *reinterpret_cast<gameboy_lcd_control_register_t*>(&m_ram[LCD_CONTROL_REGISTER]);
    }

    void update_lcd_control_register(const gameboy_lcd_control_register_t& lcdc) {
        m_ram[LCD_CONTROL_REGISTER] = *reinterpret_cast<const uint8_t*>(&lcdc);
    }

    gameboy_lcd_status_register_t get_current_lcd_status_register() {
        return *reinterpret_cast<gameboy_lcd_status_register_t*>(&m_ram[LCD_STATUS_REGISTER]);
    }

    void update_lcd_status_register(const gameboy_lcd_status_register_t& lcd_stat) {
        m_ram[LCD_STATUS_REGISTER] = *reinterpret_cast<const uint8_t*>(&lcd_stat);
    }

    gameboy_cpu_state_t m_cpu_state;
    int8_t* m_ram;
    gameboy_opcodes_t m_functors;
    bool m_cpu_halted;
    std::string m_rom_title;
    bool m_dma_register_hit;
};

#include <opcodes.hpp>

#endif