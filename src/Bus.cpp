#include "Bus.h"
#include "cartridge.h"
#include "ppu2C02.h"
#include <cstring>
#include <iostream>
#include <cstdio>

Bus::Bus(const char *romPath) {
    std::memset(internalRam, 0, sizeof(internalRam));
    std::memset(zeroPage, 0, sizeof(zeroPage));
    std::memset(stack, 0, sizeof(stack));

    try {
        cart = new cartridge(const_cast<char*>(romPath));
        if (cart->initCart() != 1) {
            throw std::runtime_error("Cartridge could not be loaded");
        }

        m_cpu->cpu6502::progc = ((cart->header[0xFFFC] & 0xFF) << 8) + (cart->header[0xFFFD] & 0xFF);
        m_cpu->cpu6502::status = 0x24;

        printf("Bus initialized with cartridge successfully loaded\n");
    } catch (const std::exception &e) {
        printf("Error initializing Bus: %s\n", e.what());
    }
}

Bus::~Bus() {
    delete cart;
}

void Bus::reset() {
    if (m_cpu) {
        m_cpu->cpu6502::progc = ((cart->header[0xFFFC] & 0xFF) << 8) + (cart->header[0xFFFD] & 0xFF);
        m_cpu->cpu6502::status = 0x24;
    }
}

void Bus::clock() {
    ppu.clock();
}

void Bus::cpuWrite(uint16_t addr, uint8_t data) {
    if (addr < 0x2000) {
        internalRam[addr] = data;

    } else if (addr < 0x4000) {
        ppu.cpuWrite(addr & 0x2007, data);
        if (addr & 0x2000) {
            ppu.nmiTriggered = true;
        }

    } else if (addr == 0x4014) {
        ppu.oam.dmaWrite(data);

    } else if (addr < 0x4018) {
        cpu->writeRegister(addr, data);

    } else {
        cpu->writeRegister(addr, data);
    }
}

uint8_t Bus::cpuRead(uint16_t addr, bool readOnly) {
    uint8_t data = 0;

    if (addr < 0x2000) {
        data = internalRam[addr];
    } else if (addr < 0x4014) {
        data = cpu->readRegister(addr);
    } else if (addr >= 0x4018) {
        return ppu.apuRegisterRead(addr & 0x001F, readOnly);
    } else if (addr == 0x4016) {
    } else {
        data = cpu->readRegister(addr);
    }

    return data;
}