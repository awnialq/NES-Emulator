#pragma once
#include <cstdint>
#include "cpu6502.h"
#include "cartridge.h"
#include <array>

class Bus{
    public:
        Bus();
        ~Bus();

    public:
        cpu6502 cpu;
        std::array<uint8_t, 65536> wires;
        cartridge game;

    public:
        void write(uint16_t addr, uint8_t data);
        uint8_t read(uint16_t addr, bool readOnly = false);
};