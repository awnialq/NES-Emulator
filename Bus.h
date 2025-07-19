#pragma once
#include <cstdint>
#include "cpu6502.h"
#include "cartridge.h"
#include "ppu2C02.h"
#include <array>
#include <memory>

class Bus{
    public:
        Bus();
        ~Bus();

    public:
        cpu6502 cpu;
        std::array<uint8_t, 2048> cpuMem;
        std::shared_ptr<cartridge> game;
       // ppu2C02 ppu;
    public:
        void cpuWrite(uint16_t addr, uint8_t data);
        uint8_t cpuRead(uint16_t addr, bool readOnly = false);

        void insertCart(const std::shared_ptr<cartridge>& cartridge); 
        void reset();
        void clock();
    private:
        uint32_t clockCntr = 0;
};