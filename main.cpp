// filepath: /home/rahat/6CCS3PRJ/GameboyEmulator/main.cpp
#include "src/Render/render.h"
#include "src/CPU/CPU.h"
#include "src/Cartridge/cartridge.h"
#include "src/MMU/MMU.h"

#include <iostream>
#include <string>

int main() {
    std::string directory = "/home/rahat/6CCS3PRJ/GameboyEmulator/ROMs/cpu_instrs.gb";
    Renderer renderer;
    Cartridge cartridge(directory);
    MMU mmu(&cartridge);
    CPU cpu(mmu);
    if (!renderer.init("Gameboy Emulator", 640, 480)) {
        return -1;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        mmu.load_game_rom(directory);
        cpu.executeInstruction();
        renderer.clear();
        renderer.drawRect(50, 50, 100, 100, {255, 0, 0, 255});
        renderer.present();
    }

    renderer.cleanup();
    return 0;
}