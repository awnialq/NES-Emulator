#pragma once
#include <cstdint>
#include "cartridge.h"
#include "ppu2C02.h"
#include "Mapper.h"
#include <array>
#include <memory>
#include <vector>

class Bus{
    public:
        Bus(char *);
        ~Bus();

    public:
        // CPU Memory (2KB at 0x0000-0x1FFF)
        std::array<uint8_t, 2048> cpuMem; 
        
        // PPU (Picture Processing Unit)
        ppu2C02 ppu;
        
        // Mapper for cartridge memory routing
        std::unique_ptr<Mapper> mapper;

        // CPU access functions
        void cpuWrite(uint16_t addr, uint8_t data);
        uint8_t cpuRead(uint16_t addr, bool readOnly = false);
        
        // PPU access functions (internal)
        void ppuWrite(uint16_t addr, uint8_t data);
        uint8_t ppuRead(uint16_t addr, bool readOnly = false);
        
        // Render a frame
        void renderFrame();
        
        // Get rendered frame buffer
        uint8_t* getFrameBuffer();
        bool isFrameReady();
        
        // Set vblank callback
        void setVBlankCallback(void (*callback)());

        cartridge *cart = nullptr;
    private:
        // Helper to route through mapper
        uint8_t readFromCartridge(uint16_t addr, bool readOnly);
        void writeToCartridge(uint16_t addr, uint8_t data);
};
