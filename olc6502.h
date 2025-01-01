#pragma once
#include <cstdint>
#include <string>
#include <vector>

class Bus;

class olc6502{
    public:
        olc6502();
        ~olc6502();
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

        uint8_t accum = 0x00; //Accumulator Register
        uint8_t x = 0x00; //X Register
        uint8_t y = 0x00; //Y Register
        uint8_t stackp = 0x00; //Stack Pointer
        uint16_t progc = 0x0000; //Status Register
        uint8_t status = 0x00; //Reps the Status Register

        void ConnectBus(Bus *bp) {bus  = bp;}
        //All the possible addressing modes for the CPU
        uint8_t IMP();
        uint8_t ZP0();

        void clock();
        void reset();
        void interruptReq(); //Can be ignored with the Disable Interruptions Flag is active
        void nonMskInter(); //CAN NOT be ignored

        uint8_t fetch();
        uint8_t fetched = 0x00;

        uint16_t addr_absolute = 0x0000;
        uint16_t addr_relatvie = 0x00;
        uint8_t opcode = 0x00;
        uint8_t cycles = 0; 

    private:
        Bus *bus = nullptr;
        uint8_t read(uint16_t a);
        void write(uint16_t a, uint8_t d);
        uint8_t getFlag(FLAGS6502 f);
        void setFlag(FLAGS6502 f, bool v);
        struct INSTRUCTION{
            std::string name;
            uint8_t(olc6502::*operate)(void) = nullptr; //The operation function that coresponds with the respective operation.
            uint8_t(olc6502::*addrmode)(void) = nullptr; //The addressing mode that the function will use
            uint8_t cycles = 0; //The # of cycles the instruction needs to fully execute.
        };
        std::vector<INSTRUCTION> lookup;
};
