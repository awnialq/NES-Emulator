#pragma once
#include "cartridge.h"
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>

class Bus;

class cpu6502{
    public:
        cpu6502();
        ~cpu6502();
    public: 
        enum FLAGS6502{
            C = (1 << 0), //Carry
            Z = (1 << 1), //Zero Result
            I = (1 << 2), //Disable Interruptions
            D = (1 << 3), //Decimal Mode (not supported by NES)
            B = (1 << 4), //Break
            U = (1 << 5), //Not Used
            V = (1 << 6), //Overflow
            N = (1 << 7), //Negative
        };

        uint8_t accum = 0x00; //Accumulator Register
        uint8_t x = 0x00; //X Register
        uint8_t y = 0x00; //Y Register
        uint8_t stackp = 0xFD; //Stack Pointer
        uint16_t progc = 0xC000; //Status Register
        uint8_t status = 0x24; //Reps the Status Register

        void ConnectBus(Bus *bp) {bus  = bp;}

        void clock();
        void reset();
        void interruptReq(); //Can be ignored with the Disable Interruptions Flag is active
        void nonMskInter(); //CAN NOT be ignored
        std::string cpuStatusLog();
        std::string cpuLog_clean();
        std::string addrmodeName(uint8_t(cpu6502::*addrmode)());

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
        uint8_t INDX(); //Indirect x
        uint8_t INDY(); //Indirect Y
    private:
        //All OpCodes - https://www.masswerk.at/6502/6502_instruction_set.html#BPL
        uint8_t ADC(); // add with carry
        uint8_t AND(); // and (with accumulator)
        uint8_t ASL(); // arithmetic shift left
        uint8_t BCC(); // branch on carry clear
        uint8_t BCS(); // branch on carry set
        uint8_t BEQ(); // branch on equal (zero set)
        uint8_t BIT(); // bit test
        uint8_t BMI(); // branch on minus (negative set)
        uint8_t BNE(); // branch on not equal (zero clear)
        uint8_t BPL(); // branch on plus (negative clear)
        uint8_t BRK(); // break / interrupt
        uint8_t BVC(); // branch on overflow clear
        uint8_t BVS(); // branch on overflow set
        uint8_t CLC(); // clear carry
        uint8_t CLD(); // clear decimal
        uint8_t CLI(); // clear interrupt disable
        uint8_t CLV(); // clear overflow
        uint8_t CMP(); // compare (with accumulator)
        uint8_t CPX(); // compare with X
        uint8_t CPY(); // compare with Y
        uint8_t DEC(); // decrement
        uint8_t DEX(); // decrement X
        uint8_t DEY(); // decrement Y
        uint8_t EOR(); // exclusive or (with accumulator)
        uint8_t INC(); // increment
        uint8_t INX(); // increment X
        uint8_t INY(); // increment Y
        uint8_t JMP(); // jump
        uint8_t JSR(); // jump subroutine
        uint8_t LDA(); // load accumulator
        uint8_t LDX(); // load X
        uint8_t LDY(); // load Y
        uint8_t LSR(); // logical shift right
        uint8_t NOP(); // no operation
        uint8_t ORA(); // or with accumulator
        uint8_t PHA(); // push accumulator
        uint8_t PHP(); // push processor status (SR)
        uint8_t PLA(); // pull accumulator
        uint8_t PLP(); // pull processor status (SR)
        uint8_t ROL(); // rotate left
        uint8_t ROR(); // rotate right
        uint8_t RTI(); // return from interrupt
        uint8_t RTS(); // return from subroutine
        uint8_t SBC(); // subtract with carry
        uint8_t SEC(); // set carry
        uint8_t SED(); // set decimal
        uint8_t SEI(); // set interrupt disable
        uint8_t STA(); // store accumulator
        uint8_t STX(); // store X
        uint8_t STY(); // store Y
        uint8_t TAX(); // transfer accumulator to X
        uint8_t TAY(); // transfer accumulator to Y
        uint8_t TSX(); // transfer stack pointer to X
        uint8_t TXA(); // transfer X to accumulator
        uint8_t TXS(); // transfer X to stack pointer
        uint8_t TYA(); // transfer Y to accumulator
        uint8_t DUM(); // dummy instruction for non-defined opcodes
    public:
        Bus *bus = nullptr;
        uint8_t read(uint16_t a);
        void write(uint16_t a, uint8_t d);
        uint8_t getFlag(FLAGS6502 f);
        void setFlag(FLAGS6502 f, bool v);
        struct INSTRUCTION{
            std::string name;
            uint8_t(cpu6502::*operate)(void) = nullptr; //The operation function that coresponds with the respective operation.
            uint8_t(cpu6502::*addrmode)(void) = nullptr; //The addressing mode that the function will use
            uint8_t cycles = 0; //The # of cycles the instruction needs to fully execute.
        };
        std::vector<INSTRUCTION> lookup;
        std::ofstream cpuLog;
};
