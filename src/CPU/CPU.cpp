#include "CPU.h"
#include <iostream>

CPU::CPU(MMU* mmu, Scheduler *scheduler) {
    this->mmu = mmu;
    this->scheduler = scheduler;
    memory = mmu->memory;
    PC = 0;
    reset();
}

void CPU::info() {
    std::cout << "A: " << (int)A << " B: " << (int)B << " C: " << (int)C << " D: " << (int)D << " E: " << (int)E << " H: " << (int)H << " L: " << (int)L << " F: " << (int)F << std::endl;
    std::cout << "SP: " << SP << " PC: " << PC << std::endl;
}

void CPU::reset() {
    A = 0x01;
    B = 0x00;
    C = 0x13;
    D = 0x00;
    E = 0xD8;
    F = 0xB0;
    H = 0x01;
    L = 0x4D;
    SP = 0xFFFE;
    PC = 0x0100;
    setZeroFlag(true);
    setHalfCarryFlag(true);
    setCarryFlag(true);
    setSubtractFlag(false);

    IME = false;
    mmu->rom_disabled = true;
    mmu->memory[0xFF0F] = 0xE1;
    memory[0xFF40] = 0x91; // LCDC
    memory[0xFF41] = 0x80; // STAT
    mmu->DIV = 0xAB;
    mmu->TIMA = 0x00;
    mmu->TMA = 0x00;
    mmu->TAC = 0xF8;
    std::cout << "CPU Initialized" << std::endl;
}

bool CPU::getZeroFlag() { return F & 0x80; }
bool CPU::getSubtractFlag() { return F & 0x40; }
bool CPU::getHalfCarryFlag() { return F & 0x20; }
bool CPU::getCarryFlag() { return F & 0x10; }

void CPU::setZeroFlag(bool value) { 
    F = (F & ~0x80) | (value << 7); 
}
void CPU::setSubtractFlag(bool value) { 
    F = (F & ~0x40) | (value << 6); 
}
void CPU::setHalfCarryFlag(bool value) { 
    F = (F & ~0x20) | (value << 5); 
}
void CPU::setCarryFlag(bool value) { 
    F = (F & ~0x10) | (value << 4); 
}

bool CPU::checkInterrupts() {
    if (mmu->read_byte(0xFFFF) & mmu->read_byte(0xFF0F) & 0x0F) {
        halted = false;
    }
    if (!IME) {
        return false;
    }
    if (mmu->is_interrupt_enabled(mmu->VBLANK) && mmu->is_interrupt_flag_enabled(mmu->VBLANK)) {
        updateInterrupt(mmu->VBLANK, 0x40);
        return true;
    }
    if (mmu->is_interrupt_enabled(mmu->LCD) && mmu->is_interrupt_flag_enabled(mmu->LCD)) {
        updateInterrupt(mmu->LCD, 0x48);
        return true;
    }
    if (mmu->is_interrupt_enabled(mmu->TIMER) && mmu->is_interrupt_flag_enabled(mmu->TIMER)) {
        updateInterrupt(mmu->TIMER, 0x50);
        return true;
    }
    return false;
}

void CPU::updateInterrupt(uint8_t interruptFlag, uint8_t pc) {
    SP -= 2;
    mmu->write_byte(SP, (uint8_t) (pc & 0x00FF));
    mmu->write_byte(SP, (uint8_t) ((pc & 0xFF00) >> 8));
    IME = false;
    uint8_t value = mmu->read_byte(0xFF0F);
    mmu->write_byte(0xFF0F, value | interruptFlag);
    halted = false;
}

int CPU::getCycles(uint8_t opcode) {
    switch (opcode) {
        case 0x20: { // JR NZ, offset
            if (!getZeroFlag()) { return 12; }
            break;
        }
        case 0x28: { // JR Z, offset
            if (getZeroFlag()) { return 12; }
            break;
        }
        case 0x30: { // JR NC, offset
            if (!getCarryFlag()) { return 12; }
            break;
        }
        case 0x38: { // JR C, offset
            if (getCarryFlag()) { return 12; }
            break;
        }
        case 0xC0: { // RET NZ
            if (!getZeroFlag()) { return 20; }
            break;
        }
        case 0xC8: { // RET Z
            if (getZeroFlag()) { return 20; }
            break;
        }
        case 0xCB: { // CB Prefix
            uint8_t cb = mmu->read_byte(PC++);
            return CBinstructionCycles[cb];
            break;
        }
        case 0xD0: { // RET NC
            if (!getCarryFlag()) { return 20; }
            break;
        }
        case 0xD8: { // RET C
            if (getCarryFlag()) { return 20; }
            break;
        }
        case 0xC4: { // CALL NZ, addr
            if (!getZeroFlag()) { return 24; }
            break;
        }
        case 0xCC: { // CALL Z, addr
            if (getZeroFlag()) { return 24; }
            break;
        }
        case 0xD4: { // CALL NC, addr
            if (!getCarryFlag()) { return 24; }
            break;
        }
        case 0xDC: { // CALL C, addr
            if (getCarryFlag()) { return 24; }
            break;
        }
    }
    return instructionCycles[opcode];
}

void CPU::executeInstruction(uint8_t opcode) {
    //info();
    // std::cout << "Opcode: " << std::hex << (int)opcode << "\n" << "PC: " << std::hex << (int)PC << std::endl;
    switch (opcode) {
        case 0x00: { // NOP
            break;
        } case 0x01: { // LD BC, n16
            uint16_t BC = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
            B = (uint8_t) (BC >> 8 & 0xFF);
            C = (uint8_t) (BC & 0xFF);
            PC += 2;
            break;
        } case 0x02: { // LD (BC), A
            uint16_t BC = (B << 8) | C;
            mmu->write_byte(BC, A);
            break;
        } case 0x03: { // INC BC
            uint16_t BC = (B << 8) | C;
            BC++;
            B = (uint8_t) (BC >> 8 & 0xFF);
            C = (uint8_t) (BC & 0xFF);
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
            B = mmu->read_byte(PC++);
            break;
        } case 0x07: { // RLCA (Rotate Left Circular A)
            RLC(A);
            setZeroFlag(false);
            break;
        } case 0x08: { // LD (a16), SP
            uint16_t address = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
            mmu->write_byte(address, (uint8_t)(SP & 0x00FF));
            mmu->write_byte(address + 1, (uint8_t)((SP & 0xFF00) >> 8));
            PC += 2;
            break;
        } case 0x09: { // ADD HL, BC
            uint16_t BC = (B << 8) | C;
            uint16_t HL = (H << 8) | L;
            uint32_t result = (HL + BC);
            HL = result & 0xFFFF;
            H = (HL >> 8) & 0xFF;
            L = HL & 0xFF;
            setZeroFlag(HL == 0);
            setSubtractFlag(false);
            setHalfCarryFlag(((HL & 0xFFF) + (BC & 0xFFF)) > 0xFFF);
            break;
        } case 0x0A: { // LD A, [BC]
            uint16_t address = (B << 8) | C;
            A = mmu->read_byte(address);
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
            C = mmu->read_byte(PC++);
            break;
        } case 0x0F: { // RRCA (Rotate Right Circular A)
            RRC(A);
            setZeroFlag(false);
            break;
        } case 0x10: { // STOP n8
            break;
        } case 0x11: { // LD DE, n16
            uint16_t DE = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
            D = (DE >> 8) & 0xFF;
            E = DE & 0xFF;
            PC += 2;
            break;
        } case 0x12: { // LD (DE), A
            uint16_t DE = (D << 8) | E;
            mmu->write_byte(DE, A);
            break;
        } case 0x13: { // INC DE
            u_int16_t DE = (D << 8) | E;
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
            D = mmu->read_byte(PC++);
            break;
        } case 0x17: { // RLA (Rotate Left A)
            RL(A);
            setZeroFlag(false);
            break;
        } case 0x18: { // JR e8
            uint8_t offset = mmu->read_byte(PC + 1);
            PC += 2;
            PC += int8_t(offset);
            break;
        } case 0x19: { // ADD HL, DE
            uint16_t HL = (H << 8) | L;
            uint16_t DE = (D << 8) | E;
            uint32_t result = HL + DE;
            H = (result >> 8) & 0xFF;
            L = result & 0xFF;
            setSubtractFlag(false);
            setHalfCarryFlag(((HL & 0x0FFF) + (DE & 0x0FFF)) > 0x0FFF);
            setCarryFlag(result > 0xFFFF);
            break;
        } case 0x1A: { // LD A, [DE]
            uint16_t DE = (D << 8) | E;
            A = mmu->read_byte(DE);
            break;
        } case 0x1B: { // DEC DE
            uint16_t DE = (D << 8) | E;
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
            E = mmu->read_byte(PC++);
            break;
        } case 0x1F: { // RRA
            RR(A);
            setZeroFlag(false);
            break;
        } case 0x20: { // JR NZ, e8
            uint8_t offset = mmu->read_byte(PC + 1);
            if (getZeroFlag()) {
                PC += 2;
                PC += int8_t(offset);
            } else {
                PC += 2;
            }
            break;
        } case 0x21: { // LD HL, n16
            uint16_t HL = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
            H = (HL >> 8) & 0xFF;
            L = HL & 0xFF;
            PC += 2;
            break;
        } case 0x22: { // LD [HL+], A
            uint16_t HL = (H << 8) | L;
            mmu->write_byte(HL, A);
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
            H = mmu->read_byte(PC++);
            break;
        } case 0x27: { // DAA
            uint16_t correction = 0;
            if (!getSubtractFlag()) { 
                if (getHalfCarryFlag() || (A & 0x0F) > 0x09) {
                    correction += 0x06; 
                }
                if (getCarryFlag() || A > 0x99) {
                    correction += 0x60;
                    setCarryFlag(true);
                }
            } else {
                if (getHalfCarryFlag()) {
                    correction -= 0x06;
                }
                if (getCarryFlag()) {
                    correction -= 0x60;
                }
            }
            A += correction;
            setZeroFlag(A == 0);
            setHalfCarryFlag(false); 
            break;
        } case 0x28: { // JR Z, e8
            uint8_t offset = mmu->read_byte(PC + 1);
            if (getZeroFlag()) {
                PC += 2;
                PC += int8_t(offset);
            } else {
                PC += 2;
            }
            break;
        } case 0x29: { // ADD HL, HL
            uint16_t HL = (H << 8) | L;
            uint32_t result = HL + HL;
            H = (result >> 8) & 0xFF;
            L = result & 0xFF;
            break;
        } case 0x2A: { // LD A, [HL+]
            uint16_t HL = (H << 8) | L;

            A = mmu->read_byte(HL++);
            std::cout << "Byte read!";
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
            L = mmu->read_byte(PC++);
            break;
        } case 0x2F: { // CPL
            A = ~A;
            setSubtractFlag(true);
            setHalfCarryFlag(true);
            break;
        } case 0x30: { // JR NC, e8
            uint8_t offset = mmu->read_byte(PC + 1);
            if (!getCarryFlag()) {
                PC += 2;
                PC += int8_t(offset);
            } else {
                PC += 2;
            }
            break;
        } case 0x31: { // LD SP, n16
            SP = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
            PC += 2;
            break;
        } case 0x32: { // LD [HL-], A
            uint16_t HL = (H << 8) | L;
            mmu->write_byte(HL, A);
            HL--;
            H = (HL >> 8) & 0xFF;
            L = HL & 0xFF;
            break;
        } case 0x33: {// INC SP
            SP++;
            break;
        } case 0x34: { // INC [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            uint8_t result = value + 1;
            mmu->write_byte(HL, result);
            setZeroFlag(result == 0);
            setSubtractFlag(false);
            setHalfCarryFlag((result & 0x0F) == 0x0F);
            break;
        } case 0x35: { // DEC [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            uint8_t result = value - 1;
            mmu->write_byte(HL, result);
            setZeroFlag(result == 0);
            setSubtractFlag(false);
            setHalfCarryFlag((result & 0x0F) == 0x0F);
            break;
        } case 0x36: { // LD [HL], n8
            uint16_t HL = (H << 8) | L;
            mmu->write_byte(HL, mmu->read_byte(PC++));
            break;
        } case 0x37: { // SCF
            setCarryFlag(true);
            setSubtractFlag(false);
            setHalfCarryFlag(false);
            break;
        } case 0x38: { // JR C, e8
            uint8_t offset = mmu->read_byte(PC + 1);
            if (getCarryFlag()) {
                PC += 2;
                PC += int8_t(offset);
            } else {
                PC += 2;
            }
            break;
        } case 0x39: { // ADD HL, SP
            uint16_t HL = (H << 8) | L;
            uint32_t result = HL + SP;
            H = (result >> 8) & 0xFF;
            L = result & 0xFF;
            setSubtractFlag(false);
            setHalfCarryFlag(((HL & 0x0F) + (SP & 0x0F)) > 0x0F);
            setCarryFlag(HL > 0xFFFF);
            break;
        } case 0x3A: { // LD A, [HL-]
            uint16_t HL = (H << 8) | L;
            A = mmu->read_byte(HL--);
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
            A = mmu->read_byte(PC++);
            break;
        } case 0x3F: { // CCF
            setCarryFlag(!getCarryFlag());
            setSubtractFlag(false);
            setHalfCarryFlag(false);
            break;
        } case 0x40: { // LD B, B
            B = B;
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
            B = mmu->read_byte((H << 8) | L);
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
            C = mmu->read_byte((H << 8) | L);
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
            D = mmu->read_byte((H << 8) | L);
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
            E = mmu->read_byte((H << 8) | L);
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
            H = mmu->read_byte((H << 8) | L);
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
            L = mmu->read_byte((H << 8) | L);
            break;
        } case 0x6F: { // LD L, A
            L = A;
            break;
        } case 0x70: { // LD [HL], B
            mmu->write_byte((H << 8) | L, B);
            break;
        } case 0x71: { // LD [HL], C
            mmu->write_byte((H << 8) | L, C);
            break;
        } case 0x72: { // LD [HL], D
            mmu->write_byte((H << 8) | L, D);
            break;
        } case 0x73: { // LD [HL], E
            mmu->write_byte((H << 8) | L, E);
            break;
        } case 0x74: { // LD [HL], H
            mmu->write_byte((H << 8) | L, H);
            break;
        } case 0x75: { // LD [HL], L
            mmu->write_byte((H << 8) | L, L);
            break;
        } case 0x76: { // HALT
            if (!IME && (mmu->read_byte(0xFFFF) & mmu->read_byte(0xFF0F) & 0x1F)) {
                IME = true;
                halted = false;
                break;
            }
            halted = true;
            break;
        } case 0x77: { // LD [HL], A
            mmu->write_byte((H << 8) | L, A);
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
            A = mmu->read_byte((H << 8) | L);
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
            uint8_t value = mmu->read_byte((H << 8) | L);
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
            uint8_t value = mmu->read_byte(HL);
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
            uint8_t value = mmu->read_byte(HL);
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
            uint8_t value = mmu->read_byte(HL);
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
            uint8_t value = mmu->read_byte(HL);
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
            uint8_t value = mmu->read_byte(HL);
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
            uint8_t value = mmu->read_byte(HL);
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
            uint8_t value = mmu->read_byte(HL);
            subtractionFlags(A, value, A - value);
            break;
        } case 0xBF: { // CP A, A
            subtractionFlags(A, A, A - A);
            break;
        } case 0xC0: { // RET NZ
            if (!getZeroFlag()) {
                PC = mmu->read_byte(SP) | (mmu->read_byte(SP + 1) << 8);
            }
            break;
        } case 0xC1: { // POP BC
            uint16_t BC = mmu->read_byte(SP) | (mmu->read_byte(SP + 1) << 8);
            SP += 2;

            B = BC >> 8;
            C = BC & 0xFF;
            break;
        } case 0xC2: { // JP NZ, a16
            if (!getZeroFlag()) {
                PC = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
                break;
            }
            PC++;
            break;
        } case 0xC3: { // JP a16
            uint16_t address = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
            PC = address;
            break;
        } case 0xC4: { // CALL NZ, a16
            uint16_t address = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
            PC += 2;
            if (!getZeroFlag()) {
                SP -= 2;
                mmu->write_byte(SP, (uint8_t)(PC & 0x00ff));
                mmu->write_byte(SP + 1, (uint8_t)((PC & 0xff00) >> 8));
                PC = address;
            } 
            break;
        } case 0xC5: { // PUSH BC
            uint16_t BC = (B << 8) | C;
            SP -= 2;
            mmu->write_byte(SP, (uint8_t)(BC & 0x00ff));
            mmu->write_byte(SP + 1, (uint8_t)((BC & 0xff00) >> 8));
            break;
        } case 0xC6: { // ADD A, n8
            uint8_t value = mmu->read_byte(PC++);
            uint16_t result = A + value;
            additionFlags(A, value, result);
            A = result & 0xFF;
            break;
        } case 0xC7: { // RST $00
            uint16_t return_address = PC;
            SP -= 2;
            mmu->write_byte(SP, (uint8_t)(return_address & 0x00FF));         // LOW byte
            mmu->write_byte(SP + 1, (uint8_t)((return_address >> 8) & 0x00FF)); // HIGH byte
            PC = 0x0000;
            break;
        } case 0xC8: { // RET Z
            if (getZeroFlag()) {
                PC = mmu->read_byte(SP) | (mmu->read_byte(SP + 1) << 8);
            }
            break;
        } case 0xC9: { // RET
            PC = mmu->read_byte(SP) | (mmu->read_byte(SP + 1) << 8);
            break;
        } case 0xCA: { // JP Z, a16
            if (getZeroFlag()) {
                PC = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
                break;
            }
            PC += 2;
            break;
        } case 0xCB: { // CB PREFIX
            uint8_t cb_opcode = mmu->read_byte(PC++);
            executeCBInstruction(cb_opcode);
            break;
        } case 0xCC: { // CALL Z, a16
            uint16_t address = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
            PC += 2;
            if (getZeroFlag()) {
                SP -= 2;
                mmu->write_byte(SP, (uint8_t)(PC & 0x00ff));
                mmu->write_byte(SP + 1, (uint8_t)((PC & 0xff00) >> 8));
                PC = address;
            } 
            break;
        } case 0xCD: { // CALL a16
            uint16_t address = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
            PC += 2;
            SP -= 2;
            mmu->write_byte(SP, (uint8_t)(PC & 0x00ff));
            mmu->write_byte(SP + 1, (uint8_t)((PC & 0xff00) >> 8));
            PC = address;
            break;
        } case 0xCE: { // ADC A, n8
            uint8_t value = mmu->read_byte(PC++);            
            uint16_t result = A + value + (getCarryFlag() ? 1 : 0);            
            additionFlags(A, value + (getCarryFlag() ? 1 : 0), result);
            A = result & 0xFF;
            break;
        } case 0xCF: { // RST $08
            uint16_t return_address = PC;
            SP -= 2;
            mmu->write_byte(SP, (uint8_t)(return_address & 0x00FF));         // LOW byte
            mmu->write_byte(SP + 1, (uint8_t)((return_address >> 8) & 0x00FF)); // HIGH byte
            PC = 0x0008;
            break;
        } case 0xD0: { // RET NC
            if (!getCarryFlag()) {
                PC = mmu->read_byte(SP) | (mmu->read_byte(SP + 1) << 8);
            }
            break;
        } case 0xD1: { // POP DE
            uint16_t DE = mmu->read_byte(SP) | (mmu->read_byte(SP + 1) << 8);
            SP += 2;

            D = DE >> 8;
            E = DE & 0xFF;
            break;
        } case 0xD2: { // JP NC, a16
            if (!getCarryFlag()) {
                PC = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
                break;
            }
            PC += 2;
            break;
        } case 0xD4: { // CALL NC, a16
            uint16_t address = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
            PC += 2;
            if (!getCarryFlag()) { 
                SP -= 2;
                mmu->write_byte(SP, (uint8_t)(PC & 0x00ff));
                mmu->write_byte(SP + 1, (uint8_t)((PC & 0xff00) >> 8));
                PC = address;
                break;
            } 
            break;
        } case 0xD5: { // PUSH DE
            uint16_t DE = (D << 8) | E;
            SP -= 2;
            mmu->write_byte(SP, (uint8_t)(DE & 0x00ff));
            mmu->write_byte(SP + 1, (uint8_t)((DE & 0xff00) >> 8));
            break;
        } case 0xD6: { // SUB A, n8
            uint8_t value = mmu->read_byte(PC++);
            uint16_t result = A - value;
            subtractionFlags(A, value, result);
            A = result & 0xFF;
            break;
        } case 0xD7: { // RST $10
            uint16_t return_address = PC;
            SP -= 2;
            mmu->write_byte(SP, (uint8_t)(return_address & 0x00FF));         // LOW byte
            mmu->write_byte(SP + 1, (uint8_t)((return_address >> 8) & 0x00FF)); // HIGH byte
            PC = 0x0010;
            break;
        } case 0xD8: { // RET C
            if (getCarryFlag()) {
                PC = mmu->read_byte(SP) | (mmu->read_byte(SP + 1) << 8);
            }
            break;
        } case 0xD9: { // RETI
            IME = true;
            PC = mmu->read_byte(SP) | (mmu->read_byte(SP + 1) << 8);
            SP += 2;
            break;
        } case 0xDA: { // JP C, a16
            if (getCarryFlag()) {
                PC = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
                break;
            }
            PC += 2;
            break;
        } case 0xDC: { // CALL C, a16
            uint16_t address = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
            PC += 2;
            if (getCarryFlag()) { 
                SP -= 2;
                mmu->write_byte(SP, (uint8_t)(PC & 0x00ff));
                mmu->write_byte(SP + 1, (uint8_t)((PC & 0xff00) >> 8));
                PC = address;
                break;
            } 
            break;
        } case 0xDE: { // SBC A, n8
            uint8_t value = mmu->read_byte(PC++);
            uint16_t result = A - value - getCarryFlag();
            subtractionFlags(A, value + getCarryFlag(), result);
            A = result & 0xFF;
            break;
        } case 0xDF: { // RST $18
            uint16_t return_address = PC;
            SP -= 2;
            mmu->write_byte(SP, (uint8_t)(return_address & 0x00FF));         // LOW byte
            mmu->write_byte(SP + 1, (uint8_t)((return_address >> 8) & 0x00FF));
            PC = 0x0018;
            break;
        } case 0xE0: { // LDH [a8], A
            mmu->write_byte(0xFF00 + mmu->read_byte(PC++), A);
            break;
        } case 0xE1: { // POP HL
            uint16_t HL = mmu->read_byte(SP) | (mmu->read_byte(SP + 1) << 8);
            SP += 2;

            H = HL >> 8;
            L = HL & 0xFF;
            break;
        } case 0xE2: { // LD [C], A
            mmu->write_byte(0xFF00 + C, A);
            break;
        } case 0xE5: { // PUSH HL
            uint16_t HL = (H << 8) | L;
            SP -= 2;
            mmu->write_byte(SP, (uint8_t)(HL & 0x00ff));
            mmu->write_byte(SP + 1, (uint8_t)((HL & 0xff00) >> 8));
            break;
        } case 0xE6: { // AND A, n8
            A &= mmu->read_byte(PC++);
            andFlags(A);
            break;
        } case 0xE7: { // RST $20
            uint16_t return_address = PC;
            SP -= 2;
            mmu->write_byte(SP, (uint8_t)(return_address & 0x00FF));         // LOW byte
            mmu->write_byte(SP + 1, (uint8_t)((return_address >> 8) & 0x00FF));
            PC = 0x0020;
            break;
        } case 0xE8: { // ADD SP, e8
            int8_t value = mmu->read_byte(PC++);
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
            uint16_t address = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
            PC += 2;  // Increment PC after reading address
            mmu->write_byte(address, A);
            break;
        } case 0xEE: { // XOR A, n8
            A ^= mmu->read_byte(PC++);
            orFlags(A);
            break;
        } case 0xEF: { // RST $28
            uint16_t return_address = PC;
            SP -= 2;
            mmu->write_byte(SP, (uint8_t)(return_address & 0x00FF));         // LOW byte
            mmu->write_byte(SP + 1, (uint8_t)((return_address >> 8) & 0x00FF));
            PC = 0x0028;
            break;
        } case 0xF0: { // LDH A, [a8]
            uint8_t address = mmu->read_byte(PC++);
            A = mmu->read_byte(0xFF00 + address);
            break;
        } case 0xF1: { // POP AF
            uint16_t AF = mmu->read_byte(SP) | (mmu->read_byte(SP + 1) << 8);
            SP += 2;

            A = AF >> 8;
            F = AF & 0xF0;
            break;
        } case 0xF2: { // LD A, [C]
            A = mmu->read_byte(0xFF00 + C);
            break;
        } case 0xF3: { // DI
            IME = false;
            break;
        } case 0xF5: { // PUSH AF
            uint16_t AF = (A << 8) | F;
            SP -= 2;
            mmu->write_byte(SP, (uint8_t)(AF & 0x00ff));
            mmu->write_byte(SP + 1, (uint8_t)((AF & 0xff00) >> 8));
            break;
        } case 0xF6: { // OR A, n8
            A |= mmu->read_byte(PC++);
            orFlags(A);
            break;
        } case 0xF7: { // RST $30
            uint16_t return_address = PC;
            SP -= 2;
            mmu->write_byte(SP, (uint8_t)(return_address & 0x00FF));         // LOW byte
            mmu->write_byte(SP + 1, (uint8_t)((return_address >> 8) & 0x00FF));
            PC = 0x0030;
            break;
        } case 0xF8: { // LD HL, SP + e8
            int8_t e8 = mmu->read_byte(PC++);
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
            uint16_t address = mmu->read_byte(PC) | (mmu->read_byte(PC + 1) << 8);
            PC += 2;
            A = mmu->read_byte(address);
            break;
        } case 0xFB: { // EI
            IME = true;
            break;
        } case 0xFE: { // CP A, n8
            setSubtractFlag(true);
            setZeroFlag(A == mmu->read_byte(PC++));
            setHalfCarryFlag(((A & 0x0F) - (mmu->read_byte(PC) & 0x0F)) < 0);
            setCarryFlag(A < mmu->read_byte(PC));
            break;
        } case 0xFF: { // RST $38
            uint16_t return_address = PC;
            SP -= 2;
            mmu->write_byte(SP, (uint8_t)(return_address & 0x00FF));         // LOW byte
            mmu->write_byte(SP + 1, (uint8_t)((return_address >> 8) & 0x00FF));
            PC = 0x0038;
            break;
        } default: {
            std::cout << "Invalid opcode: " << std::hex << (int)opcode << std::endl;
            break;
        }
    }
}

void CPU::executeCBInstruction(uint8_t cb_opcode) {
    switch (cb_opcode) {
        case 0x00: { // RLC B
            RLC(B);
            break;
        } case 0x01: { // RLC C
            RLC(C);
            break;
        } case 0x02: { // RLC D
            RLC(D);
            break;
        } case 0x03: { // RLC E
            RLC(E);
            break;
        } case 0x04: { // RLC H
            RLC(H);
            break;
        } case 0x05: { // RLC L
            RLC(L);
            break;
        } case 0x06: { // RLC [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            RLC(value);
            mmu->write_byte(HL, value);
            break;
        } case 0x07: { // RLC A
            RLC(A);
            break;
        } case 0x08: { // RRC B
            RRC(B);
            break;
        } case 0x09: { // RRC C
            RRC(C);
            break;
        } case 0x0A: { // RRC D
            RRC(D);
            break;
        } case 0x0B: { // RRC E
            RRC(E);
            break;
        } case 0x0C: { // RRC H
            RRC(H);
            break;
        } case 0x0D: { // RRC L
            RRC(L);
            break;
        } case 0x0E: { // RRC [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            RRC(value);
            mmu->write_byte(HL, value);
            break;
        } case 0x0F: { // RRC A
            RRC(A);
            break;
        } case 0x10: { // RL B
            RL(B);
            break;
        } case 0x11: { // RL C
            RL(C);
            break;
        } case 0x12: { // RL D
            RL(D);
            break;
        } case 0x13: { // RL E
            RL(E);
            break;
        } case 0x14: { // RL H
            RL(H);
            break;
        } case 0x15: { // RL L
            RL(L);
            break;
        } case 0x16: { // RL [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            RL(value);
            mmu->write_byte(HL, value);
            break;
        } case 0x17: { // RL A
            RL(A);
            break;
        } case 0x18: { // RR B
            RR(B);
            break;
        } case 0x19: { // RR C
            RR(C);
            break;
        } case 0x1A: { // RR D
            RR(D);
            break;
        } case 0x1B: { // RR E
            RR(E);
            break;
        } case 0x1C: { // RR H
            RR(H);
            break;
        } case 0x1D: { // RR L
            RR(L);
            break;
        } case 0x1E: { // RR [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            RR(value);
            mmu->write_byte(HL, value);
            break;
        } case 0x1F: { // RR A
            RR(A);
            break;
        } case 0x20: { // SLA B
            SLA(B);
            break;
        } case 0x21: { // SLA C
            SLA(C);
            break;
        } case 0x22: { // SLA D
            SLA(D);
            break;
        } case 0x23: { // SLA E
            SLA(E);
            break;
        } case 0x24: { // SLA H
            SLA(H);
            break;
        } case 0x25: { // SLA L
            SLA(L);
            break;
        } case 0x26: { // SLA [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            SLA(value);
            mmu->write_byte(HL, value);
            break;
        } case 0x27: { // SLA A
            SLA(A);
            break;
        } case 0x28: { // SRA B
            SRA(B);
            break;
        } case 0x29: { // SRA C
            SRA(C);
            break;
        } case 0x2A: { // SRA D
            SRA(D);
            break;
        } case 0x2B: { // SRA E
            SRA(E);
            break;
        } case 0x2C: { // SRA H
            SRA(H);
            break;
        } case 0x2D: { // SRA L
            SRA(L);
            break;
        } case 0x2E: { // SRA [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            SRA(value);
            mmu->write_byte(HL, value);
            break;
        } case 0x2F: { // SRA A
            SRA(A);
            break;
        } case 0x30: { // SWAP B
            SWAP(B);
            break;
        } case 0x31: { // SWAP C
            SWAP(C);
            break;
        } case 0x32: { // SWAP D
            SWAP(D);
            break;
        } case 0x33: { // SWAP E
            SWAP(E);
            break;
        } case 0x34: { // SWAP H
            SWAP(H);
            break;
        } case 0x35: { // SWAP L
            SWAP(L);
            break;
        } case 0x36: { // SWAP [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            SWAP(value);
            mmu->write_byte(HL, value);
            break;
        } case 0x37: { // SWAP A
            SWAP(A);
            break;
        } case 0x38: { // SRL B
            SRL(B);
            break;
        } case 0x39: { // SRL C
            SRL(C);
            break;
        } case 0x3A: { // SRL D
            SRL(D);
            break;
        } case 0x3B: { // SRL E
            SRL(E);
            break;
        } case 0x3C: { // SRL H
            SRL(H);
            break;
        } case 0x3D: { // SRL L
            SRL(L);
            break;
        } case 0x3E: { // SRL [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            SRL(value);
            mmu->write_byte(HL, value);
            break;
        } case 0x3F: { // SRL A
            SRL(A);
            break;
        } case 0x40: { // BIT 0, B
            BIT(B, 0);
            break;
        } case 0x41: { // BIT 0, C
            BIT(C, 0);
            break;
        } case 0x42: { // BIT 0, D
            BIT(D, 0);
            break;
        } case 0x43: { // BIT 0, E
            BIT(E, 0);
            break;
        } case 0x44: { // BIT 0, H
            BIT(H, 0);
            break;
        } case 0x45: { // BIT 0, L
            BIT(L, 0);
            break;
        } case 0x46: { // BIT 0, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            BIT(value, 0);
            mmu->write_byte(HL, value);
            break;
        } case 0x47: { // BIT 0, A
            BIT(A, 0);
            break;
        } case 0x48: { // BIT 1, B
            BIT(B, 1);
            break;
        } case 0x49: { // BIT 1, C
            BIT(C, 1);
            break;
        } case 0x4A: { // BIT 1, D
            BIT(D, 1);
            break;
        } case 0x4B: { // BIT 1, E
            BIT(E, 1);
            break;
        } case 0x4C: { // BIT 1, H
            BIT(H, 1);
            break;
        } case 0x4D: { // BIT 1, L
            BIT(L, 1);
            break;
        } case 0x4E: { // BIT 1, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            BIT(value, 1);
            mmu->write_byte(HL, value);
            break;
        } case 0x4F: { // BIT 1, A
            BIT(A, 1);
            break;
        } case 0x50: { // BIT 2, B
            BIT(B, 2);
            break;
        } case 0x51: { // BIT 2, C
            BIT(C, 2);
            break;
        } case 0x52: { // BIT 2, D
            BIT(D, 2);
            break;
        } case 0x53: { // BIT 2, E
            BIT(E, 2);
            break;
        } case 0x54: { // BIT 2, H
            BIT(H, 2);
            break;
        } case 0x55: { // BIT 2, L
            BIT(L, 2);
            break;
        } case 0x56: { // BIT 2, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            BIT(value, 2);
            mmu->write_byte(HL, value);
            break;
        } case 0x57: { // BIT 2, A
            BIT(A, 2);
            break;
        } case 0x58: { // BIT 3, B
            BIT(B, 3);
            break;
        } case 0x59: { // BIT 3, C
            BIT(C, 3);
            break;
        } case 0x5A: { // BIT 3, D
            BIT(D, 3);
            break;
        } case 0x5B: { // BIT 3, E
            BIT(E, 3);
            break;
        } case 0x5C: { // BIT 3, H
            BIT(H, 3);
            break;
        } case 0x5D: { // BIT 3, L
            BIT(L, 3);
            break;
        } case 0x5E: { // BIT 3, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            BIT(value, 3);
            mmu->write_byte(HL, value);
            break;
        } case 0x5F: { // BIT 3, A
            BIT(A, 3);
            break;
        } case 0x60: { // BIT 4, B
            BIT(B, 4);
            break;
        } case 0x61: { // BIT 4, C
            BIT(C, 4);
            break;
        } case 0x62: { // BIT 4, D
            BIT(D, 4);
            break;
        } case 0x63: { // BIT 4, E
            BIT(E, 4);
            break;
        } case 0x64: { // BIT 4, H
            BIT(H, 4);
            break;
        } case 0x65: { // BIT 4, L
            BIT(L, 4);
            break;
        } case 0x66: { // BIT 4, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            BIT(value, 4);
            mmu->write_byte(HL, value);
            break;
        } case 0x67: { // BIT 4, A
            BIT(A, 4);
            break;
        } case 0x68: { // BIT 5, B
            BIT(B, 5);
            break;
        } case 0x69: { // BIT 5, C
            BIT(C, 5);
            break;
        } case 0x6A: { // BIT 5, D
            BIT(D, 5);
            break;
        } case 0x6B: { // BIT 5, E
            BIT(E, 5);
            break;
        } case 0x6C: { // BIT 5, H
            BIT(H, 5);
            break;
        } case 0x6D: { // BIT 5, L
            BIT(L, 5);
            break;
        } case 0x6E: { // BIT 5, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            BIT(value, 5);
            mmu->write_byte(HL, value);
            break;
        } case 0x6F: { // BIT 5, A
            BIT(A, 5);
            break;
        } case 0x70: { // BIT 6, B
            BIT(B, 6);
            break;
        } case 0x71: { // BIT 6, C
            BIT(C, 6);
            break;
        } case 0x72: { // BIT 6, D
            BIT(D, 6);
            break;
        } case 0x73: { // BIT 6, E
            BIT(E, 6);
            break;
        } case 0x74: { // BIT 6, H
            BIT(H, 6);
            break;
        } case 0x75: { // BIT 6, L
            BIT(L, 6);
            break;
        } case 0x76: { // BIT 6, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            BIT(value, 6);
            mmu->write_byte(HL, value);
            break;
        } case 0x77: { // BIT 6, A
            BIT(A, 6);
            break;
        } case 0x78: { // BIT 7, B
            BIT(B, 7);
            break;
        } case 0x79: { // BIT 7, C
            BIT(C, 7);
            break;
        } case 0x7A: { // BIT 7, D
            BIT(D, 7);
            break;
        } case 0x7B: { // BIT 7, E
            BIT(E, 7);
            break;
        } case 0x7C: { // BIT 7, H
            BIT(H, 7);
            break;
        } case 0x7D: { // BIT 7, L
            BIT(L, 7);
            break;
        } case 0x7E: { // BIT 7, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            BIT(value, 7);
            mmu->write_byte(HL, value);
            break;
        } case 0x7F: { // BIT 7, A
            BIT(A, 7);
            break;
        } case 0x80: { // RES 0, B
            RES(B, 0);
            break;
        } case 0x81: { // RES 0, C
            RES(C, 0);
            break;
        } case 0x82: { // RES 0, D
            RES(D, 0);
            break;
        } case 0x83: { // RES 0, E
            RES(E, 0);
            break;
        } case 0x84: { // RES 0, H
            RES(H, 0);
            break;
        } case 0x85: { // RES 0, L
            RES(L, 0);
            break;
        } case 0x86: { // RES 0, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            RES(value, 0);
            mmu->write_byte(HL, value);
            break;
        } case 0x87: { // RES 0, A
            RES(A, 0);
            break;
        } case 0x88: { // RES 1, B
            RES(B, 1);
            break;
        } case 0x89: { // RES 1, C
            RES(C, 1);
            break;
        } case 0x8A: { // RES 1, D
            RES(D, 1);
            break;
        } case 0x8B: { // RES 1, E
            RES(E, 1);
            break;
        } case 0x8C: { // RES 1, H
            RES(H, 1);
            break;
        } case 0x8D: { // RES 1, L
            RES(L, 1);
            break;
        } case 0x8E: { // RES 1, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            RES(value, 1);
            mmu->write_byte(HL, value);
            break;
        } case 0x8F: { // RES 1, A
            RES(A, 1);
            break;
        } case 0x90: { // RES 2, B
            RES(B, 2);
            break;
        } case 0x91: { // RES 2, C
            RES(C, 2);
            break;
        } case 0x92: { // RES 2, D
            RES(D, 2);
            break;
        } case 0x93: { // RES 2, E
            RES(E, 2);
            break;
        } case 0x94: { // RES 2, H
            RES(H, 2);
            break;
        } case 0x95: { // RES 2, L
            RES(L, 2);
            break;
        } case 0x96: { // RES 2, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            RES(value, 2);
            mmu->write_byte(HL, value);
            break;
        } case 0x97: { // RES 2, A
            RES(A, 2);
            break;
        } case 0x98: { // RES 3, B
            RES(B, 3);
            break;
        } case 0x99: { // RES 3, C
            RES(C, 3);
            break;
        } case 0x9A: { // RES 3, D
            RES(D, 3);
            break;
        } case 0x9B: { // RES 3, E
            RES(E, 3);
            break;
        } case 0x9C: { // RES 3, H
            RES(H, 3);
            break;
        } case 0x9D: { // RES 3, L
            RES(L, 3);
            break;
        } case 0x9E: { // RES 3, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            RES(value, 3);
            mmu->write_byte(HL, value);
            break;
        } case 0x9F: { // RES 3, A
            RES(A, 3);
            break;
        } case 0xA0: { // RES 4, B
            RES(B, 4);
            break;
        } case 0xA1: { // RES 4, C
            RES(C, 4);
            break;
        } case 0xA2: { // RES 4, D
            RES(D, 4);
            break;
        } case 0xA3: { // RES 4, E
            RES(E, 4);
            break;
        } case 0xA4: { // RES 4, H
            RES(H, 4);
            break;
        } case 0xA5: { // RES 4, L
            RES(L, 4);
            break;
        } case 0xA6: { // RES 4, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            RES(value, 4);
            mmu->write_byte(HL, value);
            break;
        } case 0xA7: { // RES 4, A
            RES(A, 4);
            break;
        } case 0xA8: { // RES 5, B
            RES(B, 5);
            break;
        } case 0xA9: { // RES 5, C
            RES(C, 5);
            break;
        } case 0xAA: { // RES 5, D
            RES(D, 5);
            break;
        } case 0xAB: { // RES 5, E
            RES(E, 5);
            break;
        } case 0xAC: { // RES 5, H
            RES(H, 5);
            break;
        } case 0xAD: { // RES 5, L
            RES(L, 5);
            break;
        } case 0xAE: { // RES 5, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            RES(value, 5);
            mmu->write_byte(HL, value);
            break;
        } case 0xAF: { // RES 5, A
            RES(A, 5);
            break;
        } case 0xB0: { // RES 6, B
            RES(B, 6);
            break;
        } case 0xB1: { // RES 6, C
            RES(C, 6);
            break;
        } case 0xB2: { // RES 6, D
            RES(D, 6);
            break;
        } case 0xB3: { // RES 6, E
            RES(E, 6);
            break;
        } case 0xB4: { // RES 6, H
            RES(H, 6);
            break;
        } case 0xB5: { // RES 6, L
            RES(L, 6);
            break;
        } case 0xB6: { // RES 6, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            RES(value, 6);
            mmu->write_byte(HL, value);
            break;
        } case 0xB7: { // RES 6, A
            RES(A, 6);
            break;
        } case 0xB8: { // RES 7, B
            RES(B, 7);
            break;
        } case 0xB9: { // RES 7, C
            RES(C, 7);
            break;
        } case 0xBA: { // RES 7, D
            RES(D, 7);
            break;
        } case 0xBB: { // RES 7, E
            RES(E, 7);
            break;
        } case 0xBC: { // RES 7, H
            RES(H, 7);
            break;
        } case 0xBD: { // RES 7, L
            RES(L, 7);
            break;
        } case 0xBE: { // RES 7, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            RES(value, 7);
            mmu->write_byte(HL, value);
            break;
        } case 0xBF: { // RES 7, A
            RES(A, 7);
            break;
        } case 0xC0: { // SET 0, B
            SET(B, 0);
            break;
        } case 0xC1: { // SET 0, C
            SET(C, 0);
            break;
        } case 0xC2: { // SET 0, D
            SET(D, 0);
            break;
        } case 0xC3: { // SET 0, E
            SET(E, 0);
            break;
        } case 0xC4: { // SET 0, H
            SET(H, 0);
            break;
        } case 0xC5: { // SET 0, L
            SET(L, 0);
            break;
        } case 0xC6: { // SET 0, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            SET(value, 0);
            mmu->write_byte(HL, value);
            break;
        } case 0xC7: { // SET 0, A
            SET(A, 0);
            break;
        } case 0xC8: { // SET 1, B
            SET(B, 1);
            break;
        } case 0xC9: { // SET 1, C
            SET(C, 1);
            break;
        } case 0xCA: { // SET 1, D
            SET(D, 1);
            break;
        } case 0xCB: { // SET 1, E
            SET(E, 1);
            break;
        } case 0xCC: { // SET 1, H
            SET(H, 1);
            break;
        } case 0xCD: { // SET 1, L
            SET(L, 1);
            break;
        } case 0xCE: { // SET 1, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            SET(value, 1);
            mmu->write_byte(HL, value);
            break;
        } case 0xCF: { // SET 1, A
            SET(A, 1);
            break;
        } case 0xD0: { // SET 2, B
            SET(B, 2);
            break;
        } case 0xD1: { // SET 2, C
            SET(C, 2);
            break;
        } case 0xD2: { // SET 2, D
            SET(D, 2);
            break;
        } case 0xD3: { // SET 2, E
            SET(E, 2);
            break;
        } case 0xD4: { // SET 2, H
            SET(H, 2);
            break;
        } case 0xD5: { // SET 2, L
            SET(L, 2);
            break;
        } case 0xD6: { // SET 2, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            SET(value, 2);
            mmu->write_byte(HL, value);
            break;
        } case 0xD7: { // SET 2, A
            SET(A, 2);
            break;
        } case 0xD8: { // SET 3, B
            SET(B, 3);
            break;
        } case 0xD9: { // SET 3, C
            SET(C, 3);
            break;
        } case 0xDA: { // SET 3, D
            SET(D, 3);
            break;
        } case 0xDB: { // SET 3, E
            SET(E, 3);
            break;
        } case 0xDC: { // SET 3, H
            SET(H, 3);
            break;
        } case 0xDD: { // SET 3, L
            SET(L, 3);
            break;
        } case 0xDE: { // SET 3, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            SET(value, 3);
            mmu->write_byte(HL, value);
            break;
        } case 0xDF: { // SET 3, A
            SET(A, 3);
            break;
        } case 0xE0: { // SET 4, B
            SET(B, 4);
            break;
        } case 0xE1: { // SET 4, C
            SET(C, 4);
            break;
        } case 0xE2: { // SET 4, D
            SET(D, 4);
            break;
        } case 0xE3: { // SET 4, E
            SET(E, 4);
            break;
        } case 0xE4: { // SET 4, H
            SET(H, 4);
            break;
        } case 0xE5: { // SET 4, L
            SET(L, 4);
            break;
        } case 0xE6: { // SET 4, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            SET(value, 4);
            mmu->write_byte(HL, value);
            break;
        } case 0xE7: { // SET 4, A
            SET(A, 4);
            break;
        } case 0xE8: { // SET 5, B
            SET(B, 5);
            break;
        } case 0xE9: { // SET 5, C
            SET(C, 5);
            break;
        } case 0xEA: { // SET 5, D
            SET(D, 5);
            break;
        } case 0xEB: { // SET 5, E
            SET(E, 5);
            break;
        } case 0xEC: { // SET 5, H
            SET(H, 5);
            break;
        } case 0xED: { // SET 5, L
            SET(L, 5);
            break;
        } case 0xEE: { // SET 5, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            SET(value, 5);
            mmu->write_byte(HL, value);
            break;
        } case 0xEF: { // SET 5, A
            SET(A, 5);
            break;
        } case 0xF0: { // SET 6, B
            SET(B, 6);
            break;
        } case 0xF1: { // SET 6, C
            SET(C, 6);
            break;
        } case 0xF2: { // SET 6, D
            SET(D, 6);
            break;
        } case 0xF3: { // SET 6, E
            SET(E, 6);
            break;
        } case 0xF4: { // SET 6, H
            SET(H, 6);
            break;
        } case 0xF5: { // SET 6, L
            SET(L, 6);
            break;
        } case 0xF6: { // SET 6, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            SET(value, 6);
            mmu->write_byte(HL, value);
            break;
        } case 0xF7: { // SET 6, A
            SET(A, 6);
            break;
        } case 0xF8: { // SET 7, B
            SET(B, 7);
            break;
        } case 0xF9: { // SET 7, C
            SET(C, 7);
            break;
        } case 0xFA: { // SET 7, D
            SET(D, 7);
            break;
        } case 0xFB: { // SET 7, E
            SET(E, 7);
            break;
        } case 0xFC: { // SET 7, H
            SET(H, 7);
            break;
        } case 0xFD: { // SET 7, L
            SET(L, 7);
            break;
        } case 0xFE: { // SET 7, [HL]
            uint16_t HL = (H << 8) | L;
            uint8_t value = mmu->read_byte(HL);
            SET(value, 7);
            mmu->write_byte(HL, value);
            break;
        } case 0xFF: { // SET 7, A
            SET(A, 7);
            break;
        }
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

void CPU::RLC(uint8_t& register1) {
    setCarryFlag(register1 & (1 << 7));
    uint8_t bit = (register1 >> 7) & 0x01;
    register1 <<= 1;
    register1 += bit;
    setZeroFlag(register1 == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}
void CPU::RRC(uint8_t& register1) {
    uint carry = getCarryFlag();
    setCarryFlag(register1 & 0x01);
    register1 >>= 1;
    register1 |= (carry << 7);
    setZeroFlag(register1 == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}
void CPU::RL(uint8_t& register1) {
    int carry = getCarryFlag();
    setCarryFlag(register1 & (1 << 7));

    register1 = (register1 << 1) | carry;
    setZeroFlag(register1 == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}
void CPU::RR(uint8_t& register1) {
    int carry = getCarryFlag();
    setCarryFlag(register1 & 0x01);
    register1 >>= 1;
    register1 |= (carry << 7);
    setZeroFlag(register1 == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}
void CPU::SLA(uint8_t& register1) {
    setCarryFlag(register1 & (1 << 7));
    register1 <<= 1;
    setZeroFlag(register1 == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}
void CPU::SRA(uint8_t& register1) {
    setCarryFlag(register1 & 0x01);
    int bit = register1 & (1 << 7);
    register1 >>= 1;
    register1 |= bit;
    setZeroFlag(register1 == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}
void CPU::SWAP(uint8_t& register1) {
    uint8_t lower = register1 << 4;
    register1 = (register1 >> 4) | lower;
    setZeroFlag(register1 == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(false);
}
void CPU::SRL(uint8_t& register1) {
    setCarryFlag(register1 & 0x01);
    register1 >>= 1;
    setZeroFlag(register1 == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}
void CPU::BIT(uint8_t& register1, uint8_t bit) {
    setZeroFlag(!(register1 & bit));
    setSubtractFlag(false);
    setHalfCarryFlag(true);
}
void CPU::RES(uint8_t& register1, uint8_t bit) {
    register1 &= ~(bit);
}
void CPU::SET(uint8_t& register1, uint8_t bit) {
    register1 |= bit; 
}