#include "CPU.h"
#include <iostream>

// Constructor to initialize the CPU
CPU::CPU() {
    reset();
}

// Reset the CPU to its initial state
void CPU::reset() {
    A = B = C = D = E = H = L = F = 0;
    SP = 0xFFFE; // Stack Pointer starts at this address
    PC = 0x0100; // Program Counter starts at this address
    memory.fill(0); // Clear all memory
}

// Get flag values from the F register
bool CPU::getZeroFlag() { return F & 0x80; }
bool CPU::getSubtractFlag() { return F & 0x40; }
bool CPU::getHalfCarryFlag() { return F & 0x20; }
bool CPU::getCarryFlag() { return F & 0x10; }

// Set flag values in the F register
void CPU::setZeroFlag(bool value) { F = (F & ~0x80) | (value << 7); }
void CPU::setSubtractFlag(bool value) { F = (F & ~0x40) | (value << 6); }
void CPU::setHalfCarryFlag(bool value) { F = (F & ~0x20) | (value << 5); }
void CPU::setCarryFlag(bool value) { F = (F & ~0x10) | (value << 4); }

// Fetch-Decode-Execute cycle
void CPU::executeInstruction() {
    uint8_t opcode = memory[PC++];  // Fetch opcode and increment PC
    decodeAndExecute(opcode);      // Decode and execute the opcode
}

// Decode and execute the opcode
void CPU::decodeAndExecute(uint8_t opcode) {
    switch (opcode) {
        case 0x00: // NOP
            break;
        case 0x01: // LD BC, d16
            C = memory[PC++];
            B = memory[PC++];
            break;
        case 0x02: // LD (BC), A
            memory[(B << 8) | C] = A;
            break;
        case 0x03: // INC BC
            uint16_t bc = (B << 8) | C;
            bc++;
            B = bc >> 8 & 0xFF;
            C = bc & 0xFF;
            break;
        case 0x04: // INC B
            B++;
            setZeroFlag(B == 0);
            setSubtractFlag(false);
            setHalfCarryFlag((B & 0x0F) == 0);
            break;
        case 0x05: // DEC B
            B--;
            setZeroFlag(B == 0);
            setSubtractFlag(true);
            setHalfCarryFlag((B & 0x0F) == 0);
            break;
        case 0x06: // LD B, d8
            B = memory[PC++];
            break;
        case 0x07: // RLCA

            break;
        case 0x08: // LD (a16), SP
            uint16_t address = memory[PC++] | (memory[PC++] << 8);
            memory[address] = SP & 0xFF;        
            memory[address + 1] = (SP >> 8) & 0xFF;
            break;
        default:
            std::cerr << "Unknown opcode: " << std::hex << (int)opcode << std::endl;
            break;
    }
}

// Interrupt handling (placeholder)
void CPU::handleInterrupts() {
    // Implement interrupt logic here
}