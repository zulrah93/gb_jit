#ifndef GAMEBOY_ROM_STRUCTS_HPP
#define GAMEBOY_ROM_STRUCTS_HPP

#include <vector>
#include <cstring>
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
        m_rom_buffer = new int8_t[MAX_GB_ROM_SIZE];
        memset(m_rom_buffer, 0, MAX_GB_ROM_SIZE);
        FILE* handle = fopen(path.c_str(), "rb");
        if (nullptr == handle) {
            m_rom_loaded = false;
            fclose(handle);
            return;
        }
        m_bytes_read = fread(m_rom_buffer, sizeof(uint8_t), MAX_GB_ROM_SIZE, handle);
        std::cout << "ROM is " << m_bytes_read << " byte(s)" << std::endl;
        m_rom_loaded = m_bytes_read > 0;
        fclose(handle);
    }

    ~gameboy_rom_t() {
        if (m_rom_buffer) {
            delete[] m_rom_buffer;
        }
    }

    bool has_rom_loaded() const {
        return m_rom_loaded;
    }

    operator bool() const {
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

    std::string get_rom_title() const {
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


    //Dangerous no copy is made
    const int8_t* get_rom_data() const {
        return m_rom_buffer;
    }

    const size_t get_rom_size() const {
        return m_bytes_read;
    }

private:
    bool m_rom_loaded{false};
    int8_t* m_rom_buffer{nullptr};
    size_t m_bytes_read;

};




#endif //GAMEBOY_ROM_STRUCTS_HPP