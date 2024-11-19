#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <array>

class CPU 
{
public:
    CPU();

    uint8_t A, B, C, D, E, H, L, F; 
    uint16_t SP, PC;          

    // Memory: Simulating the Game Boy's address space (64 KB)
    std::array<uint8_t, 0x10000> memory;

    // Flags
    bool getZeroFlag();
    bool getSubtractFlag();
    bool getHalfCarryFlag();
    bool getCarryFlag();
    void setZeroFlag(bool value);
    void setSubtractFlag(bool value);
    void setHalfCarryFlag(bool value);
    void setCarryFlag(bool value);

    // CPU operations
    void reset();                   // Reset the CPU state
    void executeInstruction();      // Fetch-Decode-Execute cycle
    void handleInterrupts();        // Interrupt handling

private:
    void fetch();                   // Fetch an opcode
    void decodeAndExecute(uint8_t opcode); // Decode and execute
};

#endif // CPU_H
