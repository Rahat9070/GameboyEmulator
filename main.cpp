#include "CPU/CPU.h"
#include <iostream>

int main() {
    CPU cpu;
    uint8_t opcode = 0x15;
    std::cout << cpu.D << "\n";
    cpu.decodeAndExecute(opcode);
    std::cout << cpu.D << "\n";
    return 0;
}