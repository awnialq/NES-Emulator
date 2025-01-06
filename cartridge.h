#pragma once
#include <cstdint>
#include <vector>
#include <string>

struct cartHeader{
    char id[4];
    uint8_t prgSize;
    uint8_t chrSize;
};

class cartridge{
    public: 
        cartridge(std::string fileName);
        ~cartridge();
    private: 
        struct cartHeader *header;
        uint32_t prgDataSize;
        uint32_t chrDataSize;
        std::string romFileName;

    public:
        void loadRom(uint8_t *rom);
    private:
        void defHeader(uint8_t *rom);
        void confirmRom(uint8_t *rom);
};