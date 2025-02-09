// filepath: /home/rahat/6CCS3PRJ/GameboyEmulator/main.cpp
#include "src/Render/render.h"
#include <iostream>

int main() {
    Renderer renderer;
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

        renderer.clear();
        renderer.drawRect(50, 50, 100, 100, {255, 0, 0, 255});
        renderer.present();
    }

    renderer.cleanup();
    return 0;
}