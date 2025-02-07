#include "MMU.h"

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