#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <memory>

struct cartHeader{
    char id[4];
    uint8_t prgSize;    //program size
    uint8_t chrSize;    //character memory size
};

class cartridge{
    public: 
        cartridge();
        cartridge(std::string &fileName);
        ~cartridge();
    private: 
        std::vector<uint8_t> prgMem;    //Program memory
        std::vector<uint8_t> chrMem;    //Character memory

        uint8_t mapperID = 0;
        struct cartHeader header;
};