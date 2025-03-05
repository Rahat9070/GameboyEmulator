#include "MMU.h"
#include <fstream>
#include <iostream>

MMU::MMU(Cartridge* cartridge) {
    this->cartridge = cartridge;
}

void MMU::load_game_rom(std::string ROM_location) {
    std::ifstream DMG_ROM(ROM_location, std::ios::binary);
    DMG_ROM.seekg(0, std::ios::end);
    long size = DMG_ROM.tellg();
    DMG_ROM.seekg(0, std::ios::beg);
    DMG_ROM.read((char *)memory, 0x100);
}

uint8_t MMU::read_byte(uint16_t address) {
    if (address == 0xff00) {
        switch (memory[0xff00] & 0x30) {  // Mask `00110000` to check which SELECT
            default:
                return 0xFF;
        }
    }
    if (address == 0xff0f)
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

void MMU::updateTile(uint16_t address, uint8_t value) {
    uint16_t addres = address & 0xFFFE;

    uint16_t tile = (address >> 4) & 511;
    uint16_t y = (address >> 1) & 7;

    uint8_t index;
    uint8_t x = 0;
    for (x; x < 8; x++) {
        index = 1 << (7 - x);
        tiles[tile].pixels[y][x] = ((memory[addres] & index) ? 1 : 0) + ((memory[addres + 1] & index) ? 2 : 0);
    }
}
void MMU::updateSprite(uint16_t address, uint8_t value) {
}

bool MMU::is_interrupt_enabled(uint8_t interruptFlag) {
    return (interrupt_enable & interruptFlag);
}
bool MMU::is_interrupt_flag_enabled(uint8_t interruptFlag) {
    return (interrupt_flags & interruptFlag);
}
