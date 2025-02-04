#include "cartridge.h"

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
        delete[] memory;
        memory = nullptr;
        return;
    }

    banks_rom = size / 0x4000;
    switch (memory[0x149]) {
        case 0x00: {
            banks_ram = 0;
            break;
        }
        case 0x01:
            banks_ram = 0;
            break;
        case 0x02:
            banks_ram = 1;
            break;
        case 0x03:
            banks_ram = 4;
            break;
        case 0x04:
            banks_ram = 16;
            break;
        case 0x05:
            banks_ram = 8;
            break;
        default:
            std::cout << "Unknown RAM size " << memory[0x149] << std::endl;
            return;
    }

    uint8_t ram_size = memory[0x149];
    switch (ram_size) {
        case 0x00: // No RAM
            ram = nullptr;
            break;
        case 0x01: // 2 KB
            ram = new uint8_t[2 * 1024];
            break;
        case 0x02: // 8 KB
            ram = new uint8_t[8 * 1024];
            break;
        case 0x03: // 32 KB
            ram = new uint8_t[32 * 1024];
            break;
        case 0x04: // 128 KB
            ram = new uint8_t[128 * 1024];
            break;
        case 0x05: // 64 KB
            ram = new uint8_t[64 * 1024];
            break;
    }

    uint8_t mbc_type = memory[0x147];
    switch (mbc_type) {
        case 0x00:
        case 0x08:
        case 0x09:
            mbc = new MBC0(memory);
            break;
        case 0x01:
        case 0x02:
        case 0x03:
            mbc = new MBC1(memory, ram, banks_rom, banks_ram);
            break;
        case 0x05:
        case 0x06:
            mbc = new MBC2(memory, ram, banks_rom, banks_ram);
            break;
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            mbc = new MBC3(memory, ram, banks_rom, banks_ram);
            break;
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
            mbc = new MBC5(memory, ram, banks_rom, banks_ram);
            break;
        default:
            std::cerr << "Error: Unsupported MBC type: " << std::hex << (int)mbc_type << std::endl;
            return;
    }

}

uint8_t Cartridge::MBC_read(uint16_t address) {
    return mbc->read_byte(address);
}
void Cartridge::MBC_write(uint16_t address, uint8_t value) {
    mbc->write_byte(address);
}