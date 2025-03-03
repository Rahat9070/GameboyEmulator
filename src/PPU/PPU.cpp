#include "PPU.h"

PPU::PPU(CPU *cpu, MMU *mmu) {
    this->cpu = cpu;
    this->mmu = mmu;
    ticks = 0;
}

void PPU::render_scanline() {
    render_background();
    render_sprites();
    render_window();
}

void PPU::render_background() {
    uint16_t address = 0x9800;

    address += ((*scrollY + *scanline) / 8 * 32) % (32 * 32);

    uint16_t start = address;
    uint16_t end = address + 32;

    int x = *scrollX & 7;
    int y = (*scrollY + *scanline) & 7;
    int offset = *scanline * 160;

    int pixel = 0;
    for (int i = 0; i < 21;  i++) {
        uint16_t tile_address = address + i;
        uint8_t tile = mmu->read_byte(tile_address);
        
        for (x; x < 0; x++) {
            if (pixel >= 160) {
                break;
            }
            int colour = mmu->read_byte(0x8000 + tile * 16 + y * 2);
            // framebuffer[offset + pixel] = mmu->colours[colour];
            pixel++;
        }
        x = 0;
    }
}

void PPU::render_sprites() {

}

void PPU::PPU::render_window() {
    if (control->spriteDisplayEnable == 0) {
        return;
    }
    if (mmu->read_byte(0xFF40) > *scanline) {
        return;
    }
    uint16_t address = 0x9800;
    if (control->windowDisplaySelect == 1) {
        address = 0x9C00;
    }
    address += ((*scanline - mmu->read_byte(0xFF4A)) / 8 * 32) * 32;
    int y = (*scanline - mmu->read_byte(0xFF4A)) & 7;
    int x = 0;
    int offset = *scanline * 160 + (mmu->read_byte(0xFF4B) - 7);
    uint16_t tile_address = address;
    for (tile_address; tile_address < address + 20; tile_address++) {
        int tile = mmu->read_byte(tile_address);

        for (x; x < 8; x++) {
            if (offset > sizeof(framebuffer)) {
                break;
            }
            // framebuffer[offset] =  mmu->palette_BGP[colour];
        }
        x = 0;
    }
}