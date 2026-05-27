#include "Mapper003.h"

Mapper003::Mapper003(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks,chrBanks){}

Mapper003::~Mapper003(){}

bool Mapper003::modCpuRead(uint16_t addr, uint32_t &mappedAddr){
    if(addr >= 0x8000 && addr <= 0xFFFF){
        mappedAddr = addr & (numPrgBanks > 1 ? 0x7FFF : 0x3FFF);
        return true;
    }
    return false;
}

bool Mapper003::modCpuWrite(uint16_t addr, uint8_t data, uint32_t &mappedAddr){
    if(addr >= 0x8000 && addr <= 0xFFFF){
        if(numChrBanks > 0){
            chrBankSelect = data & 0x03;
            chrBankSelect %= numChrBanks;
        }

        // Mapper 003 writes update the CHR bank select register rather than PRG ROM.
        // Signal to the cartridge that no PRG memory write should occur.
        mappedAddr = UINT32_MAX;
        return true;
    }
    return false;
}

bool Mapper003::modPpuRead(uint16_t addr, uint32_t &mappedAddr){
    if(addr >= 0x0000 && addr <= 0x1FFF){
        mappedAddr = (chrBankSelect * 0x2000) + addr;
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