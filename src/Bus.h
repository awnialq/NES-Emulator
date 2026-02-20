#pragma once
#include <cstdint>
#include "cpu6502.h"
#include "cartridge.h"
#include "ppu2C02.h"
#include <array>
#include <memory>
#include <vector>

class Bus{
    public:
        Bus();
        ~Bus();

    public:
        std::array<uint8_t, 2048> cpuMem; 
       // ppu2C02 ppu;
        void cpuWrite(uint16_t addr, uint8_t data);
        uint8_t cpuRead(uint16_t addr, bool readOnly = false);
        //void reset();
        //void clock();
        cartridge *cart = nullptr;
    private:
        //uint32_t clockCntr = 0;
};