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

        void connectCart(const std::shared_ptr<cartridge>& cart);
        void clock();
    private:
        std::shared_ptr<cartridge> game;

        uint8_t nameTables[2][1024];    //two 1kb nametables stored as a 2d array (to mimmic a table)
        uint8_t paletteTable[32];       //32 bytes are allocated to the palette memory for the ppu
};