// filepath: /home/rahat/6CCS3PRJ/GameboyEmulator/main.cpp
#include "src/Render/render.h"
#include "src/CPU/CPU.h"
#include "src/Scheduler/scheduler.h"
#include "src/Cartridge/cartridge.h"
#include "src/MMU/MMU.h"

#include <iostream>
#include <string>

bool isValidROMFile(const std::string& filename) {
    return filename.size() > 3 && filename.substr(filename.size() - 3) == ".gb";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_rom_file>" << std::endl;
        return -1;
    }
    std::string directory = argv[1];
    if (!isValidROMFile(directory)) {
        std::cerr << "Error: The file must have a .gb extension" << std::endl;
        return -1;
    }
    Renderer renderer;
    Cartridge cartridge(directory);
    MMU mmu(&cartridge);
    Scheduler scheduler(&mmu);
    CPU cpu(mmu, scheduler);
    if (!renderer.init("Gameboy Emulator", 640, 480)) {
        return -1;
    }

    bool running = true;
    SDL_Event event;

    mmu.load_game_rom(directory);
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        cpu.step();
        renderer.clear();
        renderer.drawRect(50, 50, 100, 100, {255, 0, 0, 255});
        renderer.present();
    }

    renderer.cleanup();
    return 0;
}