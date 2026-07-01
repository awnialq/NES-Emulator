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
        bool active_pulse1 = false;
        double pulse1 = 0.0;

};