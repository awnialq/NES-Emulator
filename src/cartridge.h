#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include "Mapper.h"


class cartridge{
    public: 
        cartridge(char *);
        ~cartridge() = default;
        uint8_t initCart();
    public: 
        uint8_t header[16];
        std::vector<uint8_t> prgMem;    //Program memory
        std::vector<uint8_t> chrMem;    //Character memory

        uint8_t mapperID = 0;
        enum MIRROR{
            HORIZONTAL,
            VERTICAL
        };
        MIRROR mirror = HORIZONTAL;

        bool cpuRead(uint16_t addr, uint8_t &data);
        bool cpuWrite(uint16_t addr, uint8_t data);
        bool ppuRead(uint16_t addr, uint8_t &data);
        bool ppuWrite(uint16_t addr, uint8_t data);

    private: 
        std::string romPath;
        std::shared_ptr<Mapper> mapper;
};