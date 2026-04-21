#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <memory>


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

    private: 
        std::string romPath;
};