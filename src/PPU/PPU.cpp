#include "PPU.h"

PPU::PPU(CPU *cpu, MMU *mmu) {
    this->cpu = cpu;
    this->mmu = mmu;
    ticks = 0;
}

void PPU::render_scanline() {
    bool rows[160] = {0};
    render_background();
    render_sprites(rows);
    render_window();
}

void PPU::step(int cycles) {
    modeclock += cycles;

    if (!control->lcdEnable) {
        mode = 0;
        if (modeclock >= 70224)
            modeclock -= 70224;
        return;
    }

    switch (mode) {
        case 0:  // HBLANK
            if (modeclock >= 204) {
                modeclock -= 204;
                mode = 2;

                *scanline += 1;
                uint8_t lyc = mmu->read_byte(0xFF45);
                stat->coincidence_flag = int(lyc == *scanline);

                if (lyc == *scanline && stat->coincidence_interrupt)
                    mmu->set_interrupt_flag(mmu->LCD);

                if (*scanline == 144) {
                    mode = 1;
                    can_render = true;
                    mmu->set_interrupt_flag(mmu->VBLANK);
                    if (stat->vblank_interrupt)
                        mmu->set_interrupt_flag(mmu->LCD);
                } else if (stat->oam_interrupt)
                    mmu->set_interrupt_flag(mmu->LCD);

                mmu->write_byte(0xff41, (mmu->read_byte(0xff41) & 0xFC) | (mode & 3));
            }
            break;
        case 1:  // VBLANK
            if (modeclock >= 456) {
                modeclock -= 456;
                *scanline += 1;
                uint8_t lyc = mmu->read_byte(0xFF45);
                stat->coincidence_flag = int(lyc == *scanline);

                if (lyc == *scanline && stat->coincidence_interrupt)
                    mmu->set_interrupt_flag(mmu->LCD);
                if (*scanline == 153) {
                    *scanline = 0;
                    mode = 2;
                    mmu->write_byte(0xff41, (mmu->read_byte(0xff41) & 0xFC) | (mode & 3));
                    if (stat->oam_interrupt)
                        mmu->set_interrupt_flag(mmu->LCD);
                }
            }

            break;
        case 2:  // OAM
            if (modeclock >= 80) {
                modeclock -= 80;
                mode = 3;
                mmu->write_byte(0xff41, (mmu->read_byte(0xff41) & 0xFC) | (mode & 3));
            }
            break;
        case 3:  // VRAM
            if (modeclock >= 172) {
                modeclock -= 172;
                mode = 0;
                render_scanline();
                mmu->write_byte(0xff41, (mmu->read_byte(0xff41) & 0xFC) | (mode & 3));

                if (stat->hblank_interrupt)
                    mmu->set_interrupt_flag(mmu->LCD);
            }
            break;
    }
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
            framebuffer[offset + pixel] = mmu->colour[colour];
            pixel++;
        }
        x = 0;
    }
}

void PPU::render_sprites(bool* rows) {
    int sprite_height = control->spriteSize == 0 ? 8 : 16;
    bool visible_sprites[40] = {false};
    int sprite_rows = 0;

    for (int i = 0; i < 40; i++) {
        Sprite sprite = mmu->sprites[i];
        if (!sprite.ready) {
            visible_sprites[i] = false;
            continue;
        }
        if ((sprite.y > *scanline) || ((sprite.y + sprite_height) <= *scanline)) {
            visible_sprites[i] = false;
            continue;
        }
        if (sprite.y <= *scanline && sprite.y + sprite_height > *scanline) {
            visible_sprites[sprite_rows] = true;
            sprite_rows++;
        }
        sprite_rows++;
        visible_sprites[i] = sprite_rows <= 10;
    }

    for (int i = 39; i >= 0; i--) {
        if (!visible_sprites[i])
            continue;

        Sprite sprite = mmu->sprites[i];

        if ((sprite.x < -7) || (sprite.x >= 160))
            continue;

        // Flip vertically
        int pixel_y = *scanline - sprite.y;
        pixel_y = sprite.options.yFlip ? (7 + 8 * control->spriteSize) - pixel_y : pixel_y;

        for (int x = 0; x < 8; x++) {
            int tile_num = sprite.tile & (control->spriteSize ? 0xFE : 0xFF);
            int colour = 0;

            int x_temp = sprite.x + x;
            if (x_temp < 0 || x_temp >= 160)
                continue;

            int pixelOffset = *this->scanline * 160 + x_temp;

            // Flip horizontally
            uint8_t pixel_x = sprite.options.xFlip ? 7 - x : x;

            if (control->spriteSize && (pixel_y >= 8))
                colour = mmu->tiles[tile_num + 1].pixels[pixel_y - 8][pixel_x];
            else
                colour = mmu->tiles[tile_num].pixels[pixel_y][pixel_x];

            // Black is transparent
            if (!colour)
                continue;

            if (!rows[x_temp] || !sprite.options.renderPriority)
                framebuffer[pixelOffset] = sprite.colourPalette[colour];
        }
    }
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
                continue;
            }
            int colour = mmu->tiles[tile].pixels[y][x];
            framebuffer[offset] =  mmu->palette_BGP[colour];
        }
        x = 0;
    }
}