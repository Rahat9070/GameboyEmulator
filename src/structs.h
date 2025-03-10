#pragma once

#include <cstdint>

struct Colour {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
    uint8_t colours[4];
}; 

struct Sprite {
    bool ready;
    int y;
    int x;
    uint8_t tile;
    Colour *colourPalette;
    struct {
        union {
            struct {
                uint8_t gbcPaletteNumber1 : 1;
                uint8_t gbcPaletteNumber2 : 1;
                uint8_t gbcPaletteNumber3 : 1;
                uint8_t gbcVRAMBank : 1;
                uint8_t paletteNumber : 1;
                uint8_t xFlip : 1;
                uint8_t yFlip : 1;
                uint8_t renderPriority : 1;
            };
            uint8_t value;
        };
    } options;
};

struct Tile {
    uint8_t pixels[8][8] = {0};
};