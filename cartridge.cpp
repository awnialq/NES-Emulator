//Note to self: use xxd on the nestest rom to check byte structure with WSL.

#include "cartridge.h"
#include <iostream>
#include <assert.h>

cartridge::cartridge(std::string &fName){
    this -> prgDataSize = 0;
    this -> chrDataSize = 0;
    this -> fileName = fName;
}

cartridge::~cartridge(){
    free(this -> header);
}
/*
* Check if the rom is an NES rom file.
*
*/
void cartridge::confirmRom(){
    assert(header -> id[0] == 'N');
    assert(header -> id[0] == 'E');
    assert(header -> id[0] == 'S');
    assert(header -> id[0] == 0x1a);
}
/*
* This function first opens the file with the filestream and reads the binary. it then checks if the file actually opened properly
* After doing so, it then stores the first 16 bytes of the file into a char array and that array then gets converted into a cart header.
* After that, the rom is confirmed to be an nes rom and the program and character data sizes are calculated.
*/
void cartridge::loadRom(){
    this -> fileStream.open(this -> fileName, std::ifstream::binary);
    if(this -> fileStream.is_open()){
        char* arr; 
        arr = strcpy(arr, this -> fileName.c_str());
        fileStream.read(arr, sizeof(struct cartHeader));
        this -> header = reinterpret_cast<struct cartHeader*>(arr);
        free(arr);
    }   
    cartridge::confirmRom();
    printf("ROM type verified...\n");
    this -> prgDataSize = header -> prgSize * 16384; //Gets the size of the program data in bytes.
    this -> chrDataSize = header -> chrSize * 8192; //Gets the size of the character data in bytes.
}