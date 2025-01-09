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
        //All the possible addressing modes for the CPU
        uint8_t IMP(); //Implicit
        uint8_t IMM(); //Immediate
        uint8_t ZP0(); //Zero Page 0
        uint8_t ZPX(); //Zero Page X
        uint8_t ZPY(); //Zero Page Y
        uint8_t REL(); //Relative
        uint8_t ABS(); //Absolute
        uint8_t ABX(); //Absolute X
        uint8_t ABY(); //Absolute Y
        uint8_t IND(); //Indirect  
    private:
        //All OpCodes - https://www.masswerk.at/6502/6502_instruction_set.html#BPL - every column left to right bottom down before moving onto the next one.
        uint8_t BRK(); /*Break*/                          
        uint8_t BPL(); /*Branch on plus*/            
        uint8_t JSR(); /*Jump to Subroutine*/             
        uint8_t BMI(); /*Branch on Minus negative set*/   
        uint8_t RTI(); /*Return from Interrupt*/          
        uint8_t BVC(); /*Branch on Overflow Clear*/       
        uint8_t RTS(); /*Return from Subroutine*/         
        uint8_t BVS(); /*Branch on Overflow Set*/         
        uint8_t BCC(); /*Branch on Carry Clear*/          
        uint8_t LDY(); /*Load Y Register*/
        uint8_t BCS(); /*Branch on Carry Set*/            
        uint8_t CPY(); /*Compare Y Register*/             
        uint8_t BNE(); /*Branch on Not Equal*/            
        uint8_t CPX(); /*Compare X Register*/
        uint8_t BEQ(); /*Branch on Equal*/
        //Column 2
        uint8_t ORA(); /*Or with accumulator*/
        uint8_t AND();/*And*/
        uint8_t EOR(); /*Exclusive Or*/
        uint8_t ADC(); /*Add with Carry*/
        uint8_t STA(); /*Store Accumulator*/
        uint8_t LDA(); /*Load Accumulator*/
        uint8_t CMP(); /*Compare with accumulator*/
        uint8_t SBC(); /*Subtract with Carry*/
        //Column 3
        uint8_t LDX(); /*Load X Register*/
        //Collumn 4
        
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
