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
        std::vector<uint8_t> boot_logo;
        if (!rom) {
            std::cout << "Invalid rom @ " << rom_path << std::endl;
        }
        rom.get_nintendo_boot_logo(boot_logo);
        std::cout << "Nintendo Boot Logo Bytes: " << std::endl;
        for (auto& byte : boot_logo) {
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
            int32_t x{5}, y{10};
            for(size_t top_half_index{}; top_half_index < 24; top_half_index++) {
                std::bitset<8> current_byte{boot_logo[top_half_index]};
                for(size_t bit_index{}; bit_index < current_byte.count(); bit_index++) {
                    if (!current_byte[current_byte.count() - bit_index - 1]) {
                        DrawPixel(x + (bit_index), y + ((bit_index < 4) ? 0 : 1), BLACK);
                    }
                }
            }
            for(size_t bottom_half_index{}; bottom_half_index < 24; bottom_half_index++) {
                std::bitset<8> current_byte{boot_logo[bottom_half_index]};
                for(size_t bit_index{}; bit_index < current_byte.count(); bit_index++) {
                    if (!current_byte[current_byte.count() - bit_index - 1]) {
                        DrawPixel(x + (bit_index), y + 1 + ((bit_index < 4) ? 0 : 1), BLACK);
                    }
                }
            }

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
