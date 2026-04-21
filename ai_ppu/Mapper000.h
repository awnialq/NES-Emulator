#pragma once
#include "Mapper.h"

class Mapper000 : public Mapper{    //Inherit the base mapper class
    public:
        Mapper000(uint8_t prgBanks, uint8_t chrBanks);
        ~Mapper000() override;

    public:
        bool modCpuRead(uint16_t addr, uint32_t &mappedAddr) override;
        bool modCpuWrite(uint16_t addr, uint32_t &mappedAddr) override;
        bool modPpuRead(uint16_t addr, uint32_t &mappedAddr) override;
        bool modPpuWrite(uint16_t addr, uint32_t &mappedAddr) override;
};