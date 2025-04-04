#include "MMU.h"
#include <fstream>
#include <iostream>

MMU::MMU(Cartridge* cartridge) {
    this->cartridge = cartridge;
}

void MMU::set_debug() {
    debug_mode = true;
}

bool MMU::is_interrupt_enabled(uint8_t interruptFlag) {
    return (this->read_byte(0xFFFF) & interruptFlag);
}
bool MMU::is_interrupt_flag_enabled(uint8_t interruptFlag) {
    return (this->read_byte(0xFF0F) & interruptFlag);
}
void MMU::set_interrupt_flag(uint8_t interruptFlag) {
    uint8_t interrupt_flag = this->read_byte(0xFF0F);
    interrupt_flag |= interruptFlag;
    write_byte(0xFF0F, interrupt_flag);
    return;
}
void MMU::unset_interrupt_flag(uint8_t interruptFlag) {
    uint8_t interrupt_flag = this->read_byte(0xFF0F);
    interrupt_flag &= ~interruptFlag;
    write_byte(0xFF0F, interrupt_flag);
    return;
}

uint8_t MMU::read_byte(uint16_t address) {
    if (debug_mode) {
        std::cout << "Reading from address: " << std::hex << address << std::endl;
    }

    switch (address) {
        case 0xFF04: {
            return DIV;
        } case 0xFF05: { 
            return TIMA;
        } case 0xFF06: {
            return TMA;
        } case 0xFF07: {
            return TAC;
        } case 0xFF0F: {
            return memory[0xFF0F];
        }
    }
    if (address < 0x100 && !rom_disabled) {
        return memory[address];
    }
    if (address < 0x8000 || (address >= 0xA000 && address <= 0xBFFF)) {
        return cartridge->MBC_read(address);
    }

    return memory[address];
}

void MMU::write_byte(uint16_t address, uint8_t value) {
    if (debug_mode) {
        std::cout << "Writing to address: " << std::hex << address << " value: " << (int)value << std::endl;
    }

    if (address == 0xFF40) {
        memory[address] = value;
        if (!(value & (1 << 7))) {
            memory[0xFF44] = 0x00;
            memory[0xFF41] &= 0x7C;
        }
        return;
    }
    if (address >= 0xFEA0 && address <= 0xFEFF) {
        std::cout << "Writing in unused area" << std::endl;
        return;
    }
    if (address == 0xFF46) {
        uint16_t source = value << 8;
        for (int i = 0; i < 160; i++) {
            write_byte(0xFE00 + i, read_byte((value << 8) + i));
        }
        return;
    }
    if (address == 0xFF50) {
        rom_disabled = true;
        return;
    }
    else if (address == 0xFF04) {
        DIV = 0;
    }
    else if (address == 0xFF05) {
        TIMA = value;
    }
    else if (address == 0xFF06) {
        TMA = value;
    }
    else if (address == 0xFF07) {
        TAC = value;
    }
    
    else if (address == 0xff47)
        updatePalette(palette_BGP, value);
    else if (address == 0xff48)
        updatePalette(palette_OBP0, value);
    else if (address == 0xff49)
        updatePalette(palette_OBP1, value);

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

    if (address >= 0xFE00 && address <= 0xFE9F) {
        updateSprite(address, value);
    }
}

void MMU::updateTile(uint16_t addres, uint8_t value) {
    uint16_t address = addres & 0xFFFE;

    uint16_t tile = (address >> 4) & 511;
    uint16_t y = (address >> 1) & 7;

    uint8_t index;
    for (uint8_t x = 0; x < 8; x++) {
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
            sprite->value = value;
            sprite->colourPalette = (sprite->paletteNumber == 0) ? palette_OBP1 : palette_OBP0;
            sprite->ready = true;
            break;
    }
}

void MMU::updatePalette(Colour *palette, uint8_t value) {
    palette[0] = palette_colours[value & 0x3];
    palette[1] = palette_colours[(value >> 2) & 0x3];
    palette[2] = palette_colours[(value >> 4) & 0x3];
    palette[3] = palette_colours[(value >> 6) & 0x3];
}

void MMU::info() {
    std::cout << "DIV: " << (int)DIV << " TIMA: " << (int)TIMA << " TMA: " << (int)TMA << " TAC: " << (int)TAC << std::endl;
}