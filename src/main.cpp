#include "raylib.h"
#include <iostream>
#include <gameboy_cpu_structures.hpp>
#include <format>
#include <bitset>

static const uint32_t SCREEN_WIDTH{160};
static const uint32_t SCREEN_HEIGHT{144};

int main(int argument_count, char** arguments)
{
    if (argument_count > 1) {

        const char* rom_path = arguments[1];
        const std::string title_string = std::format("Gameboy JIT [{}]", rom_path);
        gameboy_rom_t rom(rom_path);
        std::vector<uint8_t> boot_logo_buffer;
        if (!rom) {
            std::cout << "Invalid rom @ " << rom_path << std::endl;
        }
        rom.get_nintendo_boot_logo(boot_logo_buffer);
        std::cout << "Nintendo Boot Logo Bytes: " << std::endl;
        for (auto& byte : boot_logo_buffer) {
            std::cout << std::hex << static_cast<uint16_t>(byte) << std::dec << " ";
        }
        std::cout << std::endl;
        InitWindow(SCREEN_WIDTH * 4, SCREEN_HEIGHT * 4, title_string.c_str());
        RenderTexture2D raster_buffer = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
        SetTargetFPS(60);
        std::string initial_text{"Loading Rom:\n"};
        initial_text += rom_path;
        while (!WindowShouldClose())
        {
            BeginTextureMode(raster_buffer);

            ClearBackground(WHITE);
            DrawText(rom.get_rom_title().c_str(), 0, 0, 5, BLACK);

            int32_t x_origin{SCREEN_WIDTH / 2};
            int32_t y_origin{SCREEN_HEIGHT / 2};
            for (int index = 0; index < boot_logo_buffer.size(); index++) {
                int32_t half = (index < 24) ? 0 : 1;
                int32_t block = index % 12;
                for (int32_t row{}; row < 4; row++) {
                    uint8_t byte = boot_logo_buffer[half * 24 + block * 2 + row / 2];
                    uint8_t nibble = (row % 2 == 0) ? (byte >> 4) : (byte & 0x0F);
                    for (int bit = 0; bit < 4; bit++) // MSB of nibble = leftmost pixel
                    {
                        int32_t x = block * 4 + bit;
                        int32_t y = half * 4 + row;
                        if ((nibble >> (3 - bit)) & 1) {
                            DrawPixel(x + x_origin , y + y_origin, BLACK);
                        }
                    }
                }
            }
            /*for (int half = 0; half < 2; half++)          // 0 = top rows 0-3, 1 = bottom rows 4-7
            {
                for (int block = 0; block < 12; block++)  // 12 blocks of 4 pixels across
                {
                    for (int row = 0; row < 4; row++)     // 4 rows per block
                    {
                        // 2 bytes per block, 2 nibbles (rows) per byte
                        uint8_t byte = boot_logo_buffer[half * 24 + block * 2 + row / 2];
                        uint8_t nibble = (row % 2 == 0) ? (byte >> 4) : (byte & 0x0F);

                        for (int bit = 0; bit < 4; bit++) // MSB of nibble = leftmost pixel
                        {
                            int x = block * 4 + bit;
                            int y = half * 4 + row;
                            if ((nibble >> (3 - bit)) & 1) {
                                DrawPixel(x, y, BLACK);
                            }
                        }
                    }
                }
            }*/


            EndTextureMode();

            BeginDrawing();
            ClearBackground(RAYWHITE);
            Rectangle source{0, 0, static_cast<float>(raster_buffer.texture.width), static_cast<float>(-raster_buffer.texture.height)};
            Rectangle destination{0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
            Vector2 origin{0, 0};
            DrawTexturePro(raster_buffer.texture, source, destination, origin , 0.0, WHITE);
            EndDrawing();
        }

        UnloadRenderTexture(raster_buffer);
        CloseWindow();

    }

    return 0;
}
