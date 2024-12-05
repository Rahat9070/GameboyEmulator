#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <array>

class CPU 
{
public:
    CPU();

    bool halted = false;
    uint8_t A, B, C, D, E, H, L, F; // Registers
    uint16_t SP, PC;  // Stack Pointer & Program Counter
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
    
    void decodeAndExecute(uint8_t opcode);


private:
    void fetch(); // opcode
    void executeCBInstruction(uint8_t cb_opcode);
    void incrementFlags(uint8_t register1);
    void decrementFlags(uint8_t register1);
    void additionFlags(uint8_t register1, uint8_t value, uint16_t result);
    void subtractionFlags(uint8_t register1, uint8_t value, uint16_t result);
    void andFlags(uint8_t result);
    void orFlags(uint8_t result);
};

#endif
