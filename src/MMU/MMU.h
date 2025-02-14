#pragma once
#include "Cartridge/cartridge.h"

class MMU {
    public:
        Cartridge *cartridge;
        uint8_t memory[0xFFFF];
        uint8_t interrupt_enable = memory[0xFFFF]; // IE register
        uint8_t interrupt_flags = memory[0xFF0F];  // IF register


        MMU(Cartridge* cartridge);
        void load_game_rom(std::string ROM_location);
        uint8_t read_byte(uint16_t address);
        void write_byte(uint16_t address, uint8_t value);

    private:
};