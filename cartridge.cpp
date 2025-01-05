#include "cartridge.h"
#include <iostream>
#include <assert.h>

cartridge::cartridge(){
    this -> prgDataSize = 0;
    this -> chrDataSize = 0;
}

cartridge::~cartridge(){
    free(this -> header);
}
/*
* Checks if the rom is an NES rom file.
*
*/
void cartridge::confirmRom(uint8_t *rom){
    assert(rom != NULL);
    auto header = reinterpret_cast<struct cartHeader*>(rom);
    assert(header -> id[0] == 'N');
    assert(header -> id[0] == 'E');
    assert(header -> id[0] == 'S');
    assert(header -> id[0] == 0x1a);
}

void cartridge::loadRom(uint8_t *rom){
    this -> header = reinterpret_cast<struct cartHeader*>(rom);
    confirmRom(rom);
    printf("ROM type verified...\n");
    this -> prgDataSize = header -> prgSize * 16384; //Gets the size of the program data in bytes.
    this -> chrDataSize = header -> chrSize * 8192; //Gets the size of the character data in bytes.
}