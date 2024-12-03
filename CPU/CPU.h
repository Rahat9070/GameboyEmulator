#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <array>

class CPU 
{
public:
    CPU();

    bool halted = false;
    uint8_t A, B, C, D, E, H, L, F; 
    uint16_t SP, PC;  
    uint32_t cycles;     

    std::array<uint8_t, 0x10000> memory;

    bool getZeroFlag();
    bool getSubtractFlag();
    bool getHalfCarryFlag();
    bool getCarryFlag();
    void setZeroFlag(bool value);
    void setSubtractFlag(bool value);
    void setHalfCarryFlag(bool value);
    void setCarryFlag(bool value);

    // CPU operations
    void reset();     
    void executeInstruction();
    void handleInterrupts();

private:
    void fetch(); // opcode
    void decodeAndExecute(uint8_t opcode);
};

#endif
