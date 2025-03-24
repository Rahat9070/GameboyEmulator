#pragma once

#include "PPU/PPU.h"
#include "MMU/MMU.h"
#include "CPU/CPU.h"
#include "structs.h"

#include <array>
#include <chrono>
#include <thread>

#include <SDL2/SDL.h>

class Renderer {
public:
    CPU *cpu;
    MMU *mmu;
    PPU *ppu;
    Renderer(CPU *cpu, PPU *ppu, MMU *mmu);

    bool init(const char* title, int width, int height);
    void clear();
    void present();
    void drawRect(int x, int y, int width, int height, SDL_Color color);
    void cleanup();
    void draw();
    void render();

    int gb_width = 160;
    int gb_height = 144;

    int window_width = gb_width * 2;
    int window_height = gb_height * 2;

    std::array<uint8_t, 92160> view_pixels;
    SDL_Rect view_rect = {0, 0, window_width, window_height};
    SDL_Texture* texture;

    int framerate = 1000 / 60;
    std::chrono::steady_clock::time_point start_frame;
    std::chrono::steady_clock::time_point end_frame;
    void check_framerate();


private:
    SDL_Window* window;
    SDL_Renderer* renderer;
};