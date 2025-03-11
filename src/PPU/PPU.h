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

        struct Control {
            union {
                struct {
                    uint8_t bgDisplay : 1;
                    uint8_t spriteDisplayEnable : 1;
                    uint8_t spriteSize : 1;  // True means 8x16 tiles
                    uint8_t bgDisplaySelect : 1;
                    uint8_t bgWindowDataSelect : 1;
                    uint8_t windowEnable : 1;
                    uint8_t windowDisplaySelect : 1;
                    uint8_t lcdEnable : 1;
                };
            };
        } *control;
    
        struct Stat {
            union {
                struct {
                    uint8_t mode_flag : 2;
                    uint8_t coincidence_flag : 1;
                    uint8_t hblank_interrupt : 1;
                    uint8_t vblank_interrupt : 1;
                    uint8_t oam_interrupt : 1;
                    uint8_t coincidence_interrupt : 1;
                };
            };
        } *stat;
    
        Colour framebuffer[160 * 144];
        uint8_t background[32 * 32];
    
        int modeclock = 0;
    
        bool can_render = false;
    
        void step(int cycles);
        void compare_ly_lyc();
        PPU(CPU *cpu, MMU *mmu);
        void render_scanline();
        void render_background();
        void render_sprites(bool* rows);
        void render_window();
};