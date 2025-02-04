#pragma once

#include "MBC/MBC.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

class Cartridge {
    public:
        std::string name;

        MBC *mbc;
        uint8_t *memory;
        uint8_t *ram;

        int banks_rom;
        int banks_ram;

        Cartridge(std::string ROM_location);

        void load_game_rom(std::string ROM_location);

        uint8_t MBC_read(uint16_t address);
        void MBC_write(uint16_t address, uint8_t value);
}