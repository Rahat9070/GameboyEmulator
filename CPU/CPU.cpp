#include "CPU.h"
#include <iostream>

CPU::CPU() {
    reset();
}

void CPU::reset() {
    A = B = C = D = E = H = L = F = 0;
    SP = 0xFFFE;
    PC = 0x0100;
    memory.fill(0); // Clear all memory
}

bool CPU::getZeroFlag() { return F & 0x80; }
bool CPU::getSubtractFlag() { return F & 0x40; }
bool CPU::getHalfCarryFlag() { return F & 0x20; }
bool CPU::getCarryFlag() { return F & 0x10; }

void CPU::setZeroFlag(bool value) { F = (F & ~0x80) | (value << 7); }
void CPU::setSubtractFlag(bool value) { F = (F & ~0x40) | (value << 6); }
void CPU::setHalfCarryFlag(bool value) { F = (F & ~0x20) | (value << 5); }
void CPU::setCarryFlag(bool value) { F = (F & ~0x10) | (value << 4); }

void CPU::executeInstruction() {
    uint8_t opcode = memory[PC++];
    decodeAndExecute(opcode);
}

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
            setHalfCarryFlag((B & 0x0F) == 0x0F);
            break;
        case 0x06: // LD B, d8
            B = memory[PC++];
            break;
        case 0x07: // RLCA (Rotate Left Circular A)
            bool carryOut = getCarryFlag();
            setCarryFlag(A & 0x80);  
            A = (A << 1) | (carryOut ? 1 : 0);
            setZeroFlag(A == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(false);
            break;
        case 0x08: // LD (a16), SP
            uint16_t address = memory[PC++] | (memory[PC++] << 8);
            memory[address] = SP & 0xFF;        
            memory[address + 1] = (SP >> 8) & 0xFF;
            break;
        case 0x09: // ADD HL, BC
            uint16_t BC = (B << 8) | C;
            uint16_t HL = (H << 8) | L;
            uint32_t result = (HL + BC);
            HL = result & 0xFFFF;
            setZeroFlag(HL == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((HL & 0xFFF) + (BC & 0xFFF)) > 0xFFF);
            break;
        case 0x0A: // LD A, [BC]
            uint16_t address = (B << 8) | C;
            A = memory[address];
            break;
        case 0x0B: // DEC BC
            uint16_t bc = (B << 8) | C;
            bc--; 
            B = bc >> 8 & 0xFF;  
            C = bc & 0xFF;  
            setZeroFlag(bc == 0);
            setSubtractFlag(true);
            setHalfCarryFlag(false);
            break;
        case 0x0C: // INC C
            C++;
            setZeroFlag(C == 0);
            setSubtractFlag(false);
            setHalfCarryFlag((C & 0x0F) == 0);
            break;
        case 0x0D: // DEC C
            C--;
            setZeroFlag(C == 0);
            setSubtractFlag(true);
            setHalfCarryFlag((C & 0x0F) == 0x0F);
            break;
        case 0x0E: // LD C, d8
            C = memory[PC++];
            break;
        case 0x0F: // RRCA (Rotate Right Circular A)
            bool carryOut = getCarryFlag(); 
            setCarryFlag(A & 0x01); 
            A = (A >> 1) | (carryOut << 7);
            setZeroFlag(false);
            setSubtractFlag(false);
            setHalfCarryFlag(false); 
            break;

        case 0x10: // STOP n8
            halted = true;
            break;
        case 0x11: // LD DE, n16
            E = memory[PC++]; 
            D = memory[PC++];
        case 0x12: // LD (DE), A
            memory[(D << 8) | E] = A;
            break;
        case 0x13: // INC DE
            u_int16_t DE = memory[(D << 8) | E];
            DE++;
            D = (DE >> 8) & 0xFF;
            E = DE & 0xFF;
            break;
        case 0x14: // INC D
            D++;
            setZeroFlag(D == 0);
            setSubtractFlag(false);
            setHalfCarryFlag((D & 0x0F) == 0);
            break;
        case 0x15: // DEC D
            D--;
            setZeroFlag(D == 0);
            setSubtractFlag(true);
            setHalfCarryFlag((D & 0x0F) == 0x0F);
        case 0x16: // LD D, n8
            D = memory[PC++];
            break;
        case 0x17: // RLA (Rotate Left A)
            break;
        
        case 0x40: // LD B, B
            B = B & 0xFF;
            break;
        case 0x41: // LD B, C
            B = C;
            break;
        case 0x42: // LD B, D
            B = D;
            break;
        case 0x43: // LD B, E
            B = E;
            break;
        case 0x44: // LD B, H
            B = H;
            break;
        case 0x45: // LD B, L
            B = L;
            break;
        case 0x46: // LD B, [HL]
            B = memory[(H << 8) | L];
            break;
        case 0x47: // LD B, A
            B = A;
            break;
        case 0x48: // LD C, B
            C = B;
            break;
        case 0x49: // LD C, C
            C = C;
            break;
        case 0x4A: // LD C, D
            C = D;
            break;
        case 0x4B: // LD C, E
            C = E;
            break;
        case 0x4C: // LD C, H
            C = H;
            break;
        case 0x4D: // LD C, L
            C = L;
            break;
        case 0x4E: // LD C, [HL]
            C = memory[(H << 8) | L];
            break;
        case 0x4F: // LD C, A
            C = A;
            break;
        
        case 0x50: // LD D, B
            D = B;
            break;
        case 0x51: // LD D, C
            D = C;
            break;


        default:
            std::cout << "Unknown opcode: " << std::hex << (int)opcode << std::endl;
            break;
    }
}

void CPU::handleInterrupts() {
    // Implement interrupt logic here
}