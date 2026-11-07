#ifndef GAME_RENDER_UTILS_HPP
#define GAME_RENDER_UTILS_HPP
#include <raylib.h>
#include <bitset>
#include <vector>

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
                    DrawPixel(x + x_origin , y + y_origin, BLACK);
                }
            }
        }
    }
}

#endif