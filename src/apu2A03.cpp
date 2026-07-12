#include "apu2A03.h"
#include <cstdint>

using apu = apu2A03;


apu::apu2A03(){

}

apu::~apu2A03(){

}

void apu::cpuWrite(uint16_t addr, uint8_t data){
    switch(addr){
        /* pulse wave 1 and 2 cases */
        case 0x4000:
            pulse1.duty = (data & 0xc0) >> 6;
            pulse1.length_counter_halt = (data & 0x20) > 0;
            pulse1.const_volume = (data & 0x10) > 0;
            pulse1.volume = data & 0x0f;
            break;
        case 0x4001:
            pulse1.sweep_active = ((data & 0x80) >> 7) > 0;
            pulse1.sweep_period = (data & 0x70) >> 4;
            pulse1.sweep_negate = (data & 0x08) > 0;
            pulse1.sweep_shift = data & 0x07;
            break;
        case 0x4002:
            pulse1.timer &= 0xff00;
            pulse1.timer |= data;
            break;
        case 0x4003:
            pulse1.length_counter = (data & 0xf8) >> 3;
            pulse1.timer &= 0x00ff;
            pulse1.timer |= (data & 0x07) << 8;
            break;
        case 0x4004:
            pulse2.duty = (data & 0xc0) >> 6;
            pulse2.length_counter_halt = (data & 0x20) > 0;
            pulse2.const_volume = (data & 0x10) > 0;
            pulse2.volume = data & 0x0f;
            break;
        case 0x4005:
            pulse2.sweep_active = ((data & 0x80) >> 7) > 0;
            pulse2.sweep_period = (data & 0x70) >> 4;
            pulse2.sweep_negate = (data & 0x08) > 0;
            pulse2.sweep_shift = data & 0x07;
            break;
        case 0x4006:
            pulse2.timer &= 0xff00;
            pulse2.timer |= data;
            break;
        case 0x4007:
            pulse2.length_counter = (data & 0xf8) >> 3;
            pulse2.timer &= 0x00ff;
            pulse2.timer |= (data & 0x07) << 8;
            break;
        /* triangle wave cases */
        case 0x4008:
            triangle.linear_count_control = (data & 0x80) > 0;
            triangle.linear_count_load = data & 0x7f;
            break;
        case 0x400A:
            triangle.timer &= 0xff00;
            triangle.timer |= data;
            break;
        case 0x400B:
            triangle.length_counter = (data & 0xf8) >> 3;
            triangle.linear_count_reload = true;
            triangle.timer &= 0x00ff;
            triangle.timer |= (data & 0x07) << 8;
            break;
        default:
            // temp will add triangle, noise,dmc, status, and frame counter support
            break;
    }
}

uint8_t apu::cpuRead(uint16_t addr){
    
    return 0x00;
}

void apu::clock(){

}

double apu::get_sample(){
    return 0.0f;
}