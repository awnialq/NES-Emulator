#include "Mapper000.h"

Mapper000::Mapper000(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks,chrBanks){}

Mapper000::~Mapper000(){}

bool Mapper000::modCpuRead(uint16_t addr, uint32_t &mappedAddr){
    if(addr >= 0x8000 && addr <= 0xFFFF){
        
    }
}

bool Mapper000::modCpuWrite(uint16_t addr, uint32_t &mappedAddr){
    
}

bool Mapper000::modPpuRead(uint16_t addr, uint32_t &mappedAddr){
    
}

bool Mapper000::modPpuWrite(uint16_t addr, uint32_t &mappedAddr){
    
}