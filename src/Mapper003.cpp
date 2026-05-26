#include "Mapper003.h"

Mapper003::Mapper003(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks,chrBanks){}

Mapper003::~Mapper003(){}

bool Mapper003::modCpuRead(uint16_t addr, uint32_t &mappedAddr){
    if(addr >= 0x8000 && addr <= 0xFFFF){
        mappedAddr = addr & 0x7FFF; //mirror the first 32KB of the cartridge ROM
        return true;
    }
    return false;
}

bool Mapper003::modCpuWrite(uint16_t addr, uint32_t &mappedAddr){
    if(addr >= 0x8000 && addr <= 0xFFFF){
        mappedAddr = addr & 0x7FFF; //mirror the first 32KB of the cartridge ROM
        return true;
    }
    return false;
}

bool Mapper003::modPpuRead(uint16_t addr, uint32_t &mappedAddr){
    //ppu has a very limited range so no extra mapping is needed for its functionality (just have to check if the addr is in range)
    if(addr >= 0x0000 && addr <= 0x1FFF){
        mappedAddr = addr;
        return true;
    }
    return false;
}

bool Mapper003::modPpuWrite(uint16_t addr, uint32_t &mappedAddr){
    if((addr >= 0x0000 && addr <= 0x1FFF) && numChrBanks == 0){
        mappedAddr = addr;
        return true;
    }
    return false;
}