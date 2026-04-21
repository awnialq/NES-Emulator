#include "ppu2C02.h"
#include <cstring>
#include <algorithm>
#include <cstdio>

using ppu = ppu2C02;

ppu::ppu2C02() {
    // Initialize frame buffers
    frameBuffer1.resize(256 * 240);
    frameBuffer2.resize(256 * 240);
    currentBuffer = frameBuffer1.data();
    
    // Clear all memory
    regPPUCTRL = 0;
    regPPUMASK = 0;
    regPPUSTATUS = 0;
    regOAMADDR = 0;
    regOAMDATA = 0;
    regPPUSCROLL = 0;
    regPPUADDR = 0;
    regPPUDATA = 0;
    
    std::memset(nameTables.data(), 0, 4096);
    std::memset(paletteRAM.data(), 0, 32);
    std::memset(oam.data(), 0, 256);
    
    // Reset state
    scanline = 0;
    clockCycle = 0;
    fineXScroll = 0;
    fineYScroll = 0;
    coarseXScroll = 0;
    coarseYScroll = 0;
}

ppu::~ppu2C02() {
    // Cleanup
}

void ppu::init(uint8_t *chrMem, uint32_t chrMemSize) {
    chrMemory = chrMem;
    this->chrMemSize = chrMemSize;
}

// ============================================================================
// CPU ACCESS TO PPU REGISTERS (0x2000-0x2007)
// ============================================================================

uint8_t ppu::cpuRead(uint16_t addr, bool readOnly) {
    (void)readOnly;
    addr &= 0x0007;  // Mirror 8 bytes
    
    switch(addr) {
        case 0x00: // PPUCTRL - Read returns PPUCTRL (but usually not read by CPU)
            return regPPUCTRL;
            
        case 0x01: // PPUMASK - Read returns PPUMASK
            return regPPUMASK;
            
        case 0x02: // PPUSTATUS - Read clears bits 7 and 6
        {
            uint8_t status = regPPUSTATUS;
            regPPUSTATUS &= ~0xC0;  // Clear sprite zero hit and vblank flags
            return status;
        }
            
        case 0x03: // OAMADDR - Read returns OAM address (usually not read)
            return regOAMADDR;
            
        case 0x04: // OAMDATA - Read returns OAM data at current address
            return oam[regOAMADDR];
            
        case 0x05: // PPUSCROLL - Read returns scroll data (usually not read)
            return regPPUSCROLL;
            
        case 0x06: // PPUADDR - Read returns PPU address (usually not read)
            return regPPUADDR;
            
        case 0x07: // PPUDATA - Read returns PPU data
            return regPPUDATA;
            
        default:
            return 0;
    }
}

void ppu::cpuWrite(uint16_t addr, uint8_t data) {
    addr &= 0x0007;  // Mirror 8 bytes
    
    switch(addr) {
        case 0x00: // PPUCTRL
            regPPUCTRL = data;
            // Bits 0-1: Name Table select (not implemented in basic version)
            // Bits 2-3: 9-bit scroll (fine X)
            fineXScroll = (data >> 2) & 0x07;
            // Bits 4-5: Background pattern table address (0x0000 or 0x1000)
            patternTableAddr = ((data >> 4) & 0x01) ? 0x1000 : 0x0000;
            // Bit 6: Sprite pattern table address (0x0000 or 0x1000)
            // Bit 7: Sprite size (8x8 or 8x16)
            break;
            
        case 0x01: // PPUMASK
            regPPUMASK = data;
            // Bit 0: GREYSCALE (not implemented)
            // Bit 1: BG leftmost 8 columns
            // Bit 2: Sprite leftmost 8 columns
            // Bit 3: Render background
            backgroundRenderEnabled = (data >> 3) & 0x01;
            // Bit 4: Render sprites
            spriteRenderEnabled = (data >> 4) & 0x01;
            // Bit 5: Priority for sprites over background (not implemented)
            // Bit 6: Render red in master color
            // Bit 7: Render green in master color
            break;
            
        case 0x02: // PPUSTATUS - Write does nothing (read-only)
            break;
            
        case 0x03: // OAMADDR
            regOAMADDR = data;
            oamWriteMode = false;  // Reset auto-increment
            break;
            
        case 0x04: // OAMDATA
            if (oamWriteMode) {
                regOAMADDR = (regOAMADDR + 1) & 0xFF;
            }
            oam[regOAMADDR] = data;
            break;
            
        case 0x05: // PPUSCROLL
            if (regPPUSCROLL & 0x80) {  // Second byte (low 8 bits of scroll)
                latchPPUSCROLL_LO = data;
                // Combine scroll values
                coarseYScroll = ((latchPPUSCROLL_HI & 0x1F) << 3) | (fineYScroll & 0x1F);
                fineYScroll = (latchPPUSCROLL_HI >> 5) & 0x07;
                coarseXScroll = ((latchPPUSCROLL_LO & 0x1F) << 3) | (fineXScroll & 0x1F);
                fineXScroll = (latchPPUSCROLL_HI >> 5) & 0x07;
                regPPUSCROLL = 0;
            } else {  // First byte (high 8 bits of scroll)
                latchPPUSCROLL_HI = data;
                regPPUSCROLL = 0x80;
            }
            break;
            
        case 0x06: // PPUADDR
            if (regPPUADDR & 0x80) {  // Second byte (low 8 bits of address)
                latchPPUADDR_LO = data;
                regPPUADDR = 0;
            } else {  // First byte (high 8 bits of address)
                latchPPUADDR_HI = data;
                regPPUADDR = 0x80;
            }
            break;
            
        case 0x07: // PPUDATA
            // Data is written to the address specified by PPUADDR
            // This is handled by ppuWrite
            break;
            
        default:
            break;
    }
}

// ============================================================================
// INTERNAL PPU MEMORY ACCESS (0x0000-0x3FFF)
// ============================================================================

uint8_t ppu::ppuRead(uint16_t addr, bool readOnly) {
    (void)readOnly;
    // CHR ROM/RAM: 0x0000-0x1FFF
    if (addr >= 0x0000 && addr <= 0x1FFF) {
        if (chrMemory && chrMemSize > 0) {
            return chrMemory[addr % chrMemSize];
        }
        return 0;
    }
    
    // Name Tables: 0x2000-0x3EFF (4 KB)
    if (addr >= 0x2000 && addr <= 0x3EFF) {
        return nameTables[addr - 0x2000];
    }
    
    // Palette RAM: 0x3F00-0x3FFF (32 bytes)
    if (addr >= 0x3F00 && addr <= 0x3FFF) {
        uint8_t paletteAddr = addr & 0x1F;
        return paletteRAM[paletteAddr];
    }
    
    return 0;
}

void ppu::ppuWrite(uint16_t addr, uint8_t data) {
    // CHR RAM: 0x0000-0x1FFF (only if CHR RAM exists)
    if (addr >= 0x0000 && addr <= 0x1FFF) {
        if (chrMemory && chrMemSize > 0) {
            chrMemory[addr % chrMemSize] = data;
        }
        return;
    }
    
    // Name Tables: 0x2000-0x3EFF
    if (addr >= 0x2000 && addr <= 0x3EFF) {
        nameTables[addr - 0x2000] = data;
        return;
    }
    
    // Palette RAM: 0x3F00-0x3FFF
    if (addr >= 0x3F00 && addr <= 0x3FFF) {
        uint8_t paletteAddr = addr & 0x1F;
        paletteRAM[paletteAddr] = data;
        return;
    }
}

// ============================================================================
// RENDERING PIPELINE
// ============================================================================

void ppu::renderFrame() {
    // Clear frame buffer
    std::fill(currentBuffer, currentBuffer + 256 * 240, 0);
    
    // Render each scanline
    for (scanline = 0; scanline < 240; scanline++) {
        renderBackground();
        renderSprites();
    }
    
    // VBlank
    inVBlank = true;
    if (vBlankCallback) {
        vBlankCallback();
    }
    
    // Mark frame as ready
    frameReady = true;
}

void ppu::renderBackground() {
    // For each pixel in the scanline
    for (int x = 0; x < 256; x++) {
        // Calculate which tile we're rendering
        uint16_t tileX = (coarseXScroll + x) % 256;  // Wrap around
        uint16_t tileY = (coarseYScroll + scanline) % 256;
        
        // Get name table address
        uint16_t nameTableAddr = 0x2000 + (tileY / 8) * 256 + (tileX / 8);
        if (nameTableAddr >= 0x2400) nameTableAddr = 0x2000;  // Mirror
        
        // Get tile number from name table
        uint8_t tileNumber = nameTables[nameTableAddr - 0x2000];
        
        // Calculate pattern table address
        uint16_t patternAddr = patternTableAddr + tileNumber * 16 + (scanline % 8);
        if (patternAddr >= 0x1000) patternAddr -= 0x1000;  // Wrap
        
        // Get pixel data (2 bytes for 2 bitplanes)
        uint8_t loByte = ppuRead(patternAddr, false);
        uint8_t hiByte = ppuRead(patternAddr + 8, false);
        
        // Extract pixel at x position (within tile)
        uint8_t bitX = 7 - (x % 8);
        uint8_t bit0 = (loByte >> bitX) & 1;
        uint8_t bit1 = (hiByte >> bitX) & 1;
        uint8_t colorIndex = (bit1 << 1) | bit0;
        
        // Get palette color
        uint8_t paletteAddr = 0x3F00 + 4 * ((tileNumber >> 4) & 0x0F) + colorIndex;
        uint8_t color = paletteRAM[paletteAddr & 0x1F];
        
        // Store in frame buffer
        currentBuffer[scanline * 256 + x] = color;
    }
}

void ppu::renderSprites() {
    // Simple sprite rendering (8x8 sprites only)
    // Process each sprite in OAM
    for (int i = 0; i < 64; i++) {  // Max 64 sprites
        uint8_t spriteY = oam[i * 4 + 0];
        uint8_t spriteX = oam[i * 4 + 1];
        uint8_t tileNumber = oam[i * 4 + 2];
        uint8_t attributes = oam[i * 4 + 3];
        
        // Check if sprite is on current scanline
        if (spriteY > scanline && spriteY + 7 >= scanline) {
            // Calculate pixel position within sprite
            uint8_t pixelY = scanline - spriteY;
            uint8_t pixelX = 7 - (spriteX % 8);
            
            // Get pattern table address
            uint16_t patternAddr = (attributes & 0x01) ? 0x1000 : 0x0000;
            patternAddr += tileNumber * 16 + pixelY;
            if (patternAddr >= 0x1000) patternAddr -= 0x1000;
            
            // Get pixel data
            uint8_t loByte = ppuRead(patternAddr, false);
            uint8_t hiByte = ppuRead(patternAddr + 8, false);
            
            // Extract pixel
            uint8_t bit0 = (loByte >> pixelX) & 1;
            uint8_t bit1 = (hiByte >> pixelX) & 1;
            uint8_t colorIndex = (bit1 << 1) | bit0;
            
            // Skip transparent pixel
            if (colorIndex == 0) continue;
            
            // Get palette color
            uint8_t paletteAddr = 0x3F10 + 4 * ((tileNumber >> 4) & 0x0F) + colorIndex;
            uint8_t color = paletteRAM[paletteAddr & 0x1F];
            
            // Render sprite pixel (with priority - sprites on top of background)
            int bufX = spriteX;
            if (bufX >= 0 && bufX < 256) {
                currentBuffer[scanline * 256 + bufX] = color;
            }
        }
    }
}

uint8_t ppu::getPixelColor(uint8_t paletteIndex, uint8_t colorIndex) {
    // Background palette: 0x3F00-0x3F1F
    // Sprite palette: 0x3F10-0x3F1F
    uint8_t paletteAddr = 0x3F00 + 4 * paletteIndex + colorIndex;
    return paletteRAM[paletteAddr & 0x1F];
}

// ============================================================================
// BUFFER MANAGEMENT
// ============================================================================

uint8_t* ppu::getFrameBuffer() {
    return frameBuffer1.data();
}

uint8_t* ppu::getSecondFrameBuffer() {
    return frameBuffer2.data();
}

bool ppu::isFrameReady() {
    return frameReady;
}

void ppu::swapBuffers() {
    std::swap(frameBuffer1, frameBuffer2);
    currentBuffer = frameBuffer2.data();
    frameReady = false;
}

// ============================================================================
// PPU CLOCK
// ============================================================================

void ppu::clock() {
    // PPU runs at 3x CPU clock (2128546.5 Hz / 1789772.5 Hz ≈ 1.19)
    // But we'll simplify: render one frame per CPU frame (approximate)
    
    // This is a simplified version - real PPU has precise timing
    // For proper emulation, you need to track PPU cycles precisely
    
    // For now, just trigger render on demand
}

// ============================================================================
// VBLANK CALLBACK
// ============================================================================

void ppu::setVBlankCallback(void (*callback)()) {
    vBlankCallback = callback;
}

// ============================================================================
// DEBUG/STATUS
// ============================================================================

std::string ppu::getStatusString() {
    return "PPU Status: Scanline=" + std::to_string(scanline) + 
           " | VBlank=" + std::to_string(inVBlank) +
           " | BG Enable=" + std::to_string(backgroundRenderEnabled) +
           " | Sprite Enable=" + std::to_string(spriteRenderEnabled);
}
