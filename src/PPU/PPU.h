#pragma once

#include "CPU/CPU.h"
#include "MMU/MMU.h"
#include "structs.h"

class PPU {

    public:
        CPU *cpu;
        MMU *mmu;
        uint32_t ticks;
        uint8_t *scrollX;
        uint8_t *scrollY;
        uint8_t *scanline;
        int mode = 0;
        
        uint8_t background_display = 1;
        uint8_t sprite_display_enable = 1;
        uint8_t sprite_size = 1;
        uint8_t bg_display_select = 1;
        uint8_t bg_window_data_select = 1;
        uint8_t window_enable = 1;
        uint8_t window_display_select = 0;
        uint8_t lcd_enable = 1;
    
        uint8_t mode_flag = 0;
        uint8_t coincidence_flag = 0;
        uint8_t hblank_interrupt = 0;
        uint8_t vblank_interrupt = 0;
        uint8_t oam_interrupt = 0;
        uint8_t coincidence_interrupt = 0;
    
        GBColour framebuffer[160 * 144];
        uint8_t background[32 * 32];
    
        int modeclock = 0;
    
        bool can_render = false;
    
        void step(int cycles);
        void compare_ly_lyc();
        PPU(CPU *cpu, MMU *mmu);
        void render_scanline();
        void render_background(bool* rows);
        void render_sprites(bool* rows);
        void render_window();
};