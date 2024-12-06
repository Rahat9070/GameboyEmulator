#include "CPU/CPU.h"
#include <iostream>

int main() {
    CPU cpu;
    uint8_t opcode = 0xD6;
   cpu.decodeAndExecute(opcode);
    return 0;
}