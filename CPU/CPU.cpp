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
        case 0x01: // LD BC, n16
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
        case 0x06: // LD B, n8
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
            uint8_t msb = (A & 0x80) >> 7;
            uint8_t carry = getCarryFlag() ? 1 : 0;
            A = (A << 1) | carry;
            setCarryFlag(msb);
            setZeroFlag(false);
            setSubtractFlag(false);
            setHalfCarryFlag(false);
            break;
        case 0x18: // JR e8
            int8_t offset = static_cast<int8_t>(memory[PC++]);
            PC += offset;
            break;
        case 0x19: // ADD HL, DE
            uint16_t hl = memory[(H << 8) | L];
            uint16_t de = memory[(D << 8) | E];
            uint32_t result = hl + de;
            H = (result >> 8) & 0xFF;
            L = result & 0xFF;
            setSubtractFlag(false);
            setHalfCarryFlag(((hl & 0x0FFF) + (de & 0x0FFF)) > 0x0FFF);
            setCarryFlag(result > 0xFFFF);
            break;
        case 0x1A: // LD A, [DE]
            A = memory[(D << 8) | E];
            break;
        case 0x1B: // DEC DE
            uint16_t de = memory[(D << 8) | E];
            de--;
            D = (result >> 8) & 0xFF;
            E = result & 0xFF;
            break;
        case 0x1C: // INC E
            E++;
            setZeroFlag(E == 0);
            setSubtractFlag(false);
            setHalfCarryFlag((E & 0x0F) == 0);
            break;
        case 0x1D: // DEC E
            E--;
            setZeroFlag(E == 0);
            setSubtractFlag(true);
            setHalfCarryFlag((E & 0x0F) == 0x0F); 
            break;
        case 0x1E: // LD E, n8
            E = memory[PC++];
            break;
        case 0x1F: // RRA
            bool oldCarry = getCarryFlag();
            bool newCarry = A & 0x01;
            setCarryFlag(newCarry);
            A = (A >> 1) | (oldCarry << 7);
            setZeroFlag(false);
            setSubtractFlag(false);
            setHalfCarryFlag(false);
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
        case 0x52: // LD D, D
            D = D;
            break;
        case 0x53: // LD D, E
            D = E;
            break;
        case 0x54: // LD D, H
            D = H;
            break;
        case 0x55: // LD D, L
            D = L;
            break;
        case 0x56: // LD D, [HL]
            D = memory[(H << 8) | L];
            break;
        case 0x57: // LD D, A
            D = A;
            break;
        case 0x58: // LD E, B
            E = B;
            break;
        case 0x59: // LD E, C
            E = C;
            break;
        case 0x5A: // LD E, D
            E = D;
            break;
        case 0x5B: // LD E, E
            E = E;
            break;
        case 0x5C: // LD E, H
            E = H;
            break;
        case 0x5D: // LD E, L
            E = L;
            break;
        case 0x5E: // LD E, [HL]
            E = memory[(H << 8) | L];
            break;
        case 0x5F: // LD E, A
            E = A;
            break;
        
        case 0x60: // LD H, B
            H = B;
            break;
        case 0x61: // LD H, C
            H = C;
            break;
        case 0x62: // LD H, D
            H = D;
            break;
        case 0x63: // LD H, E
            H = E;
        case 0x64: // LD H, H
            H = H;
            break;
        case 0x65: // LD H, L
            H = L;
            break;
        case 0x66: // LD H, [HL]
            H = memory[(H << 8) | L];
            break;
        case 0x67: // LD H, A
            H = A;
            break;
        case 0x68: // LD L, B
            L = B;
            break;
        case 0x69: // LD L, C
            L = C;
            break;
        case 0x6A: // LD L, D
            L = D;
            break;
        case 0x6B: // LD L, E
            L = E;
            break;
        case 0x6C: // LD L, H
            L = H;
            break;
        case 0x6D: // LD L, L
            L = L;
            break;
        case 0x6E: // LD L, [HL]
            L = memory[(H << 8) | L];
            break;
        case 0x6F: // LD L, A
            L = A;
            break;

        case 0x70: // LD [HL], B
            memory[(H << 8) | L] = B;
            break;
        case 0x71: // LD [HL], C
            memory[(H << 8) | L] = C;
            break;
        case 0x72: // LD [HL], D
            memory[(H << 8) | L] = D;
            break;
        case 0x73: // LD [HL], E
            memory[(H << 8) | L] = E;
            break;
        case 0x74: // LD [HL], H
            memory[(H << 8) | L] = H;
            break;
        case 0x75: // LD [HL], L
            memory[(H << 8) | L] = L;
            break;
        case 0x76: // HALT
            halted = true;
            break;
        case 0x77: // LD [HL], A
            memory[(H << 8) | L] = A;
            break;
        case 0x78: // LD A, B
            A = B;
            break;
        case 0x79: // LD A, C
            A = C;
            break;
        case 0x7A: // LD A, D
            A = D;
            break;
        case 0x7B: // LD A, E
            A = E;
            break;
        case 0x7C: // LD A, H
            A = H;
            break;
        case 0x7D: // LD A, L
            A = L;
            break;
        case 0x7E: // LD A, [HL]
            A = memory[(H << 8) | L];
            break;
        case 0x7F: // LD A, A
            A = A;
            break;
        
        case 0x80: // ADD A, B
            uint16_t result = A + B;
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (B & 0x0F)) > 0x0F);
            setCarryFlag(result > 0xFF);
            A = result & 0xFF;
            break;
        case 0x81: // ADD A, C
            uint16_t result = A + C;
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (C & 0x0F)) > 0x0F);
            setCarryFlag(result > 0xFF);
            A = result & 0xFF;
            break;
        case 0x82: // ADD A, D
            uint16_t result = A + D;
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (D & 0x0F)) > 0x0F);
            setCarryFlag(result > 0xFF);
            A = result & 0xFF;
            break;
        case 0x83: // ADD A, E
            uint16_t result = A + E;
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (E & 0x0F)) > 0x0F);
            setCarryFlag(result > 0xFF);
            A = result & 0xFF;
            break;
        case 0x84: // ADD A, H
            uint16_t result = A + H;
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (H & 0x0F)) > 0x0F);
            setCarryFlag(result > 0xFF);
            A = result & 0xFF;
            break;
        case 0x85: // ADD A, L
            uint16_t result = A + L;
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (L & 0x0F)) > 0x0F);
            setCarryFlag(result > 0xFF);
            A = result & 0xFF;
            break;
        case 0x86: // ADD A, [HL]
            uint16_t hl = (H << 8) | L;
            uint8_t value = memory[hl];
            uint16_t result = A + value;
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (value & 0x0F)) > 0x0F);
            setCarryFlag(result > 0xFF);
            break;
        case 0x87: // ADD A, A
            uint16_t result = A + A;
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (A & 0x0F)) > 0x0F);
            setCarryFlag(result > 0xFF);
            A = result & 0xFF;
            break;
        case 0x88: // ADC A, B
            uint16_t result = A + B + (getCarryFlag() ? 1 : 0);
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (B & 0x0F) + (getCarryFlag() ? 1 : 0)) > 0x0F);
            setCarryFlag(result > 0xFF);
            A = result & 0xFF;   
            break;
        case 0x89: // ADC A, C
            uint16_t result = A + C + (getCarryFlag() ? 1 : 0);
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (C & 0x0F) + (getCarryFlag() ? 1 : 0)) > 0x0F);
            setCarryFlag(result > 0xFF);
            A = result & 0xFF;
            break;
        case 0x8A: // ADC A, D
            uint16_t result = A + D + (getCarryFlag() ? 1 : 0);
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (D & 0x0F) + (getCarryFlag() ? 1 : 0)) > 0x0F);
            setCarryFlag(result > 0xFF);
            A = result & 0xFF;
            break;
        case 0x8B: // ADC A, E
            uint16_t result = A + E + (getCarryFlag() ? 1 : 0);
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (E & 0x0F) + (getCarryFlag() ? 1 : 0)) > 0x0F);
            setCarryFlag(result > 0xFF);
            A = result & 0xFF;
            break;
        case 0x8C: // ADC A, H
            uint16_t result = A + H + (getCarryFlag() ? 1 : 0);
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (H & 0x0F) + (getCarryFlag() ? 1 : 0)) > 0x0F);
            setCarryFlag(result > 0xFF);
            A = result & 0xFF;
            break;
        case 0x8D: // ADC A, L
            uint16_t result = A + L + (getCarryFlag() ? 1 : 0);
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (L & 0x0F) + (getCarryFlag() ? 1 : 0)) > 0x0F);
            setCarryFlag(result > 0xFF);
            A = result & 0xFF;
            break;
        case 0x8E: // ADC A, [HL]
            uint16_t hl = (H << 8) | L;
            uint16_t result = A + hl + (getCarryFlag() ? 1 : 0);
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (hl & 0x0F) + (getCarryFlag() ? 1 : 0)) > 0x0F);
            setCarryFlag(result > 0xFF);
            A = result & 0xFF;
            break;
        case 0x8F: // ADC A, A
            uint16_t result = A + A + (getCarryFlag() ? 1 : 0);
            setZeroFlag((result & 0xFF) == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((A & 0x0F) + (A & 0x0F) + (getCarryFlag() ? 1 : 0)) > 0x0F);
            setCarryFlag(result > 0xFF);
            A = result & 0xFF;
            break;

        default:
            std::cout << "Unknown opcode: " << std::hex << (int)opcode << std::endl;
            break;
    }
}

void CPU::handleInterrupts() {
    // Implement interrupt logic here
}