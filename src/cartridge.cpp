#include "cartridge.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include "Mapper000.h"
#include "Mapper003.h"

using crt = cartridge;

crt::cartridge(char *rp){
    romPath = rp;
}

//ret 1 if the cart inits properly
uint8_t crt::initCart(){
    std::ifstream nesStream(romPath,std::ios::binary);
    if(!nesStream.is_open()){
        std::cout << "Error: could not open ROM file " << romPath << std::endl;
        return 0;
    }
    nesStream.read(reinterpret_cast<char *>(header),16);
    if(!(header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1A)){
        std::cout << "Error: file " << romPath << " not in iNES format" << std::endl;
        return 0;
    }
    mapperID = ((header[7] >> 4) << 4) | (header[6] >> 4);
    mirror = (header[6] & 0x01) ? VERTICAL : HORIZONTAL;

    prgMem.resize(header[4] * 16384);

    nesStream.read(reinterpret_cast<char *>(prgMem.data()),(header[4] * 16384));
    
    if(header[5] != 0){
        chrMem.resize(header[5] * 8192);
        nesStream.read(reinterpret_cast<char *>(chrMem.data()), (8192 * header[5]));
    }
    else{
        // CHR RAM for mapper 0 carts without CHR ROM
        chrMem.resize(8192);
    }

    if(mapperID == 0){
        mapper = std::make_shared<Mapper000>(header[4], header[5]);
    }
    else if(mapperID == 3){
        mapper = std::make_shared<Mapper003>(header[4], header[5]);
    }
    else{
        std::cout << "Error: unsupported mapper " << static_cast<int>(mapperID) << std::endl;
        return 0;
    }

    nesStream.close();
    return 1;
}

bool crt::cpuRead(uint16_t addr, uint8_t &data){
    uint32_t mappedAddr = 0;
    if(mapper && mapper->modCpuRead(addr, mappedAddr)){
        data = prgMem[mappedAddr % prgMem.size()];
        return true;
    }
    return false;
}

bool crt::cpuWrite(uint16_t addr, uint8_t data){
    uint32_t mappedAddr = 0;
    if(mapper && mapper->modCpuWrite(addr, data, mappedAddr)){
        if(mappedAddr == UINT32_MAX){
            return true;
        }
        prgMem[mappedAddr % prgMem.size()] = data;
        return true;
    }
    return false;
}

bool crt::ppuRead(uint16_t addr, uint8_t &data){
    uint32_t mappedAddr = 0;
    if(mapper && mapper->modPpuRead(addr, mappedAddr)){
        data = chrMem[mappedAddr % chrMem.size()];
        return true;
    }
    return false;
}

bool crt::ppuWrite(uint16_t addr, uint8_t data){
    uint32_t mappedAddr = 0;
    if(mapper && mapper->modPpuWrite(addr, mappedAddr)){
        chrMem[mappedAddr % chrMem.size()] = data;
        return true;
    }
    return false;
}