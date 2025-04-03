#pragma once

#include <cstdint>

struct Colour {
    union {
        struct {
            uint8_t r, g, b, a;
        };
        uint8_t colours[4];
    };
};

struct Sprite {
    bool ready;
    int y;
    int x;
    uint8_t tile;
    Colour *colourPalette;
    uint8_t paletteNumber : 1;
    uint8_t xFlip : 1;
    uint8_t yFlip : 1;
    uint8_t renderPriority : 1;
    uint8_t value;
};

struct Tile {
    uint8_t pixels[8][8] = {0};
};