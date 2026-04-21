#pragma once
#include <cstdint>
#include <iostream>
#include "cartridge.h"
#include <array>

class ppu2C02{
    public:
        ppu2C02();
        ~ppu2C02();

        //Connection to main bus
        uint8_t cpuRead(uint16_t addr, bool readOnly);
        void cpuWrite(uint16_t addr, uint8_t data);

        //Connection to PPU bus
        uint8_t ppuRead(uint16_t addr, bool readOnly);
        void ppuWrite(uint16_t addr, uint8_t data);

        void clock();
        bool frameComplete = false;
        void connectCart(cartridge *c);
        const std::array<uint32_t, 256 * 240> &getFrameBuffer() const;
    private:
        cartridge *cart = nullptr;
        uint8_t status = 0x00;
        uint8_t mask = 0x00;
        uint8_t control = 0x00;
        uint8_t addressLatch = 0x00;
        uint8_t ppuDataBuffer = 0x00;
        uint8_t fineX = 0x00;
        uint8_t oamAddr = 0x00;
        uint16_t vramAddr = 0x0000;
        uint16_t tramAddr = 0x0000;
        int16_t scanline = 0;
        int16_t cycle = 0;

        std::array<uint8_t, 256> oam{};
        std::array<uint32_t, 256 * 240> frameBuffer{};
        std::array<uint32_t, 64> systemPalette{};

        uint8_t nameTables[2][1024];    //two 1kb nametables stored as a 2d array (to mimmic a table)
        uint8_t paletteTable[32];       //32 bytes are allocated to the palette memory for the ppu
        uint8_t patternMemory[8192];    //where sprites are stored - graphical information that all part of the ppu require to draw things on screen

        uint8_t readPalette(uint8_t palette, uint8_t pixel);
        uint32_t getColorFromPaletteRam(uint8_t palette, uint8_t pixel);
        void renderBackgroundPixel();
};

/*

- A tile is an 8x8 bitmap
    - We only have 4 color options
    - They are stored in two bit planes - the msb and lsb plane
        - The value of a pixel is the sum of both bit plane values (e.g. 0 + 1 = 01 | 1+1 = 11)
    - Once we have the "color", we combine it with a palette to actuall get what we want to output to the pixel
        - We look at the pallette table to get the proper color we want to output
            - Each palette id has 3 possible colors
                - We multiply the bitplane addition with the pallete id to get the final resulting color
            

*/
