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
