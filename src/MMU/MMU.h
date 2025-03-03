#pragma once
#include "Cartridge/cartridge.h"
#include "structs.h"

class MMU {
    public:
        Cartridge *cartridge;
        uint8_t memory[0xFFFF];
        uint8_t interrupt_enable = memory[0xFFFF]; // IE register
        uint8_t interrupt_flags = memory[0xFF0F];  // IF register

        uint16_t DIV = 0;
        uint16_t TIMA = 0;
        uint16_t TMA = 0;
        uint16_t TAC = 0;

        uint8_t colours[4];

        Sprite sprites[40] = {Sprite()};
        
        Tile tiles[384];


        MMU(Cartridge* cartridge);
        void load_game_rom(std::string ROM_location);
        uint8_t read_byte(uint16_t address);
        void write_byte(uint16_t address, uint8_t value);
        void updateTile(uint16_t address, uint8_t value);
        void updateSprite(uint16_t address, uint8_t value);

    private:
};