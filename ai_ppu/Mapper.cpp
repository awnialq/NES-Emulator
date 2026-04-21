#include "Mapper.h"

Mapper::Mapper(uint8_t prgBanks, uint8_t chrBanks){
    numPrgBanks = prgBanks;
    numChrBanks = chrBanks;
}

Mapper::~Mapper(){}

// Default implementations keep the base class complete so the vtable is emitted.
// Derived mappers override these to provide actual address translation.
bool Mapper::modCpuRead(uint16_t, uint32_t &){
    return false;
}

bool Mapper::modCpuWrite(uint16_t, uint32_t &){
    return false;
}

bool Mapper::modPpuRead(uint16_t, uint32_t &){
    return false;
}

bool Mapper::modPpuWrite(uint16_t, uint32_t &){
    return false;
}