#pragma once
#include <cstdint>
#include <iostream>
#include <vector>
#include <array>
#include "cartridge.h"

class ppu2C02{
    public:
        ppu2C02();
        ~ppu2C02();
        
        // Initialize with cartridge CHR memory
        void init(uint8_t *chrMem, uint32_t chrMemSize);

        // Connection to main bus (CPU access to PPU registers)
        uint8_t cpuRead(uint16_t addr, bool readOnly);
        void cpuWrite(uint16_t addr, uint8_t data);

        // Connection to PPU bus (internal PPU memory access)
        uint8_t ppuRead(uint16_t addr, bool readOnly);
        void ppuWrite(uint16_t addr, uint8_t data);

        // Frame rendering
        void renderFrame();
        
        // Get rendered frame data
        uint8_t* getFrameBuffer();
        uint8_t* getSecondFrameBuffer();
        bool isFrameReady();
        
        // Double buffering
        void swapBuffers();

        // PPU Clock (should run at 3x CPU speed)
        void clock();

        // Debug/Status
        std::string getStatusString();
        
        // Set callback for vblank interrupt
        void setVBlankCallback(void (*callback)());
        
    private:
        // PPU Registers (CPU-accessible, mapped to 0x2000-0x2007)
        uint8_t regPPUCTRL;   // 0x2000 - PPU Control
        uint8_t regPPUMASK;   // 0x2001 - PPU Mask
        uint8_t regPPUSTATUS; // 0x2002 - PPU Status
        uint8_t regOAMADDR;   // 0x2003 - OAM Address (write only)
        uint8_t regOAMDATA;   // 0x2004 - OAM Data
        uint8_t regPPUSCROLL; // 0x2005 - PPU Scroll
        uint8_t regPPUADDR;   // 0x2006 - PPU Address (write only)
        uint8_t regPPUDATA;   // 0x2007 - PPU Data

        // Internal PPU State
        uint8_t latchPPUADDR_HI;
        uint8_t latchPPUADDR_LO;
        uint8_t latchPPUSCROLL_HI;
        uint8_t latchPPUSCROLL_LO;
        
        // Frame buffer (256x240 pixels, indexed color)
        std::vector<uint8_t> frameBuffer1;
        std::vector<uint8_t> frameBuffer2;
        uint8_t* currentBuffer = nullptr;
        bool frameReady = false;

        // PPU memory regions
        uint8_t* chrMemory = nullptr;
        uint32_t chrMemSize = 0;
        
        // Name tables (4KB total, but we use 1KB per table)
        std::array<uint8_t, 4096> nameTables;  // 4 name tables
        
        // Palette memory (32 bytes)
        std::array<uint8_t, 32> paletteRAM;

        // OAM (Sprite Memory) - 256 bytes
        std::array<uint8_t, 256> oam;
        uint8_t oamAddress = 0;
        bool oamWriteMode = false;  // Auto-increment mode

        // Rendering state
        uint16_t scanline = 0;
        uint16_t clockCycle = 0;
        uint8_t fineXScroll = 0;
        uint8_t fineYScroll = 0;
        uint16_t coarseXScroll = 0;  // 0-29 (32 tiles wide)
        uint16_t coarseYScroll = 0;  // 0-29 (32 tiles tall)
        uint16_t patternTableAddr = 0x0000;  // 0x0000 or 0x1000
        
        // VBlank callback
        void (*vBlankCallback)() = nullptr;
        
        // Sprite overflow flag
        bool spriteOverflow = false;
        
        // Background and sprite rendering
        bool backgroundRenderEnabled = false;
        bool spriteRenderEnabled = false;

        // Render helpers
        void renderBackground();
        void renderSprites();
        uint8_t getPixelColor(uint8_t paletteIndex, uint8_t colorIndex);
        void loadSpriteLine(uint8_t spriteY);
        
        // Frame state
        bool inVBlank = false;
        bool inRendering = false;
};
