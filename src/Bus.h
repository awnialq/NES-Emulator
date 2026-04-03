#pragma once
#include <cstdint>
#include <array>
#include <memory>
#include "cpu6502.h"
#include "ppu2C02.h"
#include "cartridge.h"

class Bus {
public:
    Bus(const char *romPath);
    ~Bus();

public:
    void cpuWrite(uint16_t addr, uint8_t data);
    uint8_t cpuRead(uint16_t addr, bool readOnly = false);
    void reset();
    void clock();

    void connectCpu(cpu6502 *cpu) { m_cpu = cpu; }

private:
    uint8_t internalRam[2048];
    uint8_t zeroPage[256];
    uint8_t stack[128];

    ppu2C02 ppu;
    cartridge *cart;
    cpu6502 *m_cpu = nullptr;

    uint32_t nmiState = 0;
    uint32_t irqState = 0;
    uint32_t scanline = 0;
    uint32_t cycleCounter = 0;
};