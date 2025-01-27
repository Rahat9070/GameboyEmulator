#define MBC_H

#include <iostream>

class MBC {
    public:
        uint8_t *ram;
        uint8_t *rom;

    private:
        virtual uint8_t read_byte(uint16_t address) = 0;
        virtual void write_byte(uint16_t address) = 0;
}

class MBC0 : public MBC { 
    public:
    using MBC::MBC;
    uint8_t read_byte(uint16_t address);
    void write_byte(uint16_t address, uint8_t value) {};
}

class MBC1 : public MBC {

}