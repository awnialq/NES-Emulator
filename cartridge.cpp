//Note to self: use xxd on the nestest rom to check byte structure with WSL.

#include "cartridge.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <assert.h>
#include <string.h>

using crt = cartridge;

cartridge::cartridge(){
}

cartridge::~cartridge(){
}


//ret 1 if the cart inits properly
uint8_t crt::initCart(){
    std::ifstream nesStream("/testRoms/nestest.nes",std::ios::binary);
    assert(nesStream.is_open());
    nesStream.read(reinterpret_cast<char *>(header),16);
    if(!(header[0] == 'N' && header[1] == 'E'&& header[2] == 'S'&& header[3] == 0x1A)){
        printf("Error: Not in ines format\n");
        return 0;
    }
    nesStream.read(reinterpret_cast<char *>(prgMem.data()),(header[4] * 16384));
    
    if(header[5] != 0){
        nesStream.read(reinterpret_cast<char *>(chrMem.data()), (8192 * header[5]));
    }

    nesStream.close();
    return 1;
}