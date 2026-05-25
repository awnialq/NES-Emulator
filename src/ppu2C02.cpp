#include "ppu2C02.h"
#include <cstring>

using ppu = ppu2C02;

ppu::ppu2C02(){
    std::memset(nameTables, 0, sizeof(nameTables));
    std::memset(paletteTable, 0, sizeof(paletteTable));
    std::memset(patternMemory, 0, sizeof(patternMemory));

    // NES master palette (RGB packed as 0xAARRGGBB)
    systemPalette = {
        0xFF757575, 0xFF271B8F, 0xFF0000AB, 0xFF47009F, 0xFF8F0077, 0xFFAB0013, 0xFFA70000, 0xFF7F0B00,
        0xFF432F00, 0xFF004700, 0xFF005100, 0xFF003F17, 0xFF1B3F5F, 0xFF000000, 0xFF000000, 0xFF000000,
        0xFFBCBCBC, 0xFF0073EF, 0xFF233BEF, 0xFF8300F3, 0xFFBF00BF, 0xFFE7005B, 0xFFDB2B00, 0xFFCB4F0F,
        0xFF8B7300, 0xFF009700, 0xFF00AB00, 0xFF00933B, 0xFF00838B, 0xFF000000, 0xFF000000, 0xFF000000,
        0xFFFFFFFF, 0xFF3FBFFF, 0xFF5F97FF, 0xFFA78BFD, 0xFFF77BFF, 0xFFFF77B7, 0xFFFF7763, 0xFFFF9B3B,
        0xFFF3BF3F, 0xFF83D313, 0xFF4FDF4B, 0xFF58F898, 0xFF00EBDB, 0xFF000000, 0xFF000000, 0xFF000000,
        0xFFFFFFFF, 0xFFABE7FF, 0xFFC7D7FF, 0xFFD7CBFF, 0xFFFFC7FF, 0xFFFFC7DB, 0xFFFFBFB3, 0xFFFFDBAB,
        0xFFFFE7A3, 0xFFE3FFA3, 0xFFABF3BF, 0xFFB3FFCF, 0xFF9FFFF3, 0xFF000000, 0xFF000000, 0xFF000000
    };
}

ppu::~ppu2C02() = default;

void ppu::connectCart(cartridge *c){
    cart = c;
}

const std::array<uint32_t, 256 * 240> &ppu::getFrameBuffer() const{
    return frameBuffer;
}

uint8_t ppu::cpuRead(uint16_t addr, bool){
    uint8_t data = 0x00;
    addr &= 0x0007;

    switch(addr){
        case 0x0002: // PPUSTATUS
            data = (status & 0xE0) | (ppuDataBuffer & 0x1F);
            status &= ~0x80;
            addressLatch = 0;
            break;
        case 0x0004: // OAMDATA
            data = oam[oamAddr];
            break;
        case 0x0007: { // PPUDATA
            data = ppuDataBuffer;
            ppuDataBuffer = ppuRead(vramAddr, false);
            if(vramAddr >= 0x3F00){
                // Palette reads are immediate
                data = ppuDataBuffer;
            }
            vramAddr += (control & 0x04) ? 32 : 1;
            break;
        }
        default:
            break;
    }
    return data;
}

void ppu::cpuWrite(uint16_t addr, uint8_t data){
    addr &= 0x0007;
    switch(addr){
        case 0x0000: { // PPUCTRL
            uint8_t oldNmi = control & 0x80;
            control = data;
            tramAddr = (tramAddr & 0xF3FF) | ((static_cast<uint16_t>(data & 0x03)) << 10);
            if (!oldNmi && (control & 0x80) && (status & 0x80)) {
                nmi = true;
            }
            break;
        }
        case 0x0001: // PPUMASK
            mask = data;
            break;
        case 0x0003: // OAMADDR
            oamAddr = data;
            break;
        case 0x0004: // OAMDATA
            oam[oamAddr++] = data;
            break;
        case 0x0005: // PPUSCROLL
            if(addressLatch == 0){
                fineX = data & 0x07;
                tramAddr = (tramAddr & 0xFFE0) | (data >> 3);
                addressLatch = 1;
            }
            else{
                tramAddr = (tramAddr & 0x8FFF) | ((static_cast<uint16_t>(data & 0x07)) << 12);
                tramAddr = (tramAddr & 0xFC1F) | ((static_cast<uint16_t>(data & 0xF8)) << 2);
                addressLatch = 0;
            }
            break;
        case 0x0006: // PPUADDR
            if(addressLatch == 0){
                tramAddr = (tramAddr & 0x00FF) | ((static_cast<uint16_t>(data & 0x3F)) << 8);
                addressLatch = 1;
            }
            else{
                tramAddr = (tramAddr & 0xFF00) | data;
                vramAddr = tramAddr;
                addressLatch = 0;
            }
            break;
        case 0x0007: // PPUDATA
            ppuWrite(vramAddr, data);
            vramAddr += (control & 0x04) ? 32 : 1;
            break;
        default:
            break;
    }
}

uint8_t ppu::ppuRead(uint16_t addr, bool){
    uint8_t data = 0x00;
    addr &= 0x3FFF;

    if(cart && cart->ppuRead(addr, data)){
        return data;
    }

    if(addr <= 0x1FFF){
        data = patternMemory[addr];
    }
    else if(addr >= 0x2000 && addr <= 0x3EFF){
        addr &= 0x0FFF;
        if(cart && cart->mirror == cartridge::VERTICAL){
            if(addr <= 0x03FF) data = nameTables[0][addr & 0x03FF];
            else if(addr <= 0x07FF) data = nameTables[1][addr & 0x03FF];
            else if(addr <= 0x0BFF) data = nameTables[0][addr & 0x03FF];
            else data = nameTables[1][addr & 0x03FF];
        } else {
            if(addr <= 0x03FF) data = nameTables[0][addr & 0x03FF];
            else if(addr <= 0x07FF) data = nameTables[0][addr & 0x03FF];
            else if(addr <= 0x0BFF) data = nameTables[1][addr & 0x03FF];
            else data = nameTables[1][addr & 0x03FF];
        }
    }
    else if(addr >= 0x3F00 && addr <= 0x3FFF){
        uint16_t palAddr = addr & 0x001F;
        if(palAddr == 0x0010) palAddr = 0x0000;
        if(palAddr == 0x0014) palAddr = 0x0004;
        if(palAddr == 0x0018) palAddr = 0x0008;
        if(palAddr == 0x001C) palAddr = 0x000C;
        data = paletteTable[palAddr] & ((mask & 0x01) ? 0x30 : 0x3F);
    }

    return data;
}

void ppu::ppuWrite(uint16_t addr, uint8_t data){
    addr &= 0x3FFF;

    if(cart && cart->ppuWrite(addr, data)){
        return;
    }

    if(addr <= 0x1FFF){
        patternMemory[addr] = data;
    }
    else if(addr >= 0x2000 && addr <= 0x3EFF){
        addr &= 0x0FFF;
        if(cart && cart->mirror == cartridge::VERTICAL){
            if(addr <= 0x03FF) nameTables[0][addr & 0x03FF] = data;
            else if(addr <= 0x07FF) nameTables[1][addr & 0x03FF] = data;
            else if(addr <= 0x0BFF) nameTables[0][addr & 0x03FF] = data;
            else nameTables[1][addr & 0x03FF] = data;
        } else {
            if(addr <= 0x03FF) nameTables[0][addr & 0x03FF] = data;
            else if(addr <= 0x07FF) nameTables[0][addr & 0x03FF] = data;
            else if(addr <= 0x0BFF) nameTables[1][addr & 0x03FF] = data;
            else nameTables[1][addr & 0x03FF] = data;
        }
    }
    else if(addr >= 0x3F00 && addr <= 0x3FFF){
        uint16_t palAddr = addr & 0x001F;
        if(palAddr == 0x0010) palAddr = 0x0000;
        if(palAddr == 0x0014) palAddr = 0x0004;
        if(palAddr == 0x0018) palAddr = 0x0008;
        if(palAddr == 0x001C) palAddr = 0x000C;
        paletteTable[palAddr] = data;
    }
}

uint8_t ppu::readPalette(uint8_t palette, uint8_t pixel){
    return ppuRead(0x3F00 + (static_cast<uint16_t>(palette) << 2) + pixel, false) & 0x3F;
}

uint32_t ppu::getColorFromPaletteRam(uint8_t palette, uint8_t pixel){
    return systemPalette[readPalette(palette, pixel)];
}

void ppu::evaluateSprites() {
    activeSprites.clear();
    int count = 0;
    int height = (control & 0x20) ? 16 : 8;

    for (int i = 0; i < 64; i++) {
        uint8_t spriteY = oam[i * 4];
        int diff = scanline - spriteY;

        // In OAM, sprite Y is stored as screen Y - 1. So visible starting at Y+1.
        if (diff >= 1 && diff < 1 + height) {
            if (count < 8) {
                SpriteInfo sprite;
                sprite.index = static_cast<uint8_t>(i);
                sprite.y = spriteY;
                sprite.tileId = oam[i * 4 + 1];
                sprite.attributes = oam[i * 4 + 2];
                sprite.x = oam[i * 4 + 3];

                int row = diff - 1;
                if (sprite.attributes & 0x80) { // Vertical flip
                    row = (height - 1) - row;
                }

                uint16_t tileAddr = 0;
                if (height == 8) {
                    uint16_t patternBase = (control & 0x08) ? 0x1000 : 0x0000;
                    tileAddr = patternBase + static_cast<uint16_t>(sprite.tileId) * 16 + row;
                } else {
                    uint16_t patternBase = (sprite.tileId & 0x01) ? 0x1000 : 0x0000;
                    uint8_t tileIndex = sprite.tileId & 0xFE;
                    if (row < 8) {
                        tileAddr = patternBase + static_cast<uint16_t>(tileIndex) * 16 + row;
                    } else {
                        tileAddr = patternBase + static_cast<uint16_t>(tileIndex + 1) * 16 + (row - 8);
                    }
                }

                sprite.patternLsb = ppuRead(tileAddr, false);
                sprite.patternMsb = ppuRead(tileAddr + 8, false);
                activeSprites.push_back(sprite);
            }
            count++;
        }
    }

    if (count > 8) {
        status |= 0x20; // Sprite overflow flag
    }
}

void ppu::renderPixel() {
    const int x = cycle - 1;
    const int y = scanline;

    uint8_t bgPixel = 0;
    uint8_t bgPalette = 0;

    if (mask & 0x08) { // Background rendering enabled
        if (x >= 8 || (mask & 0x02)) { // Mask leftmost 8 pixels
            // Calculate absolute scrolled X and Y coordinates on the virtual screen
            const uint32_t absoluteX = (((vramAddr >> 10) & 0x01) * 256) + ((vramAddr & 0x001F) * 8) + fineX + x;
            
            const uint8_t scrolledCoarseX = (absoluteX / 8) & 0x1F;
            const uint8_t scrolledNametableX = (absoluteX / 256) & 0x01;
            const uint8_t scrolledFineX = absoluteX & 0x07;

            const uint8_t scrolledCoarseY = (vramAddr >> 5) & 0x001F;
            const uint8_t scrolledNametableY = (vramAddr >> 11) & 0x01;
            const uint8_t scrolledFineY = (vramAddr >> 12) & 0x07;

            const uint8_t nametable = (scrolledNametableY << 1) | scrolledNametableX;
            const uint16_t ntBase = 0x2000 + nametable * 0x0400;

            const uint16_t ntAddr = ntBase + scrolledCoarseY * 32 + scrolledCoarseX;
            const uint8_t tileId = ppuRead(ntAddr, false);

            const uint16_t patternBase = (control & 0x10) ? 0x1000 : 0x0000;
            const uint16_t tileAddr = patternBase + static_cast<uint16_t>(tileId) * 16 + scrolledFineY;

            const uint8_t lsb = ppuRead(tileAddr, false);
            const uint8_t msb = ppuRead(tileAddr + 8, false);
            const uint8_t shift = 7 - scrolledFineX;
            bgPixel = ((msb >> shift) & 0x01) << 1 | ((lsb >> shift) & 0x01);

            // Attribute table lookup for background palette select.
            const uint16_t attrAddr = (0x23C0 + nametable * 0x0400) + ((scrolledCoarseY >> 2) * 8) + (scrolledCoarseX >> 2);
            const uint8_t attrByte = ppuRead(attrAddr, false);
            const uint8_t attrShift = static_cast<uint8_t>(((scrolledCoarseY & 0x02) ? 4 : 0) + ((scrolledCoarseX & 0x02) ? 2 : 0));
            bgPalette = (attrByte >> attrShift) & 0x03;
        }
    }

    uint8_t fgPixel = 0;
    uint8_t fgPalette = 0;
    uint8_t fgPriority = 0;
    bool fgSpriteZero = false;

    if (mask & 0x10) { // Sprite rendering enabled
        if (x >= 8 || (mask & 0x04)) { // Mask leftmost 8 pixels
            for (const auto &sprite : activeSprites) {
                if (x >= sprite.x && x < sprite.x + 8) {
                    int shift = (sprite.attributes & 0x40) ? (x - sprite.x) : (7 - (x - sprite.x));
                    uint8_t pixel = ((sprite.patternMsb >> shift) & 0x01) << 1 | ((sprite.patternLsb >> shift) & 0x01);

                    if (pixel != 0) {
                        fgPixel = pixel;
                        fgPalette = sprite.attributes & 0x03;
                        fgPriority = (sprite.attributes & 0x20) >> 5;
                        fgSpriteZero = (sprite.index == 0);
                        break;
                    }
                }
            }
        }
    }

    // Merge background and foreground
    uint8_t finalPixel = 0;
    uint8_t finalPalette = 0;
    bool isSprite = false;

    if (bgPixel == 0 && fgPixel == 0) {
        finalPixel = 0;
        finalPalette = 0;
    } else if (bgPixel == 0 && fgPixel != 0) {
        finalPixel = fgPixel;
        finalPalette = fgPalette;
        isSprite = true;
    } else if (bgPixel != 0 && fgPixel == 0) {
        finalPixel = bgPixel;
        finalPalette = bgPalette;
    } else {
        if (fgPriority == 0) {
            finalPixel = fgPixel;
            finalPalette = fgPalette;
            isSprite = true;
        } else {
            finalPixel = bgPixel;
            finalPalette = bgPalette;
        }

        // Sprite Zero Hit
        if (fgSpriteZero && (x < 255)) {
            // Both must be enabled
            if ((mask & 0x08) && (mask & 0x10)) {
                // Leftmost 8 pixels mask check
                if (x >= 8 || ((mask & 0x02) && (mask & 0x04))) {
                    status |= 0x40; // Sprite zero hit!
                }
            }
        }
    }

    // Palette lookup
    uint32_t color = 0;
    if (isSprite) {
        color = systemPalette[ppuRead(0x3F10 + (static_cast<uint16_t>(finalPalette) << 2) + finalPixel, false) & 0x3F];
    } else {
        color = systemPalette[ppuRead(0x3F00 + (static_cast<uint16_t>(finalPalette) << 2) + finalPixel, false) & 0x3F];
    }

    frameBuffer[static_cast<size_t>(y) * 256 + static_cast<size_t>(x)] = color;
}

void ppu::incrementScrollY() {
    if (mask & 0x18) { // If rendering is enabled
        if ((vramAddr & 0x7000) != 0x7000) {
            vramAddr += 0x1000;        // Increment fine Y
        } else {
            vramAddr &= ~0x7000;       // Fine Y = 0
            int y = (vramAddr >> 5) & 0x001F;
            if (y == 29) {
                y = 0;
                vramAddr ^= 0x0800;    // Switch vertical nametable
            } else if (y == 31) {
                y = 0;                 // Coarse Y = 0, no nametable switch
            } else {
                y++;                   // Increment coarse Y
            }
            vramAddr = (vramAddr & ~0x03E0) | (y << 5);
        }
    }
}

void ppu::transferAddressX() {
    if (mask & 0x18) {
        vramAddr = (vramAddr & 0xFBE0) | (tramAddr & 0x041F);
    }
}

void ppu::transferAddressY() {
    if (mask & 0x18) {
        vramAddr = (vramAddr & 0x041F) | (tramAddr & 0x7BE0);
    }
}

void ppu::clock(){
    if(scanline >= 0 && scanline < 240){
        if(cycle == 0){
            evaluateSprites();
        }
        if(cycle >= 1 && cycle <= 256){
            renderPixel();
        }
        if(cycle == 256){
            incrementScrollY();
        }
        if(cycle == 257){
            transferAddressX();
        }
    }

    if(scanline == -1){
        if(cycle == 257){
            transferAddressX();
        }
        if(cycle == 304){
            transferAddressY();
        }
    }

    if(scanline == 241 && cycle == 1){
        status |= 0x80; // Enter vblank
        if (control & 0x80) {
            nmi = true;
        }
        frameComplete = true;
    }

    if(scanline == -1 && cycle == 1){
        status &= ~0xE0; // Leave vblank, clear sprite zero hit and sprite overflow
    }

    cycle++;
    if(cycle >= 341){
        cycle = 0;
        scanline++;
        if(scanline >= 261){
            scanline = -1;
        }
    }
}


