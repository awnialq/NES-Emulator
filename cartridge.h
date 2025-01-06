#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <fstream>

struct cartHeader{
    char id[4];
    uint8_t prgSize;
    uint8_t chrSize;
};

class cartridge{
    public: 
        cartridge(std::string &fileName);
        ~cartridge();
    private: 
        struct cartHeader *header;
        uint32_t prgDataSize;
        uint32_t chrDataSize;
        std::ifstream fileStream;
        std::string &fileName;
    public:
        void loadRom();
    private:
        void defHeader();
        void confirmRom();
};