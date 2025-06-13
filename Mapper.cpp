#include "Mapper.h"

Mapper::Mapper(uint8_t prgBanks, uint8_t chrBanks){
    numPrgBanks = prgBanks;
    numChrBanks = chrBanks;
}

Mapper::~Mapper(){}