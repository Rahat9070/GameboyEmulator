#pragma once

#include "MMU/MMU.h"
#include <cstdint>
#include <array>
#include <iostream>

class CPU 
{
    public:
        CPU(MMU& mmu);

        bool halted, IME;
        uint8_t A, B, C, D, E, H, L, F; // Registers
        uint16_t SP, PC;  // Stack Pointer & Program Counter
        uint32_t cycles;     

        MMU* mmu;
        uint8_t* memory;

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
        void fetch(); // opcode
        void executeCBInstruction(uint8_t cb_opcode);


    private:
        void incrementFlags(uint8_t register1);
        void decrementFlags(uint8_t register1);
        void additionFlags(uint8_t register1, uint8_t value, uint16_t result);
        void subtractionFlags(uint8_t register1, uint8_t value, uint16_t result);
        void andFlags(uint8_t result);
        void orFlags(uint8_t result);
        void RLC(uint8_t& register1);
        void RRC(uint8_t& register1);
        void RL(uint8_t& register1);
        void RR(uint8_t& register1);
        void SLA(uint8_t& register1);
        void SRA(uint8_t& register1);
};
