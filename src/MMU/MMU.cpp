#include "MMU.h"
#include <fstream>
#include <iostream>

MMU::MMU(Cartridge* cartridge) {
    this->cartridge = cartridge;
    for (int i = 0; i < 0xFFFF; i++) {
        memory[i] = 0;
    }
}

void MMU::load_game_rom(std::string location) {
    std::ifstream DMG_ROM(location, std::ios::binary);
    DMG_ROM.seekg(0, std::ios::end);
    long size = DMG_ROM.tellg();
    DMG_ROM.seekg(0, std::ios::beg);
    DMG_ROM.read((char *)memory, 0x100);
}

bool MMU::is_interrupt_enabled(uint8_t interruptFlag) {
    return (interrupt_enable & interruptFlag);
}
bool MMU::is_interrupt_flag_enabled(uint8_t interruptFlag) {
    return (interrupt_flags & interruptFlag);
}
void MMU::set_interrupt_flag(uint8_t interruptFlag) {
    interrupt_flags |= interruptFlag;
    write_byte(0xFF0F, interrupt_flags);
    return;
}
uint8_t MMU::read_byte(uint16_t address) {
    if (address == 0xFF04) {
        return DIV;
    }
    if (address == 0xFF05) {
        return TIMA;
    }
    if (address == 0xFF06) {
        return TMA;
    }
    if (address == 0xFF07) {
        return TAC;
    }
    
    if (address == 0xFF0F)
        return memory[0xFF0F];

    if (address < 0x100)
        return memory[address];

    // Switchable ROM banks
    if (address < 0x8000)
        return cartridge->MBC_read(address);

    // Switchable RAM banks
    if (address >= 0xA000 && address <= 0xBFFF)
        return cartridge->MBC_read(address);

    return memory[address];
}

void MMU::write_byte(uint16_t address, uint8_t value) {
    if (address == 0xFF40) {
        memory[address] = value;
        return;
    }
    if (address >= 0xFEA0 && address <= 0xFEFF) {
        return;
    }
    if (address == 0xFF46) {
        uint16_t source = value << 8;
        for (int i = 0; i < 160; i++) {
            write_byte(0xFE00 + i, read_byte((value << 8) + i));
        }
        return;
    }

    else if (address == 0xFF04)
        DIV = 0;
    else if (address == 0xFF05)
        TIMA = value;
    else if (address == 0xFF06)
        TMA = value;
    else if (address == 0xFF07)
        TAC = value;
    
    if (address < 0x8000) {
        cartridge->MBC_write(address, value);
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        cartridge->MBC_write(address, value);
    } else {
        memory[address] = value;
    }
    
    if (address >= 0x8000 && address <= 0x97FF) {
        updateTile(address, value);
    }
    return;
}

void MMU::updateTile(uint16_t addres, uint8_t value) {
    uint16_t address = addres & 0xFFFE;

    uint16_t tile = (addres >> 4) & 511;
    uint16_t y = (addres >> 1) & 7;

    uint8_t index;
    uint8_t x = 0;
    for (x; x < 8; x++) {
        index = 1 << (7 - x);
        tiles[tile].pixels[y][x] = ((memory[address] & index) ? 1 : 0) + ((memory[address + 1] & index) ? 2 : 0);
    }
}
void MMU::updateSprite(uint16_t addres, uint8_t value) {
    uint16_t address = addres - 0xFE00;
    Sprite *sprite = &sprites[address >> 2];
    sprite->ready = false;
    switch (address & 3) {
        case 0:
            sprite->y = value - 16;
            break;
        case 1:
            sprite->x = value - 8;
            break;
        case 2:
            sprite->tile = value;
            break;
        case 3:
            sprite->options.value = value;
            sprite->colourPalette = (sprite->options.paletteNumber == 0) ? palette_BGP : palette_OBP;
            sprite->ready = true;
            break;
    }
}