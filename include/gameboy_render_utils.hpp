#ifndef GAME_RENDER_UTILS_HPP
#define GAME_RENDER_UTILS_HPP
#include <raylib.h>
#include <bitset>
#include <vector>

static constexpr Color GB_BACKGROUND =  (Color){155, 188, 15, 255};
static constexpr Color GB_FOREGROUND =  (Color){15, 56, 15, 255};

struct pallete_t {
    Color color1;
    Color color2;
    Color color3;
    Color color4;
};

bool draw_gameboy_8_x_8_tile(const std::vector<uint8_t>& tile_data, const int32_t x, const int32_t y, pallete_t pallete) {
    if (tile_data.size() != 16) {
        return false;
    }
    size_t row{};
    for (size_t index{}; index < tile_data.size(); index += 2) {
        uint8_t tile_left = tile_data[index];
        uint8_t tile_right = tile_data[index+1];
        for (int32_t bit_index = 7; bit_index >= 0; bit_index--) {
           int32_t left_bit = ((tile_left & (1 << bit_index)) >> bit_index);
            int32_t right_bit = ((tile_right & (1 << bit_index)) >> bit_index) << 1;
            const int32_t bit_x = (8 - bit_index - 1);
            const int32_t bit_y = static_cast<int32_t>(row);

            switch (left_bit | right_bit) {
                case 0: {
                    DrawPixel(x + bit_x, y + bit_y, pallete.color1);
                    break;
                }
                case 1: {
                    DrawPixel(x + bit_x, y + bit_y, pallete.color2);
                    break;
                }
                case 2: {
                    DrawPixel(x + bit_x, y + bit_y, pallete.color3);
                    break;
                }
                case 3: {
                    DrawPixel(x + bit_x, y + bit_y, pallete.color4);
                    break;
                }
            }
        }
        row++;
    }
    return true;
}

void draw_nintendo_boot_logo(const std::vector<uint8_t>& boot_logo_buffer, int32_t x_origin, int32_t y_origin) {

    for (int index = 0; index < boot_logo_buffer.size(); index++) {
        int32_t half = (index < 24) ? 0 : 1;
        int32_t block = index % 12;
        for (int32_t row{}; row < 4; row++) {
            uint8_t byte = boot_logo_buffer[half * 24 + block * 2 + row / 2];
            uint8_t nibble = (row % 2 == 0) ? (byte >> 4) : (byte & 0x0F);
            std::bitset<8> bits{nibble};
            for (int bit = 0; bit < 4; bit++) // MSB of nibble = leftmost pixel
            {
                int32_t x = block * 4 + bit;
                int32_t y = half * 4 + row;
                if (bits[3 - bit]) {
                    DrawPixel(x + x_origin , y + y_origin, GB_FOREGROUND);
                }
            }
        }
    }
}

#endif