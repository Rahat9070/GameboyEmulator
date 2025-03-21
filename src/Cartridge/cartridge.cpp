#include "cartridge.h"
#include <iostream>

Cartridge::Cartridge(std::string ROM_location) {
    load_game_rom(ROM_location);
}

void Cartridge::load_game_rom(std::string ROM_location) {
    std::ifstream ROM_FILE(ROM_location, std::ios::binary | std::ios::ate);
    if (!ROM_FILE.is_open()) {
        std::cout << "Unable to open ROM file: " << ROM_location << std::endl;
        return;
    }

    std::streamsize size = ROM_FILE.tellg();
    ROM_FILE.seekg(0, std::ios::beg);
    memory = new uint8_t[size];
    if (!memory) {
        std::cerr << "Error: Could not allocate memory for ROM data." << std::endl;
        return;
    }

    if (!ROM_FILE.read((char*)memory, size)) {
        std::cerr << "Error: Could not read ROM data." << std::endl;
        return;
    }

    banks_rom = size / 0x4000;
    switch (memory[0x149]) {
        case 0x00: {
            banks_ram = 0;
            break;
        } case 0x01: {
            banks_ram = 0;
            break;
        } case 0x02: {
            banks_ram = 1;
            break;
        } case 0x03: {
            banks_ram = 4;
            break;
        } case 0x04: {
            banks_ram = 16;
            break;
        } case 0x05: {
            banks_ram = 8;
            break;
        } default: {
            std::cout << "Unknown RAM size " << memory[0x149] << std::endl;
            return;
        }
    }

    uint8_t ram_size = memory[0x149];
    switch (ram_size) {
        case 0x00: { // No RAM
            ram = nullptr;
            break;
        } case 0x01: { // 2 KB
            ram = new uint8_t[2 * 1024];
            break;
        } case 0x02: { // 8 KB
            ram = new uint8_t[8 * 1024];
            break;
        } case 0x03: { // 32 KB
            ram = new uint8_t[32 * 1024];
            break;
        } case 0x04: { // 128 KB
            ram = new uint8_t[128 * 1024];
            break;
        } case 0x05: { // 64 KB
            ram = new uint8_t[64 * 1024];
            break;
        }
    }

    uint8_t mbc_type = memory[0x147];
    switch (mbc_type) {
        case 0x00: {
            mbc = new MBC0(memory, ram);
            break;
        } case 0x01: {
            mbc = new MBC1(memory, ram, banks_rom, banks_ram);
            break;
        } case 0x02: {
            mbc = new MBC1(memory, ram, banks_rom, banks_ram);
            break;
        } case 0x03: {
            mbc = new MBC1(memory, ram, banks_rom, banks_ram);
            break;
        } case 0x05: {
            mbc = new MBC2(memory, ram, banks_rom, banks_ram);
            break;
        } case 0x06: {
            mbc = new MBC2(memory, ram, banks_rom, banks_ram);
            break;
        } case 0x08: {
            mbc = new MBC0(memory, ram);
            break;
        } case 0x09: {
            mbc = new MBC0(memory, ram);
            break;
        } case 0x0F: {
            mbc = new MBC3(memory, ram, banks_rom, banks_ram);
            break;
        } case 0x10: {
            mbc = new MBC3(memory, ram, banks_rom, banks_ram);
            break;
        } case 0x11: {
            mbc = new MBC3(memory, ram, banks_rom, banks_ram);
            break;
        } case 0x12: {
            mbc = new MBC3(memory, ram, banks_rom, banks_ram);
            break;
        } case 0x13: {
            mbc = new MBC3(memory, ram, banks_rom, banks_ram);
            break;
        } case 0x19: {
            mbc = new MBC5(memory, ram, banks_rom, banks_ram);
            break;
        } case 0x1A: {
            mbc = new MBC5(memory, ram, banks_rom, banks_ram);
            break;
        } case 0x1B: {
            mbc = new MBC5(memory, ram, banks_rom, banks_ram);
            break;
        } case 0x1C: {
            mbc = new MBC5(memory, ram, banks_rom, banks_ram);
            break;
        } case 0x1D: {
            mbc = new MBC5(memory, ram, banks_rom, banks_ram);
            break;
        } case 0x1E: {
            mbc = new MBC5(memory, ram, banks_rom, banks_ram);
            break;
        } default: {
            std::cerr << "Error: Unsupported MBC type: " << std::hex << (int)mbc_type << std::endl;
            return;
        }
    }
    info();
}

uint8_t Cartridge::MBC_read(uint16_t address) {
    return mbc->read_byte(address);
}
void Cartridge::MBC_write(uint16_t address, uint8_t value) {
    mbc->write_byte(address, value);
}

void Cartridge::info() {
    std::string rom_title = std::string(memory + 0x134, memory + 0x143);
    std::cout << "Rom Title: " << rom_title << std::endl;
    std::cout << "MBC: " << mbc << std::endl;
    std::cout << "ROM Banks: " << banks_rom << std::endl;
    std::cout << "RAM Banks: " << banks_ram << std::endl;
}