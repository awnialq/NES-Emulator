#include "Mapper000.h"

Mapper000::Mapper000(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks,chrBanks){}

Mapper000::~Mapper000(){}

bool Mapper000::modCpuRead(uint16_t addr, uint32_t &mappedAddr){
    if(addr >= 0x8000 && addr <= 0xFFFF){
        mappedAddr = addr & (numPrgBanks > 1 ? 0x7FFF : 0x3FFF); //if there is more than one bank then mirror a larger range of addresses.
        return true;
    }
    return false;
}

bool Mapper000::modCpuWrite(uint16_t addr, uint32_t &mappedAddr){
    if(addr >= 0x8000 && addr <= 0xFFFF){
        mappedAddr = addr & (numPrgBanks > 1 ? 0x7FFF : 0x3FFF); //if there is more than one bank then mirror a larger range of addresses.
        return true;
    }
    return false;
}

bool Mapper000::modPpuRead(uint16_t addr, uint32_t &mappedAddr){
    //ppu has a very limited range so no extra mapping is needed for its functionality (just have to check if the addr is in range)
    if(addr >= 0x0000 && 0x1FFF){
        mappedAddr = addr;
        return true;
    }
    return false;
}

bool Mapper000::modPpuWrite(uint16_t addr, uint32_t &mappedAddr){
    if(addr >= 0x0000 && 0x1FFF){
        mappedAddr = addr;
        return true;
    }
    return false;
}