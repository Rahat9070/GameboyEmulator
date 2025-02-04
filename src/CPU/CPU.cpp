#include "CPU.h"

CPU::CPU() {
    reset();
}

void CPU::reset() {
    A = B = C = D = E = H = L = F = 0;
    SP = 0xFFFE;
    PC = 0x0100;
    memory.fill(0); // Clear all memory
    cycles = 0;
    halted = IME = false;
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
        case 0x00: { // NOP
            break;
        } case 0x01: { // LD BC, n16
            C = memory[PC++];
            B = memory[PC++];
            break;
        } case 0x02: { // LD (BC), A
            memory[(B << 8) | C] = A;
            break;
        } case 0x03: { // INC BC
            uint16_t BC = (B << 8) | C;
            BC++;
            B = BC >> 8 & 0xFF;
            C = BC & 0xFF;
            break;
        } case 0x04: { // INC B
            B++;
            incrementFlags(B);
            break;
        }  case 0x05: { // DEC B
            B--;
            decrementFlags(B);
            break;
        } case 0x06: { // LD B, n8
            B = memory[PC++];
            break;
        } case 0x07: { // RLCA (Rotate Left Circular A)
            bool carryOut = getCarryFlag();
            setCarryFlag(A & 0x80);  
            A = (A << 1) | (carryOut ? 1 : 0);
            setZeroFlag(A == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(false);
            break;
        } case 0x08: { // LD (a16), SP
            uint16_t address = memory[PC++] | (memory[PC++] << 8);
            memory[address] = SP & 0xFF;        
            memory[address + 1] = (SP >> 8) & 0xFF;
            break;
        } case 0x09: { // ADD HL, BC
            uint16_t BC = (B << 8) | C;
            uint16_t HL = (H << 8) | L;
            uint32_t result = (HL + BC);
            HL = result & 0xFFFF;
            setZeroFlag(HL == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((HL & 0xFFF) + (BC & 0xFFF)) > 0xFFF);
            break;
        } case 0x0A: { // LD A, [BC]
            uint16_t address = (B << 8) | C;
            A = memory[address];
            break;
        } case 0x0B: { // DEC BC
            uint16_t BC = (B << 8) | C;
            BC--; 
            B = BC >> 8 & 0xFF;  
            C = BC & 0xFF;  
            break;
        } case 0x0C: { // INC C
            C++;
            incrementFlags(C);
            break;
        } case 0x0D: { // DEC C
            C--;
            decrementFlags(C);
            break;
        } case 0x0E: { // LD C, d8
            C = memory[PC++];
            break;
        } case 0x0F: { // RRCA (Rotate Right Circular A)
            bool carryOut = getCarryFlag(); 
            setCarryFlag(A & 0x01); 
            A = (A >> 1) | (carryOut << 7);
            setZeroFlag(false);
            setSubtractFlag(false);
            setHalfCarryFlag(false); 
            break;
        } case 0x10: { // STOP n8
            halted = true;
            break;
        } case 0x11: { // LD DE, n16
            E = memory[PC++]; 
            D = memory[PC++];
            break;
        } case 0x12: { // LD (DE), A
            memory[(D << 8) | E] = A;
            break;
        } case 0x13: { // INC DE
            u_int16_t DE = memory[(D << 8) | E];
            DE++;
            D = (DE >> 8) & 0xFF;
            E = DE & 0xFF;
            break;
        } case 0x14: { // INC D
            D++;
            incrementFlags(D);
            break;
        } case 0x15: { // DEC D
            D--;
            decrementFlags(D);
            break;
        } case 0x16: { // LD D, n8
            D = memory[PC++];
            break;
        } case 0x17: { // RLA (Rotate Left A)
            uint8_t msb = (A & 0x80) >> 7;
            uint8_t carry = getCarryFlag() ? 1 : 0;
            A = (A << 1) | carry;
            setCarryFlag(msb);
            setZeroFlag(false);
            setSubtractFlag(false);
            setHalfCarryFlag(false);
            break;
        } case 0x18: { // JR e8
            int8_t offset = static_cast<int8_t>(memory[PC++]);
            PC += offset;
            break;
        } case 0x19: { // ADD HL, DE
            uint16_t HL = memory[(H << 8) | L];
            uint16_t DE = memory[(D << 8) | E];
            uint32_t result = HL + DE;
            H = (result >> 8) & 0xFF;
            L = result & 0xFF;
            setSubtractFlag(false);
            setHalfCarryFlag(((HL & 0x0FFF) + (DE & 0x0FFF)) > 0x0FFF);
            setCarryFlag(result > 0xFFFF);
            break;
        } case 0x1A: { // LD A, [DE]
            A = memory[(D << 8) | E];
            break;
        } case 0x1B: { // DEC DE
            uint16_t DE = memory[(D << 8) | E];
            DE--;
            D = (DE >> 8) & 0xFF;
            E = DE & 0xFF;
            break;
        } case 0x1C: { // INC E
            E++;
            incrementFlags(E);
            break;
        } case 0x1D: { // DEC E
            E--;
            decrementFlags(E);
            break;
        } case 0x1E: { // LD E, n8
            E = memory[PC++];
            break;
        } case 0x1F: { // RRA
            bool oldCarry = getCarryFlag();
            bool newCarry = A & 0x01;
            setCarryFlag(newCarry);
            A = (A >> 1) | (oldCarry << 7);
            setZeroFlag(false);
            setSubtractFlag(false);
            setHalfCarryFlag(false);
            break;
        } case 0x20: { // JR NZ, e8
            int8_t offset = static_cast<int8_t>(memory[PC++]);
            if (!getZeroFlag()) {
                PC += offset;
                cycles += 12;
            } else {
                cycles += 8;
            }
            break;
        } case 0x21: { // LD HL, n16
            L = memory[PC++];
            H = memory[PC++];
            break;
        } case 0x22: { // LD [HL+], A
            uint16_t HL = (H << 8) | L;
            memory[HL] = A;
            HL++;
            H = (HL >> 8) & 0xFF;
            L = HL & 0xFF;
            break;
        } case 0x23: { // INC HL
            uint16_t HL = (H << 8) | L;
            HL++;
            H = (HL >> 8) & 0xFF;
            L = HL & 0xFF;
            break;
        } case 0x24: { // INC H
            H++;
            incrementFlags(H);
            break;
        } case 0x25: { // DEC H
            H--;
            decrementFlags(H);
            break;
        } case 0x26: { // LD H, n8
            H = memory[PC++];
            break;
        } case 0x27: { // DAA
            uint8_t correction = 0;
            bool setCarry = false;
            if (!getSubtractFlag()) { 
                if (getHalfCarryFlag() || (A & 0x0F) > 9) {
                    correction |= 0x06; 
                }
                if (getCarryFlag() || A > 0x99) {
                    correction |= 0x60;
                    setCarry = true;
                }
                A += correction;
            } else {
                if (getHalfCarryFlag()) {
                    correction |= 0x06;
                }
                if (getCarryFlag()) {
                    correction |= 0x60;
                }
                A -= correction;
            }
            setZeroFlag(A == 0);
            setHalfCarryFlag(false); 
            setCarryFlag(setCarry);
            break;
        } case 0x28: { // JR Z, e8
            int8_t offset = static_cast<int8_t>(memory[PC++]);
            if (getZeroFlag()) {
                PC += offset;
                cycles += 12;
            } else {
                cycles += 8;
            }
            break;
        } case 0x29: { // ADD HL, HL
            uint16_t HL = (H << 8) | L;
            uint32_t result = HL + HL;
            break;
        } case 0x2A: { // LD A, [HL+]
            uint16_t HL = (H << 8) | L;
            A = memory[HL];
            HL++;
            H = (HL >> 8) & 0xFF;
            L = HL & 0xFF;
            break;
        } case 0x2B: { // DEC HL
            uint16_t HL = (H << 8) | L;
            HL--;
            H = (HL >> 8) & 0xFF;
            L = HL & 0xFF;
            break;
        } case 0x2C: { // INC L
            L++;
            incrementFlags(L);
            break;
        } case 0x2D: { // DEC L
            L--;
            decrementFlags(L);
            break;
        } case 0x2E: { // LD L, n8
            L = memory[PC++];
            break;
        } case 0x2F: { // CPL
            A = ~A;
            setSubtractFlag(true);
            setHalfCarryFlag(true);
            break;
        } case 0x30: { // JR NC, e8
            int8_t offset = static_cast<int8_t>(memory[PC++]);
            if (!getCarryFlag()) {
                PC += offset;
                cycles += 12;
            } else {
                cycles += 8;
            }
            break;
        } case 0x31: { // LD SP, n16
            uint16_t low = memory[PC++];
            uint16_t high = memory[PC++];
            SP = (high << 8) | low;
            cycles += 12;
            break;
        } case 0x32: { // LD [HL-], A
            uint16_t HL = (H << 8) | L;
            memory[HL] = A;
            HL--;
            H = (HL >> 8) & 0xFF;
            L = HL & 0xFF;
            break;
        } case 0x33: {// INC SP
            SP++;
            break;
        } case 0x34: { // INC [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = memory[HL];
            uint8_t result = value + 1;
            memory[HL] = result;
            setZeroFlag(result == 0);
            setSubtractFlag(false);
            setHalfCarryFlag((value & 0x0F) == 0x0F);
            break;
        } case 0x35: { // DEC [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = memory[HL];
            uint8_t result = value - 1;
            memory[HL] = result;
            setZeroFlag(result == 0);
            setSubtractFlag(true);
            setHalfCarryFlag((value & 0x0F) == 0x00);
            break;
        } case 0x36: { // LD [HL], n8
            uint16_t HL = (H << 8) | L;
            memory[HL] = memory[PC++];
            break;
        } case 0x37: { // SCF
            setCarryFlag(true);
            setSubtractFlag(false);
            setHalfCarryFlag(false);
            break;
        } case 0x38: { // JR C, e8
            int8_t offset = static_cast<int8_t>(memory[PC++]);
            if (getCarryFlag()) {
                PC += offset;
                cycles += 12;
            } else {
                cycles += 8;
            }
            break;
        } case 0x39: { // ADD HL, SP
            uint16_t HL = (H << 8) | L;
            uint32_t result = HL + SP;
            H = (HL >> 8) & 0xFF;
            L = HL & 0xFF;
            setSubtractFlag(false);
            setHalfCarryFlag(((HL & 0x0F) + (SP & 0x0F)) > 0x0F);
            setCarryFlag(HL > 0xFFFF);
            break;
        } case 0x3A: { // LD A, [HL-]
            uint16_t HL = (H << 8) | L;
            A = memory[HL];
            HL--;
            H = (HL >> 8) & 0xFF;
            L = HL & 0xFF;
            break;
        } case 0x3B: {// DEC SP
            SP--;
            break;
        } case 0x3C: { // INC A
            A++;
            incrementFlags(A);
            break;
        } case 0x3D: { // DEC A
            A--;
            decrementFlags(A);
            break;
        } case 0x3E: { // LD A, n8
            A = memory[PC++];
            break;
        } case 0x3F: { // CCF
            setCarryFlag(!getCarryFlag());
            setSubtractFlag(false);
            setHalfCarryFlag(false);
            break;
        } case 0x40: { // LD B, B
            B = B & 0xFF;
            break;
        } case 0x41: { // LD B, C
            B = C;
            break;
        } case 0x42: { // LD B, D
            B = D;
            break;
        } case 0x43: {// LD B, E
            B = E;
            break;
        } case 0x44: {// LD B, H
            B = H;
            break;
        } case 0x45: {// LD B, L
            B = L;
            break;
        } case 0x46: { // LD B, [HL]
            B = memory[(H << 8) | L];
            break;
        } case 0x47: { // LD B, A
            B = A;
            break;
        } case 0x48: { // LD C, B
            C = B;
            break;
        } case 0x49: { // LD C, C
            C = C;
            break;
        } case 0x4A: { // LD C, D
            C = D;
            break;
        } case 0x4B: { // LD C, E
            C = E;
            break;
        } case 0x4C: { // LD C, H
            C = H;
            break;
        } case 0x4D: { // LD C, L
            C = L;
            break;
        } case 0x4E: { // LD C, [HL]
            C = memory[(H << 8) | L];
            break;
        } case 0x4F: { // LD C, A
            C = A;
            break;
        } case 0x50: { // LD D, B
            D = B;
            break;
        } case 0x51: { // LD D, C
            D = C;
            break;
        } case 0x52: { // LD D, D
            D = D;
            break;
        } case 0x53: { // LD D, E
            D = E;
            break;
        } case 0x54: { // LD D, H
            D = H;
            break;
        } case 0x55: { // LD D, L
            D = L;
            break;
        } case 0x56: { // LD D, [HL]
            D = memory[(H << 8) | L];
            break;
        } case 0x57: { // LD D, A
            D = A;
            break;
        } case 0x58: { // LD E, B
            E = B;
            break;
        } case 0x59: { // LD E, C
            E = C;
            break;
        } case 0x5A: { // LD E, D
            E = D;
            break;
        } case 0x5B: { // LD E, E
            E = E;
            break;
        } case 0x5C: { // LD E, H
            E = H;
            break;
        } case 0x5D: { // LD E, L
            E = L;
            break;
        } case 0x5E: { // LD E, [HL]
            E = memory[(H << 8) | L];
            break;
        } case 0x5F: { // LD E, A
            E = A;
            break;
        } case 0x60: { // LD H, B
            H = B;
            break;
        } case 0x61: { // LD H, C
            H = C;
            break;
        } case 0x62: { // LD H, D
            H = D;
            break;
        } case 0x63: { // LD H, E
            H = E;
            break;
        } case 0x64: { // LD H, H
            H = H;
            break;
        } case 0x65: { // LD H, L
            H = L;
            break;
        } case 0x66: { // LD H, [HL]
            H = memory[(H << 8) | L];
            break;
        } case 0x67: { // LD H, A
            H = A;
            break;
        } case 0x68: { // LD L, B
            L = B;
            break;
        } case 0x69: { // LD L, C
            L = C;
            break;
        } case 0x6A: { // LD L, D
            L = D;
            break;
        } case 0x6B: { // LD L, E
            L = E;
            break;
        } case 0x6C: { // LD L, H
            L = H;
            break;
        } case 0x6D: { // LD L, L
            L = L;
            break;
        } case 0x6E: { // LD L, [HL]
            L = memory[(H << 8) | L];
            break;
        } case 0x6F: { // LD L, A
            L = A;
            break;
        } case 0x70: { // LD [HL], B
            memory[(H << 8) | L] = B;
            break;
        } case 0x71: { // LD [HL], C
            memory[(H << 8) | L] = C;
            break;
        } case 0x72: { // LD [HL], D
            memory[(H << 8) | L] = D;
            break;
        } case 0x73: { // LD [HL], E
            memory[(H << 8) | L] = E;
            break;
        } case 0x74: { // LD [HL], H
            memory[(H << 8) | L] = H;
            break;
        } case 0x75: { // LD [HL], L
            memory[(H << 8) | L] = L;
            break;
        } case 0x76: { // HALT
            halted = true;
            break;
        } case 0x77: { // LD [HL], A
            memory[(H << 8) | L] = A;
            break;
        } case 0x78: { // LD A, B
            A = B;
            break;
        } case 0x79: { // LD A, C
            A = C;
            break;
        } case 0x7A: { // LD A, D
            A = D;
            break;
        } case 0x7B: { // LD A, E
            A = E;
            break;
        } case 0x7C: { // LD A, H
            A = H;
            break;
        } case 0x7D: { // LD A, L
            A = L;
            break;
        } case 0x7E: { // LD A, [HL]
            A = memory[(H << 8) | L];
            break;
        } case 0x7F: { // LD A, A
            A = A;
            break;
        } case 0x80: { // ADD A, B
            uint16_t result = A + B;
            additionFlags(A, B, result);
            A = result & 0xFF;
            break;
        } case 0x81: { // ADD A, C
            uint16_t result = A + C;
            additionFlags(A, C, result);
            A = result & 0xFF;
            break;
        } case 0x82: { // ADD A, D
            uint16_t result = A + D;
            additionFlags(A, D, result);
            A = result & 0xFF;
            break;
        } case 0x83: { // ADD A, E
            uint16_t result = A + E;
            additionFlags(A, E, result);
            A = result & 0xFF;
            break;
        } case 0x84: { // ADD A, H
            uint16_t result = A + H;
            additionFlags(A, H, result);
            A = result & 0xFF;
            break;
        } case 0x85: { // ADD A, L
            uint16_t result = A + L;
            additionFlags(A, L, result);
            A = result & 0xFF;
            break;
        } case 0x86: { // ADD A, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = memory[HL];
            uint16_t result = A + value;
            additionFlags(A, value, result);
            A = result & 0xFF;
            break;
        } case 0x87: { // ADD A, A
            uint16_t result = A + A;
            additionFlags(A, A, result);
            A = result & 0xFF;
            break;
        } case 0x88: { // ADC A, B
            uint16_t result = A + B + (getCarryFlag() ? 1 : 0);
            additionFlags(A, B + getCarryFlag(), result);
            A = result & 0xFF;   
            break;
        } case 0x89: { // ADC A, C
            uint16_t result = A + C + (getCarryFlag() ? 1 : 0);
            additionFlags(A, C + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0x8A: { // ADC A, D
            uint16_t result = A + D + (getCarryFlag() ? 1 : 0);
            additionFlags(A, D + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0x8B: { // ADC A, E
            uint16_t result = A + E + (getCarryFlag() ? 1 : 0);
            additionFlags(A, E + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0x8C: { // ADC A, H
            uint16_t result = A + H + (getCarryFlag() ? 1 : 0);
            additionFlags(A, H + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0x8D: { // ADC A, L
            uint16_t result = A + L + (getCarryFlag() ? 1 : 0);
            additionFlags(A, L + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0x8E: { // ADC A, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = memory[HL];
            uint16_t result = A + value + (getCarryFlag() ? 1 : 0);
            additionFlags(A, value + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0x8F: { // ADC A, A
            uint16_t result = A + A + (getCarryFlag() ? 1 : 0);
            additionFlags(A, A + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0x90: { // SUB A, B
            uint16_t result = A - B;
            subtractionFlags(A, B, result);
            A = result & 0xFF;
            break;
        } case 0x91: { // SUB A, C
            uint16_t result = A - C;
            subtractionFlags(A, C, result);
            A = result & 0xFF;
            break;
        } case 0x92: { // SUB A, D
            uint16_t result = A - D;
            subtractionFlags(A, D, result);
            A = result & 0xFF;
            break;
        } case 0x93: { // SUB A, E
            uint16_t result = A - E;
            subtractionFlags(A, E, result);
            A = result & 0xFF;
            break;
        } case 0x94: { // SUB A, H
            uint16_t result = A - H;
            subtractionFlags(A, H, result);
            A = result & 0xFF;
            break;
        } case 0x95: { // SUB A, L
            uint16_t result = A - L;
            subtractionFlags(A, L, result);
            A = result & 0xFF;
            break;
        } case 0x96: { // SUB A, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = memory[HL];
            uint16_t result = A - value;
            subtractionFlags(A, value, result);
            A = result & 0xFF;
            break;
        } case 0x97: { // SUB A, A
            uint16_t result = A - A;
            subtractionFlags(A, L, result);
            A = result & 0xFF;
            break;
        } case 0x98: { // SBC A, B
            uint16_t result = A - B - (getCarryFlag() ? 1 : 0);
            subtractionFlags(A, B + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0x99: { // SBC A, C
            uint16_t result = A - C - (getCarryFlag() ? 1 : 0);
            subtractionFlags(A, C + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0x9A: { // SBC A, D
            uint16_t result = A - D - (getCarryFlag() ? 1 : 0);
            subtractionFlags(A, D + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0x9B: { // SBC A, E
            uint16_t result = A - E - (getCarryFlag() ? 1 : 0);
            subtractionFlags(A, E + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0x9C: { // SBC A, H
            uint16_t result = A - H - (getCarryFlag() ? 1 : 0);
            subtractionFlags(A, H + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0x9D: { // SBC A, L
            uint16_t result = A - L - (getCarryFlag() ? 1 : 0);
            subtractionFlags(A, L + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0x9E: { // SBC A, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = memory[HL];
            uint16_t result = A - value - (getCarryFlag() ? 1 : 0);
            subtractionFlags(A, value, result);
            A = result & 0xFF;
            break;
        } case 0x9F: { // SBC A, A
            uint16_t result = A - A - (getCarryFlag() ? 1 : 0);
            subtractionFlags(A, A + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0xA0: { // AND A, B
            A &= B;
            andFlags(A);
            break;
        } case 0xA1: { // AND A, C
            A &= C;
            andFlags(A);
            break;
        } case 0xA2: { // AND A, D
            A &= D;
            andFlags(A);
            break;
        } case 0xA3: { // AND A, E
            A &= E;
            andFlags(A);
            break;
        } case 0xA4: { // AND A, H
            A &= H;
            andFlags(A);
            break;
        } case 0xA5: { // AND A, L
            A &= L;
            andFlags(A);
            break;
        } case 0xA6: { // AND A, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = memory[HL];
            A &= value;
            andFlags(A);
            break;
        } case 0xA7: { // AND A, A
            A &= A;
            andFlags(A);
            break;
        } case 0xA8: { // XOR A, B
            A ^= B;
            orFlags(A);
            break;
        } case 0xA9: { // XOR A, C
            A ^= C;
            orFlags(A);
            break;
        } case 0xAA: { // XOR A, D
            A ^= D;
            orFlags(A);
            break;
        } case 0xAB: { // XOR A, E
            A ^= E;
            orFlags(A);
            break;
        } case 0xAC: { // XOR A, H
            A ^= H;
            orFlags(A);
            break;
        } case 0xAD: { // XOR A, L
            A ^= L;
            orFlags(A);
            break;
        } case 0xAE: { // XOR A, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = memory[HL];
            A ^= value;
            orFlags(A);
            break;
        } case 0xAF: { // XOR A, A
            A ^= A;
            orFlags(A);
            break;
        } case 0xB0: { // OR A, B
            A |= B;
            orFlags(A);
            break;
        } case 0xB1: { // OR A, C
            A |= C;
            orFlags(A);
            break;
        } case 0xB2: { // OR A, D
            A |= D;
            orFlags(A);
            break;
        } case 0xB3: { // OR A, E
            A |= E;
            orFlags(A);
            break;
        } case 0xB4: { // OR A, H
            A |= H;
            orFlags(A);
            break;
        } case 0xB5: { // OR A, L
            A |= L;
            orFlags(A);
            break;
        } case 0xB6: { // OR A, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = memory[HL];
            A |= value;
            orFlags(A);
            break;
        } case 0xB7: { // OR A, A
            A |= A;
            orFlags(A);
            break;
        } case 0xB8: { // CP A, B
            subtractionFlags(A, B, A - B);
            break;
        } case 0xB9: { // CP A, C
            subtractionFlags(A, C, A - C);
            break;
        } case 0xBA: { // CP A, D
            subtractionFlags(A, D, A - D);
            break;
        } case 0xBB: { // CP A, E
            subtractionFlags(A, E, A - E);
            break;
        } case 0xBC: { // CP A, H
            subtractionFlags(A, H, A - H);
            break;
        } case 0xBD: { // CP A, L
            subtractionFlags(A, L, A - L);
            break;
        } case 0xBE: { // CP A, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = memory[HL];
            subtractionFlags(A, value, A - value);
            break;
        } case 0xBF: { // CP A, A
            subtractionFlags(A, A, A - A);
            break;
        } case 0xC0: { // RET NZ
            if (!getZeroFlag()) {
                uint16_t returnAddress = memory[SP] | (memory[SP + 1] << 8);
                SP += 2;                
                PC = returnAddress;
            }
            break;
        } case 0xC1: { // POP BC
            C = memory[SP];
            B = memory[SP + 1];
            SP += 2;
            break;
        } case 0xC2: { // JP NZ, a16
            uint16_t address = memory[PC] | (memory[PC + 1] << 8);
            PC += 2;
            if (!getZeroFlag()) {
                PC = address;
                cycles += 16;
            } else {
                cycles += 12;
            }
            break;
        } case 0xC3: { // JP a16
            uint16_t address = memory[PC] | (memory[PC + 1] << 8);
            PC = address;
            cycles += 16;
            break;
        } case 0xC4: { // CALL NZ, a16
            uint16_t address = memory[PC] | (memory[PC + 1] << 8);
            PC += 2;
            if (!getZeroFlag()) { 
                memory[--SP] = (PC >> 8) & 0xFF;
                memory[--SP] = PC & 0xFF;
                PC = address;
                cycles += 24;
            } 
            else {
                cycles += 12;
            }
            break;
        } case 0xC5: { // PUSH BC
            memory[--SP] = B;
            memory[--SP] = C;
            break;
        } case 0xC6: { // ADD A, n8
            uint8_t value = memory[PC++];
            uint16_t result = A + value;
            additionFlags(A, value, result);
            A = result & 0xFF;
            break;
        } case 0xC7: { // RST $00
            memory[--SP] = (PC >> 8) & 0xFF; // High byte
            memory[--SP] = PC & 0xFF;        // Low byte
            PC = 0x00;
            break;
        } case 0xC8: { // RET Z
            if (getZeroFlag()) {
                uint8_t low = memory[SP++];
                uint8_t high = memory[SP++];
                PC = (high << 8) | low;
                cycles += 20;
            } else {
                cycles += 8;
            }
            break;
        } case 0xC9: { // RET
            uint8_t low = memory[SP++];
            uint8_t high = memory[SP++];
            PC = (high << 8) | low;
            break;
        } case 0xCA: { // JP Z, a16
            uint16_t address = memory[PC] | (memory[PC + 1] << 8);
            PC += 2;
            if (getZeroFlag()) {
                PC = address;
                cycles += 16;
            } else {
                cycles += 12;
            }
            break;
        } case 0xCB: { // PREFIX
            // Needs to be completed
            uint8_t cb_opcode = memory[PC++];
            executeCBInstruction(cb_opcode);
            break;
        } case 0xCC: { // CALL Z, a16
            uint16_t address = memory[PC] | (memory[PC + 1] << 8);
            PC += 2;
            if (getZeroFlag()) { 
                memory[--SP] = (PC >> 8) & 0xFF;
                memory[--SP] = PC & 0xFF;
                PC = address;
                cycles += 24;
            } 
            else {
                cycles += 12;
            }
            break;
        } case 0xCD: { // CALL a16
            uint16_t address = memory[PC] | (memory[PC + 1] << 8);
            PC += 2;
            uint8_t low = (PC >> 8) & 0xFF;
            uint8_t high = PC & 0xFF;
            memory[SP--] = high;
            memory[SP--] = low;
            PC = address;
            break;
        } case 0xCE: { // ADC A, n8
            uint8_t value = memory[PC++];            
            uint16_t result = A + value + (getCarryFlag() ? 1 : 0);            
            additionFlags(A, value, result);
            A = result & 0xFF;
            break;
        } case 0xCF: { // RST $08
            uint8_t low = PC & 0xFF;
            uint8_t high = (PC >> 8) & 0xFF;
            memory[SP--] = high;
            memory[SP--] = low;
            PC = 0x08;
            break;
        } case 0xD0: { // RET NC
            if (!getCarryFlag()) {
                uint16_t lowByte = memory[SP];
                uint16_t highByte = memory[SP + 1];
                uint16_t returnAddress = (highByte << 8) | lowByte;
                SP += 2;
                PC = returnAddress;
            } else {
                PC += 1;
            }
            break;
        } case 0xD1: { // POP DE
            E = memory[SP];
            D = memory[SP + 1];
            SP += 2;
            break;
        } case 0xD2: { // JP NC, a16
            uint16_t address = memory[PC] | (memory[PC + 1] << 8);
            PC += 2;
            if (!getCarryFlag()) {
                PC = address;
                cycles += 16;
            } else {
                cycles += 12;
            }
            break;
        } case 0xD4: { // CALL NC, a16
            uint16_t address = memory[PC] | (memory[PC + 1] << 8);
            PC += 2;
            if (!getCarryFlag()) { 
                memory[--SP] = (PC >> 8) & 0xFF;
                memory[--SP] = PC & 0xFF;
                PC = address;
                cycles += 24;
            } 
            else {
                cycles += 12;
            }
            break;
        } case 0xD5: { // PUSH DE
            memory[SP - 1] = D;
            memory[SP - 2] = E;
            SP -= 2;
            break;
        } case 0xD6: { // SUB A, n8
            uint8_t value = memory[PC++];
            uint16_t result = A - value;
            subtractionFlags(A, value, result);
            A = result & 0xFF;
            break;
        } case 0xD7: { // RST $10
            uint8_t low = PC & 0xFF;
            uint8_t high = (PC >> 8) & 0xFF;
            SP--;
            memory[SP--] = high;
            memory[SP] = low;
            PC = 0x10;
            break;
        } case 0xD8: { // RET C
            if (getCarryFlag()) {
                uint16_t lowByte = memory[SP];
                uint16_t highByte = memory[SP + 1];
                uint16_t returnAddress = (highByte << 8) | lowByte;
                SP += 2;
                PC = returnAddress;
            } else {
                PC += 1;
            }
            break;
        } case 0xD9: { // RETI
            uint16_t lowByte = memory[SP];
            uint16_t highByte = memory[SP + 1];
            uint16_t returnAddress = (highByte << 8) | lowByte;
            SP += 2;
            PC = returnAddress;
            IME = true;
            cycles += 16;
            break;
        } case 0xDA: { // JP C, a16
            uint16_t address = memory[PC] | (memory[PC + 1] << 8);
            PC += 2;
            if (getCarryFlag()) {
                PC = address;
                cycles += 16;
            } else {
                cycles += 12;
            }
            break;
        } case 0xDC: { // CALL C, a16
            uint16_t address = memory[PC] | (memory[PC + 1] << 8);
            PC += 2;
            if (getCarryFlag()) { 
                memory[--SP] = (PC >> 8) & 0xFF;
                memory[--SP] = PC & 0xFF;
                PC = address;
                cycles += 24;
            } 
            else {
                cycles += 12;
            }
            break;
        } case 0xDE: { // SBC A, n8
            uint8_t value = memory[PC++];
            uint16_t result = A - value - getCarryFlag();
            subtractionFlags(A, value + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0xDF: { // RST $18
            uint8_t low = PC & 0xFF;
            uint8_t high = (PC >> 8) & 0xFF;
            SP--;
            memory[SP--] = high;
            memory[SP] = low;
            PC = 0x18;
            break;
        } case 0xE0: { // LDH [a8], A
            uint8_t a8 = memory[PC++];
            memory[0xFF00 + a8] = A;
            break;
        } case 0xE1: { // POP HL
            L = memory[SP];
            H = memory[SP + 1];
            SP += 2;
            break;
        } case 0xE2: { // LD [C], A
            memory[0xFF00 + C] = A;
            break;
        } case 0xE5: { // PUSH HL
            memory[SP - 1] = L;
            memory[SP - 2] = H;
            SP -= 2;
            break;
        } case 0xE6: { // AND A, n8
            A &= memory[PC++];
            andFlags(A);
            break;
        } case 0xE7: { // RST $20
            uint8_t low = PC & 0xFF;
            uint8_t high = (PC >> 8) & 0xFF;
            SP--;
            memory[SP--] = high;
            memory[SP] = low;
            PC = 0x20;
            break;
        } case 0xE8: { // ADD SP, e8
            int8_t value = static_cast<int8_t>(memory[PC++]);
            uint16_t result = SP + value;        
            setZeroFlag(result == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((SP & 0xF) + (value & 0xF)) > 0xF);
            setCarryFlag((result > 0xFFFF));
            SP = result & 0xFFFF;
            break;
        } case 0xE9: { // JP HL
            uint16_t HL = (H << 8) | L;
            PC = HL;
            break;
        } case 0xEA: { // LD [a16], A
            uint16_t address = memory[PC] | (memory[PC + 1] << 8);
            PC += 2;
            memory[address] = A;
            break;
        } case 0xEE: { // XOR A, n8
            uint8_t value = memory[PC++];
            A ^= value;
            orFlags(A);
            break;
        } case 0xEF: { // RST $28
            uint8_t low = PC & 0xFF;
            uint8_t high = (PC >> 8) & 0xFF;
            SP--;
            memory[SP--] = high;
            memory[SP] = low;
            PC = 0x28;
            break;
        } case 0xF0: { // LDH A, [a8]
            uint8_t address = memory[PC++];
            A = memory[0xFF00 + address];
            break;
        } case 0xF1: { // POP AF
            F = memory[SP];
            A = memory[SP + 1];
            SP += 2;
            F &= 0xF0;
            break;
        } case 0xF2: { // LD A, [C]
            A = memory[0xFF00 + C];
            break;
        } case 0xF3: { // DI
            IME = false;
            break;
        } case 0xF5: { // PUSH AF
            memory[--SP] = A;
            memory[--SP] = F & 0xF0;
            break;
        } case 0xF6: { // OR A, n8
            A |= memory[PC++];
            orFlags(A);
            break;
        } case 0xF7: { // RST $30
            uint8_t low = PC & 0xFF;
            uint8_t high = (PC >> 8) & 0xFF;
            SP--;
            memory[SP--] = high;
            memory[SP] = low;
            PC = 0x30;
            break;
        } case 0xF8: { // LD HL, SP + e8
            int8_t e8 = static_cast<int8_t>(memory[PC++]);
            uint16_t result = SP + e8;
            setZeroFlag(false);
            setSubtractFlag(false);
            setHalfCarryFlag(((SP & 0xF) + (e8 & 0xF)) > 0xF);
            setCarryFlag(((SP & 0xFF) + (e8 & 0xFF)) > 0xFF);
            H = (result >> 8) & 0xFF;
            L = result & 0xFF;
            break;
        } case 0xF9: { // LD SP, HL
            SP = (H << 8) | L;
            break;
        } case 0xFA: { // LD A, [a16]
            break;
        } case 0xFB: { // EI
            break;
        } case 0xFE: { // CP A, n8
            break;
        } case 0xFF: { // RST $38
            uint8_t low = PC & 0xFF;
            uint8_t high = (PC >> 8) & 0xFF;
            SP--;
            memory[SP--] = high;
            memory[SP] = low;
            PC = 0x38;
            break;
        } default: {
            std::cout << "Unknown opcode: " << std::hex << (int)opcode << std::endl;
            break;
        }
    }
}

void CPU::executeCBInstruction(uint8_t cb_opcode) {
    switch (cb_opcode) {

        default: {
            throw std::runtime_error("Unhandled CB-prefixed opcode: " + std::to_string(cb_opcode));
        }
    }
}

void CPU::incrementFlags(uint8_t register1) {
    setZeroFlag(register1 == 0);
    setSubtractFlag(false);
    setHalfCarryFlag((register1 & 0x0F) == 0);
}
void CPU::decrementFlags(uint8_t register1) {
    setZeroFlag(register1 == 0);
    setSubtractFlag(true);
    setHalfCarryFlag((register1 & 0x0F) == 0x0F);
}
void CPU::additionFlags(uint8_t register1, uint8_t value, uint16_t result) {
    setZeroFlag((result & 0xFF) == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(((register1 & 0x0F) + (value & 0x0F)) > 0x0F);
    setCarryFlag(result > 0xFF);
}
void CPU::subtractionFlags(uint8_t register1, uint8_t value, uint16_t result) {
    setSubtractFlag(true);
    setHalfCarryFlag((register1 & 0xF) < (value & 0xF));
    setCarryFlag(register1 < value);
    setZeroFlag(result == 0);
}
void CPU::andFlags(uint8_t result) {
    setZeroFlag(result == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(true);
    setCarryFlag(false);
}
void CPU::orFlags(uint8_t result) {
    setZeroFlag(result == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(false);
}

void CPU::handleInterrupts() {
    // Implement interrupt logic here
}