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
            if(active_pulse1){
                pulse1.length_counter = length_counter_lookup[(data & 0xf8) >> 3];
            }
            pulse1.timer &= 0x00ff;
            pulse1.timer |= (data & 0x07) << 8;
            pulse1.envelope_start = true;
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
            if(active_pulse2){
                pulse2.length_counter = length_counter_lookup[(data & 0xf8) >> 3];
            }
            pulse2.timer &= 0x00ff;
            pulse2.timer |= (data & 0x07) << 8;
            pulse2.envelope_start = true;
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
            if(active_triangle){
                triangle.length_counter = length_counter_lookup[(data & 0xf8) >> 3];
            }
            triangle.linear_count_reload = true;
            triangle.timer &= 0x00ff;
            triangle.timer |= (data & 0x07) << 8;
            break;
        /* Noise channel cases */
        case 0x400C:
            noise.length_counter_halt = (data & 0x20) > 0;
            noise.const_volume = (data & 0x10) > 0;
            noise.volume = data & 0x0f;
            break;
        case 0x400E:
            noise.noise_mode = (data & 0x80) > 0;
            noise.noise_period = data & 0x0f;
            noise.current_timer = noise_period_lookup[noise.noise_period];
            break;
        case 0x400F:
            noise.length_counter_load = (data & 0xf8) >> 3;
            if(active_noise){
                noise.length_counter = length_counter_lookup[noise.length_counter_load];
            }
            noise.envelope_start = true;
            break;
        case 0x4015:
            active_pulse1 = (data & 0x01) > 0;
            active_pulse2 = (data & 0x02) > 0;
            active_triangle = (data & 0x04) > 0;
            active_noise = (data & 0x08) > 0;
            active_dmc = (data & 0x10) > 0;

            if(!active_pulse1) pulse1.length_counter = 0;
            if(!active_pulse2) pulse2.length_counter = 0;
            if(!active_triangle) triangle.length_counter = 0;
            if(!active_noise) noise.length_counter = 0;
            break;
        case 0x4017:
            // Handle frame counter register
        default:
            break;
    }
}

uint8_t apu::cpuRead(uint16_t addr){
    
    return 0x00;
}

void apu::pulse_clock(bool p1_or_p2){
    if(!p1_or_p2){
        // clock pulse1
        if(pulse1.current_timer == 0){  // will run intially which is more hardware accurage ig
            // Handle pulse1 timer overflow
            pulse1.current_timer = pulse1.timer;
            if(pulse1.sequence_step == 7){
                pulse1.sequence_step = 0;
            }
            else{
                ++pulse1.sequence_step;
            }
        }
        else{
            --pulse1.current_timer;
        }   




        turn_to_clock = false;
    }
    else{
        // clock pulse2
    }
}

void apu::clock(){
    if(turn_to_clock){
       
    }
    // if you don't clock rese the flag
    turn_to_clock = true;
}

double apu::get_sample(){
    return 0.0f;
}