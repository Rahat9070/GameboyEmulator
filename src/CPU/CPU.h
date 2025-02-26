#pragma once

#include "MMU/MMU.h"
#include "Scheduler/scheduler.h"
#include <cstdint>
#include <array>
#include <iostream>

class CPU 
{
    public:
        CPU(MMU& mmu, Scheduler& scheduler);

        const uint8_t instructionCycles[256] = {
            4, 12, 8, 8, 4, 4, 8, 4, 20, 8, 8, 8, 4, 4, 8, 4, // 0x00 - 0x0F
            4, 12, 8, 8, 4, 4, 8, 4, 12, 8, 8, 8, 4, 4, 8, 4, // 0x10 - 0x1F
            8, 12, 8, 8, 4, 4, 8, 4, 12, 8, 8, 8, 4, 4, 8, 4, // 0x20 - 0x2F
            8, 12, 8, 8, 12, 12, 12, 4, 12, 8, 8, 8, 4, 4, 8, 4, // 0x30 - 0x3F
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 0x40 - 0x4F
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 0x50 - 0x5F
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 0x60 - 0x6F
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 0x70 - 0x7F
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 0x80 - 0x8F
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 0x90 - 0x9F
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 0xA0 - 0xAF
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 0xB0 - 0xBF
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 0xC0 - 0xCF
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 0xD0 - 0xDF
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, // 0xE0 - 0xEF
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4  // 0xF0 - 0xFF
        };
        const uint8_t CBinstructionCycles[256] = {
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 0x00 - 0x0F
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 0x10 - 0x1F
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 0x20 - 0x2F
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 0x30 - 0x3F
            8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,  // 0x40 - 0x4F
            8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,  // 0x50 - 0x5F
            8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,  // 0x6_
            8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,  // 0x7_
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 0x8_
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 0x9_
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 0xa_
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 0xb_
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 0xc_
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 0xd_
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,  // 0xe_
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8   // 0xf_
        };
        bool halted, IME;
        uint8_t A, B, C, D, E, H, L, F; // Registers
        uint16_t SP, PC;  // Stack Pointer & Program Counter
        int timer_cycles, divider_cycles; // Timer & Divider Cycles

        MMU* mmu;
        Scheduler* scheduler;
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
        void executeInstruction(uint8_t opcode);
        void handleInterrupts();

        void fetch(); // opcode
        void decodeAndExecute(uint8_t opcode);
        void executeCBInstruction(uint8_t cb_opcode);
        void step();


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
        void SWAP(uint8_t& register1);
        void SRL(uint8_t& register1);
        void BIT(uint8_t& register1, uint8_t bit);
        void RES(uint8_t& register1, uint8_t bit);
        void SET(uint8_t& register1, uint8_t bit);
        int getCycles(uint8_t opcode);
};
