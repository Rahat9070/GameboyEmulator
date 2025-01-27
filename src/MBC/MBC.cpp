#include "MBC.h"
#include <iostream>

uint8_t MBC0::read_byte(uint16_t address) {
    if (address < 0x8000) {
        return rom[address];
    }
    else {
        return 0;
    }
}
void MBC0::write_byte(uint16_t address, uint8_t value) {
    if (address >= 0xA000 && address <= 0xBFFF) {
        if (ram != nullptr) {
            ram[address - 0xA000] = value;
        } 
        else {
            std::cerr << "Write attempt to external RAM, but no RAM is present!" << std::endl;
        }
    } else {
        std::cerr << "Invalid write attempt to address: " << std::hex << address << std::endl;
    }
}

