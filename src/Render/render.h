#pragma once

#include <SDL2/SDL.h>

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init(const char* title, int width, int height);
    void clear();
    void present();
    void drawRect(int x, int y, int width, int height, SDL_Color color);
    void cleanup();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
};