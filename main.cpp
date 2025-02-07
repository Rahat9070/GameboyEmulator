#include "src/CPU/CPU.h"
#include "src/Cartridge/cartridge.h"
#include "src/MBC/MBC.h"
#include "src/MMU/MMU.h"
#include <iostream>

int main() {
    CPU cpu;
    uint8_t opcode = 0xD6;
    cpu.decodeAndExecute(opcode);
    return 0;
}