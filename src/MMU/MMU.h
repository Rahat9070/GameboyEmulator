#pragma once
#include "Cartridge/cartridge.h"
#include "structs.h"

class MMU {
    public:
        Cartridge *cartridge;
        uint8_t memory[0xFFFF];
        uint8_t interrupt_enable = memory[0xFFFF];
        uint8_t interrupt_flags = memory[0xFF0F];

        uint16_t DIV = 0;
        uint16_t TIMA = 0;
        uint16_t TMA = 0;
        uint16_t TAC = 0;

        Sprite sprites[40] = {Sprite()};
        
        Tile tiles[384];

        static constexpr uint8_t VBLANK = (1 << 0);
        static constexpr uint8_t LCD = (1 << 1);
        static constexpr uint8_t TIMER = (1 << 2);
        static constexpr uint8_t SERIAL = (1 << 3);

        MMU(Cartridge* cartridge);
        void load_game_rom(std::string ROM_location);
        uint8_t read_byte(uint16_t address);
        void write_byte(uint16_t address, uint8_t value);
        void updateTile(uint16_t address, uint8_t value);
        void updateSprite(uint16_t address, uint8_t value);
        bool is_interrupt_enabled(uint8_t interruptFlag);
        bool is_interrupt_flag_enabled(uint8_t interruptFlag);
        void set_interrupt_flag(uint8_t interruptFlag);

        Colour *colour;
        const Colour palette_colours[4] = {
            {255, 255, 255, 255},
            {192, 192, 192, 255},
            {96, 96, 96, 255},
            {0, 0, 0, 255},
        };
        Colour palette_BGP[4] = {
            {255, 255, 255, 255},
            {0, 0, 0, 255},
            {0, 0, 0, 255},
            {0, 0, 0, 255},
        };
        Colour palette_OBP[4] = {
            {0, 0, 0, 255},
            {0, 0, 0, 255},
            {0, 0, 0, 255},
            {0, 0, 0, 255},
        };
    private:
};