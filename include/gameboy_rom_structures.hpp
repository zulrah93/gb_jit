#ifndef GAMEBOY_ROM_STRUCTS_HPP
#define GAMEBOY_ROM_STRUCTS_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <stdio.h>

//Source: https://gbdev.io/pandocs/The_Cartridge_Header.html
struct gameboy_rom_entry_point_t {
    uint32_t entry_point : 24;
};

using gameboy_manufacture_code_t = uint32_t;

constexpr size_t MAX_GB_ROM_SIZE{8388608};

class gameboy_rom_t {
public:
    gameboy_rom_t(const std::string path) {
        memset(m_rom_buffer, 0, sizeof(m_rom_buffer));
        FILE* handle = fopen(path.c_str(), "rb");
        if (nullptr == handle) {
            m_rom_loaded = false;
            fclose(handle);
            return;
        }
        m_bytes_read = fread(m_rom_buffer, sizeof(uint8_t), MAX_GB_ROM_SIZE, handle);
        m_rom_loaded = m_bytes_read > 0;
        fclose(handle);
    }

    bool has_rom_loaded() {
        return m_rom_loaded;
    }

    operator bool() {
        return has_rom_loaded();
    }

    void get_nintendo_boot_logo(std::vector<uint8_t>& boot_logo) {
        if (!m_rom_loaded) {
            return;
        }
        //Source: https://gbdev.io/pandocs/The_Cartridge_Header.html
        for(size_t offset{0x104}; offset <= 0x133; offset++) {
            boot_logo.push_back(m_rom_buffer[offset]);
        }
    }

    std::string get_rom_title() {
        if (!m_rom_loaded) {
            throw "Invalid rom!";
        }
        std::string title;
        for(size_t offset{0x134}; offset <= 0x143; offset++) {
            if (m_rom_buffer[offset] == '\0') {
                break;
            }
            title += m_rom_buffer[offset];
        }
        return title;
    }

private:
    bool m_rom_loaded{false};
    int8_t m_rom_buffer[MAX_GB_ROM_SIZE / 2];
    size_t m_bytes_read;

};




#endif //GAMEBOY_ROM_STRUCTS_HPP