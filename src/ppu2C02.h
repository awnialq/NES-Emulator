#pragma once
#include <cstdint>
#include <iostream>
#include "cartridge.h"

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
    private:

        uint8_t nameTables[2][1024];    //two 1kb nametables stored as a 2d array (to mimmic a table)
        uint8_t paletteTable[32];       //32 bytes are allocated to the palette memory for the ppu
        uint8_t patternMemory[8191];    //where sprites are stored - graphical information that all part of the ppu require to draw things on screen
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
