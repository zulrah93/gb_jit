#include "raylib.h"
#include <iostream>
#include <gameboy_rom_structures.hpp>
#include <gameboy_render_utils.hpp>
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
            draw_nintendo_boot_logo(boot_logo_buffer, 50, 50);

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
