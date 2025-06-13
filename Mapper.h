#pragma once
#include <cstdint>

/*
    A baseline mapper class for each seperate mapper implementation to use.
*/

class Mapper{
    public:
        Mapper(uint8_t prgBanks, uint8_t chrBanks);
        ~Mapper();
    private:
        virtual bool modCpuRead(uint16_t addr, uint16_t &mappedAddr);
        virtual bool modCpuWrite(uint16_t addr, uint16_t &mappedAddr);
        virtual bool modPpuRead(uint16_t addr, uint16_t &mappedAddr);
        virtual bool modPpuWrite(uint16_t addr, uint16_t &mappedAddr);
    protected:
        uint8_t numPrgBanks;
        uint8_t numChrBanks;
};