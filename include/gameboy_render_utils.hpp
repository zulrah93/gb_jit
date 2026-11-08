#ifndef GAME_RENDER_UTILS_HPP
#define GAME_RENDER_UTILS_HPP
#include <raylib.h>
#include <bitset>
#include <vector>

static constexpr Color GB_BACKGROUND =  (Color){155, 188, 15, 255};
static constexpr Color GB_FOREGROUND =  (Color){15, 56, 15, 255};


bool draw_gameboy_8_x_8_tile(const std::vector<uint8_t>& tile_data, const int32_t x, const int32_t y) {
    if (tile_data.size() != 16) {
        return false;
    }
    size_t row{};
    for (size_t index{}; index < tile_data.size(); index += 2) {
        const uint8_t tile = tile_data[index] | tile_data[index+1]; // Or is it boolean or?
        std::bitset<8> tile_bits{tile};
        for (size_t bit_index{}; bit_index < tile_bits.size(); bit_index++) {
            const int32_t bit_x = bit_index;
            const int32_t bit_y = static_cast<int32_t>(row);
            if (tile_bits[tile_bits.size() - bit_index - 1]) {
                DrawPixel(x + bit_x, y + bit_y, GB_FOREGROUND);
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