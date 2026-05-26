#pragma once
#include "Mapper.h"

class Mapper003 : public Mapper{    //Inherit the base mapper class
    public:
        Mapper003(uint8_t prgBanks, uint8_t chrBanks);
        ~Mapper003();

    public:
        bool modCpuRead(uint16_t addr, uint32_t &mappedAddr);
        bool modCpuWrite(uint16_t addr, uint32_t &mappedAddr);
        bool modPpuRead(uint16_t addr, uint32_t &mappedAddr);
        bool modPpuWrite(uint16_t addr, uint32_t &mappedAddr);
};