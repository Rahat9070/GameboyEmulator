#include "MBC.h"

#include <iostream>

MBC::MBC(uint8_t *rom, uint8_t *ram) {
    this->rom = rom;
    this->ram = ram;
}
MBC::MBC(uint8_t *rom, uint8_t *ram, uint8_t *banks_ram, uint8_t *banks_rom) {
    this->rom = rom;
    this->ram = ram;
    this->banks_ram = banks_ram;
    this->banks_rom = banks_rom;
}

uint8_t MBC0::read_byte(uint16_t address) {
    if (address < 0x8000) {
        return rom[address];
    } else {
        std::cout << "Cannot read byte for MBC0" << std::endl;
        return 0xFF;
    }
}
void MBC0::write_byte(uint16_t address, uint8_t value) {
    if (address >= 0xA000 && address <= 0xBFFF) {
        if (ram != nullptr) {
            ram[address - 0xA000] = value;
        } else {
            std::cerr << "Write attempt to external RAM, but no RAM is present!" << std::endl;
        }
    } else {
        std::cerr << "Invalid write attempt to address: " << std::hex << address << std::endl;
    }
}

uint8_t MBC1::read_byte(uint16_t address) {
    if (address < 0x4000) {
        int bank = bank_mode * (bank_ram << 5) % banks_ram;
        return rom[bank * 0x4000 + address];
    } else if (address > 0x4000 && address <0x8000) {
        int bank = ((bank_ram << 5) | bank_rom) % banks_rom;
        return rom[bank * 0x4000 + address - 0x4000];
    } else if (address >= 0xA000 && address < 0xC000) {
        int bank = bank_mode * bank_ram % banks_ram;
        return ram[bank * 0x2000 + address - 0xA000];
    } else {
        std::cout << "Cannot read byte for MBC1" << std::endl;
        return 0xFF;
    }
}
void MBC1::write_byte(uint16_t address, uint8_t value) {
    if (address < 0x2000) {
        // If the lower 4 bits of the value are 0x0A, RAM is enabled; otherwise, it's disabled.
        is_ram_extended = (value & 0x0F) == 0x0A;
    } else if (address >= 0x2000 && address < 0x4000) {
        bank_rom = (bank_rom & 0xE0) | (value & 0x1F);
        if (bank_rom == 0) {
            bank_rom = 1;
        }
    } else if (address >= 0x4000 && address < 0x6000) {
        if (is_ram_bank) {
            bank_ram = value & 0x03;
        } else {
            bank_rom = (bank_rom & 0x1F) | ((value & 0x03) << 5);
            if (bank_rom == 0) {
                bank_rom = 1;
            }
        }
    } else if (address >= 0x6000 && address < 0x8000) {
        // Switch between ROM and RAM banking modes
        is_ram_bank = (value & 0x01) != 0;
    } else if (address >= 0xA000 && address < 0xC000) {
        if (is_ram_extended) {
            uint16_t ram_address = (bank_ram * 0x2000) + (address - 0xA000);
            if (ram_address < *banks_ram * 0x2000) {
                ram[ram_address] = value;
            }
        }
    } else {
        std::cerr << "Invalid write attempt to address: " << std::hex << address << std::endl;
    }
}

uint8_t MBC2::read_byte(uint8_t address) {
    if (address < 0x4000) {
        return rom[address];
    } else if (address > 0x4000 && address <0x8000) {
        return rom[rom_bank * 0x4000 + address - 0x4000];
    } else if (address >= 0xA000 && address < 0xC000) {
        if (is_ram_extended == true) {
            return ram[bank_ram * 0x2000 + address - 0xA000];
        }
    } else {
        std::cout << "Cannot read byte for MBC2" << std::endl;
        return 0xFF;
    }
}