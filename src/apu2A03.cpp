#include "apu2A03.h"
#include <cstdint>

using apu = apu2A03;


apu::apu2A03(){

}

apu::~apu2A03(){

}

void apu::cpuWrite(uint16_t addr, uint8_t data){

}

uint8_t apu::cpuRead(uint16_t addr){
    
    return 0x00;
}

void apu::clock(){

}

double apu::get_sample(){
    return pulse1;
}