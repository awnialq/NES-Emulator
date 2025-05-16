#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <fstream>

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