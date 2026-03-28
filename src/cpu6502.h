#pragma once
#include "cartridge.h"
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>

class Bus;

class cpu6502{
    public:
        cpu6502(char *);
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
        //All the possible addressing modes for the cpu6502
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
            uint8_t(cpu6502::*operate)() = nullptr; //The operation function that coresponds with the respective operation.
            uint8_t(cpu6502::*addrmode)() = nullptr; //The addressing mode that the function will use
            uint8_t cycles = 0; //The # of cycles the instruction needs to fully execute.
        };
        inline static const std::vector<INSTRUCTION> lookup = {
                            //Horizontal (LSB) 0x0_
    /*Vertical 0x_0 (MSB)*/ { "BRK", &cpu6502::BRK, &cpu6502::IMM, 7 },{ "ORA", &cpu6502::ORA, &cpu6502::INDX, 6 },{ "???", &cpu6502::DUM, &cpu6502::DUM, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 8 },{ "???", &cpu6502::NOP, &cpu6502::ZP0, 3 },{ "ORA", &cpu6502::ORA, &cpu6502::ZP0, 3 },{ "ASL", &cpu6502::ASL, &cpu6502::ZP0, 5 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 5 },{ "PHP", &cpu6502::PHP, &cpu6502::IMP, 3 },{ "ORA", &cpu6502::ORA, &cpu6502::IMM, 2 },{ "ASL", &cpu6502::ASL, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "???", &cpu6502::NOP, &cpu6502::ABS, 4 },{ "ORA", &cpu6502::ORA, &cpu6502::ABS, 4 },{ "ASL", &cpu6502::ASL, &cpu6502::ABS, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 6 },
                            { "BPL", &cpu6502::BPL, &cpu6502::REL, 2 },{ "ORA", &cpu6502::ORA, &cpu6502::INDY, 5 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 8 },{ "???", &cpu6502::NOP, &cpu6502::ZPX, 4 },{ "ORA", &cpu6502::ORA, &cpu6502::ZPX, 4 },{ "ASL", &cpu6502::ASL, &cpu6502::ZPX, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 6 },{ "CLC", &cpu6502::CLC, &cpu6502::IMP, 2 },{ "ORA", &cpu6502::ORA, &cpu6502::ABY, 4 },{ "???", &cpu6502::NOP, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 7 },{ "???", &cpu6502::NOP, &cpu6502::ABX, 4 },{ "ORA", &cpu6502::ORA, &cpu6502::ABX, 4 },{ "ASL", &cpu6502::ASL, &cpu6502::ABX, 7 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 7 },
                            { "JSR", &cpu6502::JSR, &cpu6502::ABS, 6 },{ "AND", &cpu6502::AND, &cpu6502::INDX, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 8 },{ "BIT", &cpu6502::BIT, &cpu6502::ZP0, 3 },{ "AND", &cpu6502::AND, &cpu6502::ZP0, 3 },{ "ROL", &cpu6502::ROL, &cpu6502::ZP0, 5 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 5 },{ "PLP", &cpu6502::PLP, &cpu6502::IMP, 4 },{ "AND", &cpu6502::AND, &cpu6502::IMM, 2 },{ "ROL", &cpu6502::ROL, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "BIT", &cpu6502::BIT, &cpu6502::ABS, 4 },{ "AND", &cpu6502::AND, &cpu6502::ABS, 4 },{ "ROL", &cpu6502::ROL, &cpu6502::ABS, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 6 },
                            { "BMI", &cpu6502::BMI, &cpu6502::REL, 2 },{ "AND", &cpu6502::AND, &cpu6502::INDY, 5 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 8 },{ "???", &cpu6502::NOP, &cpu6502::ZPX, 4 },{ "AND", &cpu6502::AND, &cpu6502::ZPX, 4 },{ "ROL", &cpu6502::ROL, &cpu6502::ZPX, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 6 },{ "SEC", &cpu6502::SEC, &cpu6502::IMP, 2 },{ "AND", &cpu6502::AND, &cpu6502::ABY, 4 },{ "???", &cpu6502::NOP, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 7 },{ "???", &cpu6502::NOP, &cpu6502::ABX, 4 },{ "AND", &cpu6502::AND, &cpu6502::ABX, 4 },{ "ROL", &cpu6502::ROL, &cpu6502::ABX, 7 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 7 },
                            { "RTI", &cpu6502::RTI, &cpu6502::IMP, 6 },{ "EOR", &cpu6502::EOR, &cpu6502::INDX, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 8 },{ "???", &cpu6502::NOP, &cpu6502::ZP0, 3 },{ "EOR", &cpu6502::EOR, &cpu6502::ZP0, 3 },{ "LSR", &cpu6502::LSR, &cpu6502::ZP0, 5 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 5 },{ "PHA", &cpu6502::PHA, &cpu6502::IMP, 3 },{ "EOR", &cpu6502::EOR, &cpu6502::IMM, 2 },{ "LSR", &cpu6502::LSR, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "JMP", &cpu6502::JMP, &cpu6502::ABS, 3 },{ "EOR", &cpu6502::EOR, &cpu6502::ABS, 4 },{ "LSR", &cpu6502::LSR, &cpu6502::ABS, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 6 },
                            { "BVC", &cpu6502::BVC, &cpu6502::REL, 2 },{ "EOR", &cpu6502::EOR, &cpu6502::INDY, 5 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 8 },{ "???", &cpu6502::NOP, &cpu6502::ZPX, 4 },{ "EOR", &cpu6502::EOR, &cpu6502::ZPX, 4 },{ "LSR", &cpu6502::LSR, &cpu6502::ZPX, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 6 },{ "CLI", &cpu6502::CLI, &cpu6502::IMP, 2 },{ "EOR", &cpu6502::EOR, &cpu6502::ABY, 4 },{ "???", &cpu6502::NOP, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 7 },{ "???", &cpu6502::NOP, &cpu6502::ABX, 4 },{ "EOR", &cpu6502::EOR, &cpu6502::ABX, 4 },{ "LSR", &cpu6502::LSR, &cpu6502::ABX, 7 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 7 },
                            { "RTS", &cpu6502::RTS, &cpu6502::IMP, 6 },{ "ADC", &cpu6502::ADC, &cpu6502::INDX, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 8 },{ "???", &cpu6502::NOP, &cpu6502::ZP0, 3 },{ "ADC", &cpu6502::ADC, &cpu6502::ZP0, 3 },{ "ROR", &cpu6502::ROR, &cpu6502::ZP0, 5 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 5 },{ "PLA", &cpu6502::PLA, &cpu6502::IMP, 4 },{ "ADC", &cpu6502::ADC, &cpu6502::IMM, 2 },{ "ROR", &cpu6502::ROR, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "JMP", &cpu6502::JMP, &cpu6502::IND, 5 },{ "ADC", &cpu6502::ADC, &cpu6502::ABS, 4 },{ "ROR", &cpu6502::ROR, &cpu6502::ABS, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 6 },
                            { "BVS", &cpu6502::BVS, &cpu6502::REL, 2 },{ "ADC", &cpu6502::ADC, &cpu6502::INDY, 5 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 8 },{ "???", &cpu6502::NOP, &cpu6502::ZPX, 4 },{ "ADC", &cpu6502::ADC, &cpu6502::ZPX, 4 },{ "ROR", &cpu6502::ROR, &cpu6502::ZPX, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 6 },{ "SEI", &cpu6502::SEI, &cpu6502::IMP, 2 },{ "ADC", &cpu6502::ADC, &cpu6502::ABY, 4 },{ "???", &cpu6502::NOP, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 7 },{ "???", &cpu6502::NOP, &cpu6502::ABX, 4 },{ "ADC", &cpu6502::ADC, &cpu6502::ABX, 4 },{ "ROR", &cpu6502::ROR, &cpu6502::ABX, 7 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 7 },
                            { "???", &cpu6502::NOP, &cpu6502::IMM, 2 },{ "STA", &cpu6502::STA, &cpu6502::INDX, 6 },{ "???", &cpu6502::NOP, &cpu6502::IMM, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 6 },{ "STY", &cpu6502::STY, &cpu6502::ZP0, 3 },{ "STA", &cpu6502::STA, &cpu6502::ZP0, 3 },{ "STX", &cpu6502::STX, &cpu6502::ZP0, 3 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 3 },{ "DEY", &cpu6502::DEY, &cpu6502::IMP, 2 },{ "???", &cpu6502::NOP, &cpu6502::IMM, 2 },{ "TXA", &cpu6502::TXA, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "STY", &cpu6502::STY, &cpu6502::ABS, 4 },{ "STA", &cpu6502::STA, &cpu6502::ABS, 4 },{ "STX", &cpu6502::STX, &cpu6502::ABS, 4 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 4 },
                            { "BCC", &cpu6502::BCC, &cpu6502::REL, 2 },{ "STA", &cpu6502::STA, &cpu6502::INDY, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 6 },{ "STY", &cpu6502::STY, &cpu6502::ZPX, 4 },{ "STA", &cpu6502::STA, &cpu6502::ZPX, 4 },{ "STX", &cpu6502::STX, &cpu6502::ZPY, 4 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 4 },{ "TYA", &cpu6502::TYA, &cpu6502::IMP, 2 },{ "STA", &cpu6502::STA, &cpu6502::ABY, 5 },{ "TXS", &cpu6502::TXS, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 5 },{ "???", &cpu6502::NOP, &cpu6502::IMP, 5 },{ "STA", &cpu6502::STA, &cpu6502::ABX, 5 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 5 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 5 },
                            { "LDY", &cpu6502::LDY, &cpu6502::IMM, 2 },{ "LDA", &cpu6502::LDA, &cpu6502::INDX, 6 },{ "LDX", &cpu6502::LDX, &cpu6502::IMM, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 6 },{ "LDY", &cpu6502::LDY, &cpu6502::ZP0, 3 },{ "LDA", &cpu6502::LDA, &cpu6502::ZP0, 3 },{ "LDX", &cpu6502::LDX, &cpu6502::ZP0, 3 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 3 },{ "TAY", &cpu6502::TAY, &cpu6502::IMP, 2 },{ "LDA", &cpu6502::LDA, &cpu6502::IMM, 2 },{ "TAX", &cpu6502::TAX, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "LDY", &cpu6502::LDY, &cpu6502::ABS, 4 },{ "LDA", &cpu6502::LDA, &cpu6502::ABS, 4 },{ "LDX", &cpu6502::LDX, &cpu6502::ABS, 4 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 4 },
                            { "BCS", &cpu6502::BCS, &cpu6502::REL, 2 },{ "LDA", &cpu6502::LDA, &cpu6502::INDY, 5 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 5 },{ "LDY", &cpu6502::LDY, &cpu6502::ZPX, 4 },{ "LDA", &cpu6502::LDA, &cpu6502::ZPX, 4 },{ "LDX", &cpu6502::LDX, &cpu6502::ZPY, 4 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 4 },{ "CLV", &cpu6502::CLV, &cpu6502::IMP, 2 },{ "LDA", &cpu6502::LDA, &cpu6502::ABY, 4 },{ "TSX", &cpu6502::TSX, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 4 },{ "LDY", &cpu6502::LDY, &cpu6502::ABX, 4 },{ "LDA", &cpu6502::LDA, &cpu6502::ABX, 4 },{ "LDX", &cpu6502::LDX, &cpu6502::ABY, 4 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 4 },
                            { "CPY", &cpu6502::CPY, &cpu6502::IMM, 2 },{ "CMP", &cpu6502::CMP, &cpu6502::INDX, 6 },{ "???", &cpu6502::NOP, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 8 },{ "CPY", &cpu6502::CPY, &cpu6502::ZP0, 3 },{ "CMP", &cpu6502::CMP, &cpu6502::ZP0, 3 },{ "DEC", &cpu6502::DEC, &cpu6502::ZP0, 5 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 5 },{ "INY", &cpu6502::INY, &cpu6502::IMP, 2 },{ "CMP", &cpu6502::CMP, &cpu6502::IMM, 2 },{ "DEX", &cpu6502::DEX, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "CPY", &cpu6502::CPY, &cpu6502::ABS, 4 },{ "CMP", &cpu6502::CMP, &cpu6502::ABS, 4 },{ "DEC", &cpu6502::DEC, &cpu6502::ABS, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 6 },
                            { "BNE", &cpu6502::BNE, &cpu6502::REL, 2 },{ "CMP", &cpu6502::CMP, &cpu6502::INDY, 5 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 8 },{ "???", &cpu6502::NOP, &cpu6502::ZPX, 4 },{ "CMP", &cpu6502::CMP, &cpu6502::ZPX, 4 },{ "DEC", &cpu6502::DEC, &cpu6502::ZPX, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 6 },{ "CLD", &cpu6502::CLD, &cpu6502::IMP, 2 },{ "CMP", &cpu6502::CMP, &cpu6502::ABY, 4 },{ "NOP", &cpu6502::NOP, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 7 },{ "???", &cpu6502::NOP, &cpu6502::ABX, 4 },{ "CMP", &cpu6502::CMP, &cpu6502::ABX, 4 },{ "DEC", &cpu6502::DEC, &cpu6502::ABX, 7 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 7 },
                            { "CPX", &cpu6502::CPX, &cpu6502::IMM, 2 },{ "SBC", &cpu6502::SBC, &cpu6502::INDX, 6 },{ "???", &cpu6502::NOP, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 8 },{ "CPX", &cpu6502::CPX, &cpu6502::ZP0, 3 },{ "SBC", &cpu6502::SBC, &cpu6502::ZP0, 3 },{ "INC", &cpu6502::INC, &cpu6502::ZP0, 5 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 5 },{ "INX", &cpu6502::INX, &cpu6502::IMP, 2 },{ "SBC", &cpu6502::SBC, &cpu6502::IMM, 2 },{ "NOP", &cpu6502::NOP, &cpu6502::IMP, 2 },{ "???", &cpu6502::SBC, &cpu6502::IMP, 2 },{ "CPX", &cpu6502::CPX, &cpu6502::ABS, 4 },{ "SBC", &cpu6502::SBC, &cpu6502::ABS, 4 },{ "INC", &cpu6502::INC, &cpu6502::ABS, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 6 },
                            { "BEQ", &cpu6502::BEQ, &cpu6502::REL, 2 },{ "SBC", &cpu6502::SBC, &cpu6502::INDY, 5 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 8 },{ "???", &cpu6502::NOP, &cpu6502::ZPX, 4 },{ "SBC", &cpu6502::SBC, &cpu6502::ZPX, 4 },{ "INC", &cpu6502::INC, &cpu6502::ZPX, 6 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 6 },{ "SED", &cpu6502::SED, &cpu6502::IMP, 2 },{ "SBC", &cpu6502::SBC, &cpu6502::ABY, 4 },{ "NOP", &cpu6502::NOP, &cpu6502::IMP, 2 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 7 },{ "???", &cpu6502::NOP, &cpu6502::ABX, 4 },{ "SBC", &cpu6502::SBC, &cpu6502::ABX, 4 },{ "INC", &cpu6502::INC, &cpu6502::ABX, 7 },{ "???", &cpu6502::DUM, &cpu6502::IMP, 7 },
	};
        std::ofstream cpuLog;
};
