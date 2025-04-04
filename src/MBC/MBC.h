#pragma once

#include <iostream>

class MBC {
    public:
        uint8_t *ram;
        uint8_t *rom;
        int banks_ram = 1;
        int banks_rom = 1;
        
        virtual uint8_t read_byte(uint16_t address) = 0;
        virtual void write_byte(uint16_t address, uint8_t value) = 0;
        void info();

        MBC(uint8_t *rom, uint8_t *ram);
        MBC(uint8_t *rom, uint8_t *ram, int banks_ram, int banks_rom);
};
class MBC0 : public MBC { 
    public:
        using MBC::MBC;
        uint8_t read_byte(uint16_t address);
        void write_byte(uint16_t address, uint8_t value);
};
class MBC1 : public MBC {
    public:
        uint8_t bank_rom = 1;
        uint8_t bank_ram = 0;
        bool is_ram_bank = false; // Starts in ROM bank
        bool is_ram_extended = false;
        using MBC::MBC;
        uint8_t read_byte(uint16_t address);
        void write_byte(uint16_t address, uint8_t value);
};
class MBC2 : public MBC {
    public:
        uint8_t bank_rom = 1;
        uint8_t bank_ram = 0;
        bool is_ram_bank = false;
        bool is_ram_extended = false;
        using MBC::MBC;
        uint8_t read_byte(uint16_t address);
        void write_byte(uint16_t address, uint8_t value);
};
class MBC3 : public MBC {
    public:
        uint8_t bank_rom = 1;
        uint8_t bank_ram = 0;
        bool is_ram_bank = false;
        bool is_ram_extended = false;
        using MBC::MBC;
        uint8_t read_byte(uint16_t address);
        void write_byte(uint16_t address, uint8_t value);
};
class MBC5 : public MBC {
    public:
        uint8_t bank_rom = 1;
        uint8_t bank_ram = 0;
        bool is_ram_bank = false;
        bool is_ram_extended = false;
        using MBC::MBC;
        uint8_t read_byte(uint16_t address);
        void write_byte(uint16_t address, uint8_t value);
};