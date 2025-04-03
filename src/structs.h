#pragma once

#include <cstdint>

enum GBColour : uint8_t {
    GB_BLACK = 0,
    GB_DARK_GRAY = 1,
    GB_LIGHT_GRAY = 2,
    GB_WHITE = 3
};

struct Sprite {
    bool ready;
    int y;
    int x;
    uint8_t tile;
    GBColour* colourPalette;
    uint8_t paletteNumber : 1;
    uint8_t xFlip : 1;
    uint8_t yFlip : 1;
    uint8_t renderPriority : 1;
    uint8_t value;
};

struct Tile {
    uint8_t pixels[8][8] = {0};
};