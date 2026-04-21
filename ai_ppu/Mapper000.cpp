#include "Mapper000.h"

Mapper000::Mapper000(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks){}

Mapper000::~Mapper000(){}

bool Mapper000::modCpuRead(uint16_t addr, uint32_t &mappedAddr){
    if(addr >= 0x8000 && addr <= 0xFFFF){
        // Mirror PRG ROM
        if(numPrgBanks > 1){
            mappedAddr = addr & 0x7FFF;
        } else {
            mappedAddr = addr & 0x3FFF;
        }
        return true;
    }
    return false;
}

bool Mapper000::modCpuWrite(uint16_t addr, uint32_t &mappedAddr){
    if(addr >= 0x8000 && addr <= 0xFFFF){
        if(numPrgBanks > 1){
            mappedAddr = addr & 0x7FFF;
        } else {
            mappedAddr = addr & 0x3FFF;
        }
        return true;
    }
    return false;
}

bool Mapper000::modPpuRead(uint16_t addr, uint32_t &mappedAddr){
    // Fixed bug: was missing "&& addr <= 0x1FFF"
    if(addr >= 0x0000 && addr <= 0x1FFF){
        mappedAddr = addr;
        return true;
    }
    return false;
}

bool Mapper000::modPpuWrite(uint16_t addr, uint32_t &mappedAddr){
    if((addr >= 0x0000 && addr <= 0x1FFF) && numChrBanks == 0){
        mappedAddr = addr;
        return true;
    }
    return false;
}
