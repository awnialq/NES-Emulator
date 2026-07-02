#pragma once
#include <cstdint>

class apu2A03{

    public:
        apu2A03();
        ~apu2A03();
    
    public:
        void cpuWrite(uint16_t addr, uint8_t data);
        uint8_t cpuRead(uint16_t addr);
        void clock();
        void reset();
        double get_sample();
    
    private:
        struct sequencer{
            uint32_t sequencer = 0x00000000;
            uint16_t timer = 0x0000;
            uint16_t reload = 0x0000;
            uint8_t output = 0x00;
        };
        
        bool active_pulse1 = false;
        double pulse1 = 0.0;

};