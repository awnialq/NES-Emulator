//Note to self: use xxd on the nestest rom to check byte structure with WSL.

#include "cartridge.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <assert.h>
#include <string>

using crt = cartridge;

crt::cartridge(char *rp){
    romPath = rp;
}

//ret 1 if the cart inits properly
uint8_t crt::initCart(){
    std::ifstream nesStream(romPath,std::ios::binary);
    assert(nesStream.is_open());
    nesStream.read(reinterpret_cast<char *>(header),16);
    if(!(header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1A)){
        std::cout << "Error: file not in iNES format" << std::endl;
        return 0;
    }
    prgMem.resize(header[4] * 16384);

    nesStream.read(reinterpret_cast<char *>(prgMem.data()),(header[4] * 16384));
    
    if(header[5] != 0){
        chrMem.resize(header[5] * 8192);
        nesStream.read(reinterpret_cast<char *>(chrMem.data()), (8192 * header[5]));
    }
    nesStream.close();
    return 1;
}