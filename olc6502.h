#pragma once
#include <cstdint>

class Bus;

class olc6502{
    public:
        olc6502();
        ~olc6502();

        void ConnectBus(Bus *bp) {bus  = bp;}
    public: 
        enum FLAGS6502{
            C = (1 << 0), //Carry
            Z = (1 << 1), //Zero Result
            I = (1 << 2), //Disable Interruptions
            D = (1 << 3), //Decimal Mode
            B = (1 << 4), //Break
            U = (1 << 5), //Not Used
            V = (1 << 6), //Overflow
            N = (1 << 7), //Negative
        };
    private:
        Bus *bus = nullptr;
        uint8_t read(uint16_t a);
        void write(uint16_t a, uint8_t d);
};
