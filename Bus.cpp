#include "Bus.h"
 

Bus::Bus(){
    for(auto &i : wires){i = 0x00;}

    cpu.ConnectBus(this);
}

Bus::~Bus(){

}

void Bus::write(uint16_t addr, uint8_t data){
    if(addr >= 0x0000 && addr <= 0x4020){
        wires[addr] = data;
    }
}
uint8_t Bus::read(uint16_t addr, bool readOnly = false){
    if(addr >= 0x0000 && addr <= 0xFFFF){
        return wires[addr];
    }
    return 0x00;
}