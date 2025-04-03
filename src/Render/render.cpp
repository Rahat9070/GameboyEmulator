#include "render.h"
#include <iostream>

Renderer::Renderer(CPU *cpu, PPU *ppu, MMU *mmu) : window(nullptr), renderer(nullptr) {
    this->cpu = cpu;
    this->ppu = ppu;
    this->mmu = mmu;
}

bool Renderer::init(const char* title, int width, int height) {
    view_pixels.fill(0xFF);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(window_width, window_height, 0, &window, &renderer);
    SDL_RenderSetLogicalSize(renderer, window_width, window_height);
    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_SetWindowTitle(window, title);
    
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

void Renderer::clear() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void Renderer::present() {
    SDL_RenderPresent(renderer);
}

void Renderer::drawRect(int x, int y, int width, int height, SDL_Color color) {
    SDL_Rect rect = { x, y, width, height };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void Renderer::cleanup() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
}

void Renderer::check_framerate() {
    end_frame = std::chrono::steady_clock::now();
    int elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_frame - start_frame).count();
    if (elapsed_time < framerate) {
        std::this_thread::sleep_for(std::chrono::milliseconds(framerate - elapsed_time));
    }
    start_frame = std::chrono::steady_clock::now();
}

void Renderer::draw(){
    for (int i = 0; i < 144 * 160; i++) {
        Colour colour = ppu->framebuffer[i];
        std::copy(colour.colours, colour.colours + 4, view_pixels.begin() + i * 4);
    }
    SDL_UpdateTexture(texture, NULL, view_pixels.data(), gb_width * 2);
}

void Renderer::render() {
    check_framerate();

    SDL_SetTextureColorMod(texture, 224, 219, 205);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, texture);
    draw();
    SDL_RenderCopy(renderer, texture, NULL, &view_rect);
    SDL_RenderPresent(renderer);
}