#include "Bus.h"
#include <cstdio>


 

Bus::Bus(){
    for(auto &i : cpuMem){i = 0x00;}
    cpuMem[0] = 0xA2;
    cpuMem[1] = 0x00;
    cpuMem[2] = 0xA2;
    cpuMem[3] = 0x10;
    cpuMem[4] = 0xA2;
    cpuMem[5] = 0xFF;
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
    

    return data;
}

void Bus::insertCart(const std::shared_ptr<cartridge>& cartridge){
    this->game = cartridge;
    //ppu.connectCart(cartridge);
}
