#pragma once
#include "Mapper.h"

class Mapper000 : public Mapper{    //Inherit the base mapper class
    public:
        Mapper000(uint8_t prgBanks, uint8_t chrBanks);
        ~Mapper000();

    public:
        bool modCpuRead(uint16_t addr, uint32_t &mappedAddr);
        bool modCpuWrite(uint16_t addr, uint32_t &mappedAddr);
        bool modPpuRead(uint16_t addr, uint32_t &mappedAddr);
        bool modPpuWrite(uint16_t addr, uint32_t &mappedAddr);
};