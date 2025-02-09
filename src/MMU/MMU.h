#pragma once
#include "Cartridge/cartridge.h"

class MMU {
    public:
        Cartridge *cartridge;
        uint8_t memory[0x10000];


        MMU(Cartridge* cartridge);
        void load_game_rom(std::string ROM_location);
        uint8_t read_byte(uint16_t address);
        void write_byte(uint16_t address, uint8_t value);

    private:
};