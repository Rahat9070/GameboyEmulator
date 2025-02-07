#pragma once
#include "src/Cartridge/cartridge.cpp"

class MMU {
    public:
        Cartridge *cartridge;
        MMU(Cartridge* cartridge);

        void load_game_rom(std::string ROM_location);

    private:
        uint8_t memory[0x10000];
};