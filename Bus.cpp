#include "Bus.h"
#include "cartridge.h"
#include <cstdint>
#include <cstdio>


 

Bus::Bus(){
    for(auto &i : cpuMem){i = 0x00;}
    cart = new cartridge();
    if(cart->initCart() == 1){
        printf("Cart succesfully loaded\n");
    }
}

Bus::~Bus(){

}

void Bus::cpuWrite(uint16_t addr, uint8_t data){
    if(addr >= 0x0000 && addr <= 0x1FFF){
        cpuMem[addr & 0x1FFF] = data;
    }
    /*
    else if(addr >= 0x2000 && addr <= 0x3FFF){
        ppu.cpuWrite(addr & 0x0007, data);  //implement addr mirroring
    }
    */
}

uint8_t Bus::cpuRead(uint16_t addr, bool readOnly){
    uint8_t data = 0x00;
    if(addr >= 0x0000 && addr <= 0x1FFF){
        data = cpuMem[addr & 0x07FF];  //Implemented the addr mirroring by anding with 2kb
    }
    /*
    else if(addr >= 0x2000 && addr <= 0x3FFF){
        data = ppu.cpuRead(addr & 0x0007, readOnly);
    }
    */
    if(addr >= 0xC000){
        data = cart->prgMem[addr - 0xC000];
    }
    

    return data;
}

