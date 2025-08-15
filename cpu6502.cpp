#include "cpu6502.h"
#include "Bus.h"
#include "cartridge.h"
#include <cstdint>
#include <cstdio>
#include <string>
#include <format>
#include <cassert>

using cpu = cpu6502; //Creates a temporary naming variable to make the table more simple.

cpu::cpu6502(){
    bus = new Bus();
    cpuLog.open("cpu_log.txt", std::ios::out | std::ios::trunc); // Overwrite each run
    assert(cpuLog.is_open());
    cpuLog << cpuLog_clean() << std::endl;
    /*Illegal Opcodes have not fully been implemented so yea.*/this->lookup = {
                            //Horizontal (LSB) 0x0_
    /*Vertical 0x_0 (MSB)*/ { "BRK", &cpu::BRK, &cpu::IMM, 7 },{ "ORA", &cpu::ORA, &cpu::INDX, 6 },{ "???", &cpu::DUM, &cpu::DUM, 2 },{ "???", &cpu::DUM, &cpu::IMP, 8 },{ "???", &cpu::NOP, &cpu::ZP0, 3 },{ "ORA", &cpu::ORA, &cpu::ZP0, 3 },{ "ASL", &cpu::ASL, &cpu::ZP0, 5 },{ "???", &cpu::DUM, &cpu::IMP, 5 },{ "PHP", &cpu::PHP, &cpu::IMP, 3 },{ "ORA", &cpu::ORA, &cpu::IMM, 2 },{ "ASL", &cpu::ASL, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "???", &cpu::NOP, &cpu::ABS, 4 },{ "ORA", &cpu::ORA, &cpu::ABS, 4 },{ "ASL", &cpu::ASL, &cpu::ABS, 6 },{ "???", &cpu::DUM, &cpu::IMP, 6 },
                            { "BPL", &cpu::BPL, &cpu::REL, 2 },{ "ORA", &cpu::ORA, &cpu::INDY, 5 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 8 },{ "???", &cpu::NOP, &cpu::ZPX, 4 },{ "ORA", &cpu::ORA, &cpu::ZPX, 4 },{ "ASL", &cpu::ASL, &cpu::ZPX, 6 },{ "???", &cpu::DUM, &cpu::IMP, 6 },{ "CLC", &cpu::CLC, &cpu::IMP, 2 },{ "ORA", &cpu::ORA, &cpu::ABY, 4 },{ "???", &cpu::NOP, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 7 },{ "???", &cpu::NOP, &cpu::ABX, 4 },{ "ORA", &cpu::ORA, &cpu::ABX, 4 },{ "ASL", &cpu::ASL, &cpu::ABX, 7 },{ "???", &cpu::DUM, &cpu::IMP, 7 },
                            { "JSR", &cpu::JSR, &cpu::ABS, 6 },{ "AND", &cpu::AND, &cpu::INDX, 6 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 8 },{ "BIT", &cpu::BIT, &cpu::ZP0, 3 },{ "AND", &cpu::AND, &cpu::ZP0, 3 },{ "ROL", &cpu::ROL, &cpu::ZP0, 5 },{ "???", &cpu::DUM, &cpu::IMP, 5 },{ "PLP", &cpu::PLP, &cpu::IMP, 4 },{ "AND", &cpu::AND, &cpu::IMM, 2 },{ "ROL", &cpu::ROL, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "BIT", &cpu::BIT, &cpu::ABS, 4 },{ "AND", &cpu::AND, &cpu::ABS, 4 },{ "ROL", &cpu::ROL, &cpu::ABS, 6 },{ "???", &cpu::DUM, &cpu::IMP, 6 },
                            { "BMI", &cpu::BMI, &cpu::REL, 2 },{ "AND", &cpu::AND, &cpu::INDY, 5 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 8 },{ "???", &cpu::NOP, &cpu::ZPX, 4 },{ "AND", &cpu::AND, &cpu::ZPX, 4 },{ "ROL", &cpu::ROL, &cpu::ZPX, 6 },{ "???", &cpu::DUM, &cpu::IMP, 6 },{ "SEC", &cpu::SEC, &cpu::IMP, 2 },{ "AND", &cpu::AND, &cpu::ABY, 4 },{ "???", &cpu::NOP, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 7 },{ "???", &cpu::NOP, &cpu::ABX, 4 },{ "AND", &cpu::AND, &cpu::ABX, 4 },{ "ROL", &cpu::ROL, &cpu::ABX, 7 },{ "???", &cpu::DUM, &cpu::IMP, 7 },
                            { "RTI", &cpu::RTI, &cpu::IMP, 6 },{ "EOR", &cpu::EOR, &cpu::INDX, 6 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 8 },{ "???", &cpu::NOP, &cpu::ZP0, 3 },{ "EOR", &cpu::EOR, &cpu::ZP0, 3 },{ "LSR", &cpu::LSR, &cpu::ZP0, 5 },{ "???", &cpu::DUM, &cpu::IMP, 5 },{ "PHA", &cpu::PHA, &cpu::IMP, 3 },{ "EOR", &cpu::EOR, &cpu::IMM, 2 },{ "LSR", &cpu::LSR, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "JMP", &cpu::JMP, &cpu::ABS, 3 },{ "EOR", &cpu::EOR, &cpu::ABS, 4 },{ "LSR", &cpu::LSR, &cpu::ABS, 6 },{ "???", &cpu::DUM, &cpu::IMP, 6 },
                            { "BVC", &cpu::BVC, &cpu::REL, 2 },{ "EOR", &cpu::EOR, &cpu::INDY, 5 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 8 },{ "???", &cpu::NOP, &cpu::ZPX, 4 },{ "EOR", &cpu::EOR, &cpu::ZPX, 4 },{ "LSR", &cpu::LSR, &cpu::ZPX, 6 },{ "???", &cpu::DUM, &cpu::IMP, 6 },{ "CLI", &cpu::CLI, &cpu::IMP, 2 },{ "EOR", &cpu::EOR, &cpu::ABY, 4 },{ "???", &cpu::NOP, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 7 },{ "???", &cpu::NOP, &cpu::ABX, 4 },{ "EOR", &cpu::EOR, &cpu::ABX, 4 },{ "LSR", &cpu::LSR, &cpu::ABX, 7 },{ "???", &cpu::DUM, &cpu::IMP, 7 },
                            { "RTS", &cpu::RTS, &cpu::IMP, 6 },{ "ADC", &cpu::ADC, &cpu::INDX, 6 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 8 },{ "???", &cpu::NOP, &cpu::ZP0, 3 },{ "ADC", &cpu::ADC, &cpu::ZP0, 3 },{ "ROR", &cpu::ROR, &cpu::ZP0, 5 },{ "???", &cpu::DUM, &cpu::IMP, 5 },{ "PLA", &cpu::PLA, &cpu::IMP, 4 },{ "ADC", &cpu::ADC, &cpu::IMM, 2 },{ "ROR", &cpu::ROR, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "JMP", &cpu::JMP, &cpu::IND, 5 },{ "ADC", &cpu::ADC, &cpu::ABS, 4 },{ "ROR", &cpu::ROR, &cpu::ABS, 6 },{ "???", &cpu::DUM, &cpu::IMP, 6 },
                            { "BVS", &cpu::BVS, &cpu::REL, 2 },{ "ADC", &cpu::ADC, &cpu::INDY, 5 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 8 },{ "???", &cpu::NOP, &cpu::ZPX, 4 },{ "ADC", &cpu::ADC, &cpu::ZPX, 4 },{ "ROR", &cpu::ROR, &cpu::ZPX, 6 },{ "???", &cpu::DUM, &cpu::IMP, 6 },{ "SEI", &cpu::SEI, &cpu::IMP, 2 },{ "ADC", &cpu::ADC, &cpu::ABY, 4 },{ "???", &cpu::NOP, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 7 },{ "???", &cpu::NOP, &cpu::ABX, 4 },{ "ADC", &cpu::ADC, &cpu::ABX, 4 },{ "ROR", &cpu::ROR, &cpu::ABX, 7 },{ "???", &cpu::DUM, &cpu::IMP, 7 },
                            { "???", &cpu::NOP, &cpu::IMM, 2 },{ "STA", &cpu::STA, &cpu::INDX, 6 },{ "???", &cpu::NOP, &cpu::IMM, 2 },{ "???", &cpu::DUM, &cpu::IMP, 6 },{ "STY", &cpu::STY, &cpu::ZP0, 3 },{ "STA", &cpu::STA, &cpu::ZP0, 3 },{ "STX", &cpu::STX, &cpu::ZP0, 3 },{ "???", &cpu::DUM, &cpu::IMP, 3 },{ "DEY", &cpu::DEY, &cpu::IMP, 2 },{ "???", &cpu::NOP, &cpu::IMM, 2 },{ "TXA", &cpu::TXA, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "STY", &cpu::STY, &cpu::ABS, 4 },{ "STA", &cpu::STA, &cpu::ABS, 4 },{ "STX", &cpu::STX, &cpu::ABS, 4 },{ "???", &cpu::DUM, &cpu::IMP, 4 },
                            { "BCC", &cpu::BCC, &cpu::REL, 2 },{ "STA", &cpu::STA, &cpu::INDY, 6 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 6 },{ "STY", &cpu::STY, &cpu::ZPX, 4 },{ "STA", &cpu::STA, &cpu::ZPX, 4 },{ "STX", &cpu::STX, &cpu::ZPY, 4 },{ "???", &cpu::DUM, &cpu::IMP, 4 },{ "TYA", &cpu::TYA, &cpu::IMP, 2 },{ "STA", &cpu::STA, &cpu::ABY, 5 },{ "TXS", &cpu::TXS, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 5 },{ "???", &cpu::NOP, &cpu::IMP, 5 },{ "STA", &cpu::STA, &cpu::ABX, 5 },{ "???", &cpu::DUM, &cpu::IMP, 5 },{ "???", &cpu::DUM, &cpu::IMP, 5 },
                            { "LDY", &cpu::LDY, &cpu::IMM, 2 },{ "LDA", &cpu::LDA, &cpu::INDX, 6 },{ "LDX", &cpu::LDX, &cpu::IMM, 2 },{ "???", &cpu::DUM, &cpu::IMP, 6 },{ "LDY", &cpu::LDY, &cpu::ZP0, 3 },{ "LDA", &cpu::LDA, &cpu::ZP0, 3 },{ "LDX", &cpu::LDX, &cpu::ZP0, 3 },{ "???", &cpu::DUM, &cpu::IMP, 3 },{ "TAY", &cpu::TAY, &cpu::IMP, 2 },{ "LDA", &cpu::LDA, &cpu::IMM, 2 },{ "TAX", &cpu::TAX, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "LDY", &cpu::LDY, &cpu::ABS, 4 },{ "LDA", &cpu::LDA, &cpu::ABS, 4 },{ "LDX", &cpu::LDX, &cpu::ABS, 4 },{ "???", &cpu::DUM, &cpu::IMP, 4 },
                            { "BCS", &cpu::BCS, &cpu::REL, 2 },{ "LDA", &cpu::LDA, &cpu::INDY, 5 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 5 },{ "LDY", &cpu::LDY, &cpu::ZPX, 4 },{ "LDA", &cpu::LDA, &cpu::ZPX, 4 },{ "LDX", &cpu::LDX, &cpu::ZPY, 4 },{ "???", &cpu::DUM, &cpu::IMP, 4 },{ "CLV", &cpu::CLV, &cpu::IMP, 2 },{ "LDA", &cpu::LDA, &cpu::ABY, 4 },{ "TSX", &cpu::TSX, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 4 },{ "LDY", &cpu::LDY, &cpu::ABX, 4 },{ "LDA", &cpu::LDA, &cpu::ABX, 4 },{ "LDX", &cpu::LDX, &cpu::ABY, 4 },{ "???", &cpu::DUM, &cpu::IMP, 4 },
                            { "CPY", &cpu::CPY, &cpu::IMM, 2 },{ "CMP", &cpu::CMP, &cpu::INDX, 6 },{ "???", &cpu::NOP, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 8 },{ "CPY", &cpu::CPY, &cpu::ZP0, 3 },{ "CMP", &cpu::CMP, &cpu::ZP0, 3 },{ "DEC", &cpu::DEC, &cpu::ZP0, 5 },{ "???", &cpu::DUM, &cpu::IMP, 5 },{ "INY", &cpu::INY, &cpu::IMP, 2 },{ "CMP", &cpu::CMP, &cpu::IMM, 2 },{ "DEX", &cpu::DEX, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "CPY", &cpu::CPY, &cpu::ABS, 4 },{ "CMP", &cpu::CMP, &cpu::ABS, 4 },{ "DEC", &cpu::DEC, &cpu::ABS, 6 },{ "???", &cpu::DUM, &cpu::IMP, 6 },
                            { "BNE", &cpu::BNE, &cpu::REL, 2 },{ "CMP", &cpu::CMP, &cpu::INDY, 5 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 8 },{ "???", &cpu::NOP, &cpu::ZPX, 4 },{ "CMP", &cpu::CMP, &cpu::ZPX, 4 },{ "DEC", &cpu::DEC, &cpu::ZPX, 6 },{ "???", &cpu::DUM, &cpu::IMP, 6 },{ "CLD", &cpu::CLD, &cpu::IMP, 2 },{ "CMP", &cpu::CMP, &cpu::ABY, 4 },{ "NOP", &cpu::NOP, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 7 },{ "???", &cpu::NOP, &cpu::ABX, 4 },{ "CMP", &cpu::CMP, &cpu::ABX, 4 },{ "DEC", &cpu::DEC, &cpu::ABX, 7 },{ "???", &cpu::DUM, &cpu::IMP, 7 },
                            { "CPX", &cpu::CPX, &cpu::IMM, 2 },{ "SBC", &cpu::SBC, &cpu::INDX, 6 },{ "???", &cpu::NOP, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 8 },{ "CPX", &cpu::CPX, &cpu::ZP0, 3 },{ "SBC", &cpu::SBC, &cpu::ZP0, 3 },{ "INC", &cpu::INC, &cpu::ZP0, 5 },{ "???", &cpu::DUM, &cpu::IMP, 5 },{ "INX", &cpu::INX, &cpu::IMP, 2 },{ "SBC", &cpu::SBC, &cpu::IMM, 2 },{ "NOP", &cpu::NOP, &cpu::IMP, 2 },{ "???", &cpu::SBC, &cpu::IMP, 2 },{ "CPX", &cpu::CPX, &cpu::ABS, 4 },{ "SBC", &cpu::SBC, &cpu::ABS, 4 },{ "INC", &cpu::INC, &cpu::ABS, 6 },{ "???", &cpu::DUM, &cpu::IMP, 6 },
                            { "BEQ", &cpu::BEQ, &cpu::REL, 2 },{ "SBC", &cpu::SBC, &cpu::INDY, 5 },{ "???", &cpu::DUM, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 8 },{ "???", &cpu::NOP, &cpu::ZPX, 4 },{ "SBC", &cpu::SBC, &cpu::ZPX, 4 },{ "INC", &cpu::INC, &cpu::ZPX, 6 },{ "???", &cpu::DUM, &cpu::IMP, 6 },{ "SED", &cpu::SED, &cpu::IMP, 2 },{ "SBC", &cpu::SBC, &cpu::ABY, 4 },{ "NOP", &cpu::NOP, &cpu::IMP, 2 },{ "???", &cpu::DUM, &cpu::IMP, 7 },{ "???", &cpu::NOP, &cpu::ABX, 4 },{ "SBC", &cpu::SBC, &cpu::ABX, 4 },{ "INC", &cpu::INC, &cpu::ABX, 7 },{ "???", &cpu::DUM, &cpu::IMP, 7 },
	};
}


cpu::~cpu6502(){

}

//core functionality

uint8_t cpu::fetch(){
    if(!(lookup[opcode].addrmode == &cpu::IMP)){
        fetched = read(addr_absolute);
    }
    return fetched;
}

uint8_t cpu::read(uint16_t addr){
    return bus->cpuRead(addr, false);
}

void cpu::write(uint16_t addr, uint8_t dest){
    bus->cpuWrite(addr, dest);

    
}
void cpu::clock(){
    if(cycles == 0){
        printf("Progc: %s\n", std::format("{:#06x}",progc).c_str());
        opcode = read(progc++);
        printf("Opcode: %s\n", lookup[opcode].name.c_str());
        printf("Addr mode: %s\n", addrmodeName(lookup[opcode].addrmode).c_str());
        cycles = lookup[opcode].cycles; //Gets the # of cycles that given operation needs to execute. *BEST CASE SCENARIO*

        uint8_t addCycleAddr = (this->*lookup[opcode].addrmode)();
        uint8_t addCycleOp = (this->*lookup[opcode].operate)();
        cycles += (addCycleAddr & addCycleOp);
        cpuLog << cpuLog_clean() << std::endl;

    }
    printf("%s\n", cpuStatusLog().c_str());
    cycles--;
}

std::string cpu::addrmodeName(uint8_t(cpu6502::*addrmode)()) {
    if (addrmode == &cpu6502::IMP) return "IMP";
    if (addrmode == &cpu6502::IMM) return "IMM";
    if (addrmode == &cpu6502::ZP0) return "ZP0";
    if (addrmode == &cpu6502::ZPX) return "ZPX";
    if (addrmode == &cpu6502::ZPY) return "ZPY";
    if (addrmode == &cpu6502::REL) return "REL";
    if (addrmode == &cpu6502::ABS) return "ABS";
    if (addrmode == &cpu6502::ABX) return "ABX";
    if (addrmode == &cpu6502::ABY) return "ABY";
    if (addrmode == &cpu6502::IND) return "IND";
    if (addrmode == &cpu6502::INDX) return "INDX";
    if (addrmode == &cpu6502::INDY) return "INDY";
    return "UNKNOWN";
}

std::string cpu::cpuStatusLog(){
    return "Accum: " + std::format("{:#04x}", this->accum) + " | X: " + std::format("{:#04x}", this->x) + 
    " | Y: " + std::format("{:#04x}", this->y) + " | Stackp: " + std::format("{:#04x}", this->stackp) + " | Progc: " + std::format("{:#06x}", this->progc)
    + " | Status: C: " + std::to_string((this->status & 0x01)) + " Z: " + std::to_string(((this->status >> 1) & 0x01))
    + " I: " + std::to_string(((this->status >> 2) & 0x01)) + " B: " + std::to_string(((this->status >> 4) & 0x01))
    + " V: " + std::to_string(((this->status >> 6) & 0x01)) + " N: " + std::to_string(((this->status >> 7) & 0x01));
}

std::string cpu::cpuLog_clean(){
    return std::format("pc: {:04x} a: {:02x} x: {:02x} y: {:02x} p: {:02x} sp: {:02x}", this->progc, this->accum, this->x, this->y, this->status, this->stackp);
}


void cpu::setFlag(FLAGS6502 f, bool v){
    
    v ? status |= f : status &= ~f;
}

uint8_t cpu::getFlag(FLAGS6502 f){
    if((status & f) != 0){
        return 1;
    }
    return 0;
}

void cpu::reset(){
    addr_absolute = 0xFFFC; //the addresss that the game programmer sets to be the starting address of the software
    uint16_t low = read(addr_absolute); //get the lower half of the address from the memory location @ addr_absolute
    uint16_t hi = read(addr_absolute + 1); //get the higher half of the address from the memory location @ addr_absolute
    progc = (hi << 8) | low;
    accum = 0;
    x = 0;
    y = 0;
    stackp = 0xFD;  //the very first address on the stack of the cpu
    status = 0x00 | U;
    addr_absolute = 0x0000;
    addr_relatvie = 0x0000;
    fetched = 0x00;
    cycles = 8; //interrupts take a while so we will use 8 to represent how "long" it takes for the interrupt to execute
}

void cpu::interruptReq(){
    if(getFlag(I) == 0){
        write(0x0100 + stackp, (progc >> 8));//write the upper half of progc first then the bottom half as when you increment you will get the addr reversed.
        stackp--;
        write(0x0100 + stackp, progc);//write the bottom half of progc second
        stackp--;
        //Before pushing status edit the values in the status register to be post interrupt req
        setFlag(B, false);
        setFlag(U, true);
        setFlag(I, true);
        write(0x0100 + stackp, status);//write the status register to the stack (it is already 8 bits so we don't have to do any funny stuff)
        stackp--;
        addr_absolute = 0xfffe; //hard coded value that tells you where the new value of the progc should be
        uint16_t hi = read(addr_absolute);
        uint16_t lo = read(addr_absolute + 1);
        progc = (hi << 8) | lo;
        cycles = 7; //same reason why we did what we did in reset.
    }
}

void cpu::nonMskInter(){    //does the same as interruptReq but can't be stopped
    write(0x0100 + stackp, (progc >> 8));//write the upper half of progc first then the bottom half aBs when you increment you will get the addr reversed.
    stackp--;
    write(0x0100 + stackp, progc);//write the bottom half of progc second
    stackp--;
    //Before pushing status edit the values in the status register to be post interrupt req
    setFlag(B, false);
    setFlag(U, true);
    setFlag(I, true);
    write(0x0100 + stackp, status);//write the status register to the stack (it is already 8 bits so we don't have to do any funny stuff)
    stackp--;
    addr_absolute = 0xfffa; //hard coded value that tells you where the new value of the progc should be
    uint16_t hi = read(addr_absolute);
    uint16_t lo = read(addr_absolute + 1);
    progc = (hi << 8) | lo;
    cycles = 8; //same reason why we did what we did in reset.
}

// Implicit Addressing Mode
uint8_t cpu::IMP() {
    // Use the accumulator to store the fetched dat
    fetched = accum;
    return 0;
}

// Immediate Addressing Mode
uint8_t cpu::IMM() {
    // Directly load a byte from memory
    addr_absolute = progc++;
    return 0;
}

// Zero Page 0 Addressing Mode
uint8_t cpu::ZP0() {
    addr_absolute = read(progc++);
    addr_absolute &= 0x00FF;
    printf("zp0 addr_abs: %x\n", addr_absolute);
    return 0;
}

// Zero Page X Addressing Mode
uint8_t cpu::ZPX() {
    addr_absolute = (read(progc++) + x);
    addr_absolute &= 0x00FF;
    return 0;
}

// Zero Page Y Addressing Mode
uint8_t cpu::ZPY() {
    addr_absolute = (read(progc++) + y);
    addr_absolute &= 0x00FF;
    return 0;
}

// Absolute Addressing Mode
uint8_t cpu::ABS() {
    uint16_t lowByte = read(progc++);
    uint16_t hiByte = read(progc++);
    addr_absolute = (hiByte << 8) | lowByte;
    return 0;
}

//Absolute X Addressing Mode
uint8_t cpu::ABX(){
    uint16_t lowByte = read(progc++);
    uint16_t hiByte = read(progc++);
    addr_absolute = (hiByte << 8) | lowByte;
    addr_absolute += x;
    if((addr_absolute & 0xFF00) != (hiByte << 8)){  //If the increment addr is in a new page, tell the cpu to run an extra cycle.
        return 1;
    }
    else{
        return 0;
    }
}

uint8_t cpu::ABY(){
    uint16_t lowByte = read(progc++);
    uint16_t hiByte = read(progc++);
    addr_absolute = (hiByte << 8) | lowByte;
    addr_absolute += y;
    if((addr_absolute & 0xFF00) != (hiByte << 8)){  //If the increment addr is in a new page, tell the cpu to run an extra cycle.
        return 1;
    }
    else{
        return 0;
    }
}

uint8_t cpu::IND(){
    uint16_t ptrLow = read(progc++);
    uint16_t ptrHi = read(progc++);
    uint16_t totPtr = (ptrHi << 8) | ptrLow;
    if(ptrLow == 0x00FF){
        addr_absolute = (read(totPtr & 0xff00) << 8) | read (totPtr + 0); //we need to emulate a hardware bug (lol)
    }
    else{
        addr_absolute = (read(totPtr + 1) << 8 | read(totPtr + 0));
    }
    return 0;
}

uint8_t cpu::INDX(){
    uint16_t pshift = read(progc++);
    uint16_t low = read((uint16_t)(pshift + (uint16_t)x) & 0x00ff);
    uint16_t hi = read((uint16_t)(pshift + (uint16_t)x + 1) & 0x00ff);
    addr_absolute = (hi << 8) | low;
    return 0;
}

uint8_t cpu::INDY(){
    uint16_t pshift = read(progc++);
    uint16_t low = read(pshift & 0x00ff);
    uint16_t hi = read((pshift + 1) & 0x00ff);
    addr_absolute = (hi << 8) | low;
    addr_absolute += y;
    if((addr_absolute & 0xff00) != (hi << 8)){  //if incrementation crosses page boundary, tell the cpu to run one more cycle for the love of the game.
        return 1;
    }
    else{
        return 0;
    }

}

uint8_t cpu::INC(){
    fetch();
    write(addr_absolute, ++fetched);
    setFlag(Z, fetched == 0x00);
    setFlag(N, (fetched >> 7) != 0);
    return 0;    
}

uint8_t cpu::REL(){
    addr_relatvie = read(progc++);
    if(addr_relatvie & 0x80){
        addr_relatvie |= 0xff00; //sign extend if relative address is negative
    }
    return 0;
}

//Instructions

uint8_t cpu::DUM(){
    return 1;
}

uint8_t cpu::DEY(){
    y--;
    setFlag(Z, y == 0x00);
    setFlag(N, (y >> 7 != 0));
    return 0;
}

uint8_t cpu::DEX(){
    x--;
    setFlag(Z, x == 0x00);
    setFlag(N, (x >> 7 != 0));
    return 0;
}

uint8_t cpu::DEC(){
    fetch();
    write(addr_absolute, --fetched);
    setFlag(Z, fetched == 0);
    setFlag(N, (fetched >> 7) != 0);
    
    return 0;
}

uint8_t cpu::ADC(){
    fetch();
    uint16_t temp = (uint16_t)accum + (uint16_t)fetched + (uint16_t)getFlag(C); //a temporary 16 bit value to allow us to easily modify the carry flag and/or overflow flag
    setFlag(C, temp > 255); //value overflows if it is larger than 255 in the 16 bit variable
    setFlag(Z, ((temp & 0x00ff) == 0));
    setFlag(N, temp & 0x0080);
    setFlag(V, (~((uint16_t)accum ^ (uint16_t)fetched) & ((uint16_t)accum ^ (uint16_t)temp)) & 0x0080);
    accum = temp & 0x00ff;
    return 1;
}

uint8_t cpu::ASL(){
    fetch();
    setFlag(C, fetched >> 7);
    fetched = fetched << 1;
    setFlag(Z, fetched == 0x00);
    setFlag(N, fetched >> 7);
    if(lookup[opcode].addrmode == &cpu::IMP){
        accum = fetched;
    }
    else{
        write(addr_absolute, fetched);
    }
    return 0;
}

uint8_t cpu::SBC(){
    fetch();
    uint16_t negatedFetch = (uint16_t)fetched ^ 0x00ff;
    uint16_t temp = (uint16_t)accum + negatedFetch + (uint16_t)getFlag(C); //a temporary 16 bit value to allow us to easily modify the carry flag and/or overflow flag
    setFlag(C, temp > 255); //value overflows if it is larger than 255 in the 16 bit variable
    setFlag(Z, ((temp & 0x00ff) == 0));
    setFlag(N, temp & 0x0080);
    setFlag(V, (~((uint16_t)accum ^ (uint16_t)negatedFetch) & ((uint16_t)accum ^ (uint16_t)temp)) & 0x0080);
    accum = temp & 0x00ff;
    return 1;
}

uint8_t cpu::AND(){
    fetch();
    accum = accum & fetched;
    setFlag(Z, accum == 0x00);
    setFlag(N, accum & 0x80);
    return 1;
}

uint8_t cpu::BCC(){
    if(getFlag(C) == 0){
        cycles++;
        addr_absolute = progc + addr_relatvie;

        if((addr_absolute & 0xff00) != (progc & 0xff00)){
            cycles++;
        }
        progc = addr_absolute;
    }
    return 0;
}

// %%%%%%%%%%%%%%#%%%%%%%%%%%@%#*##%###%#%@@@@@@@@%%%%@@@@%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// @%%%%%%%%%%%%%%%%%%@@@@@%#**%%#*##%@@@@@@@@@@@@@%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// @@@%%%%%%@@@%%%%%%%@@@@#*#%#*##%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%%#%@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// @@@@%%%%%%@@%%@%%%@@@#%%%#*%%%%@@@@@@@@@@@@%%%%%%#**####%%%@@@%%%%%*#%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@
// @@@@@%%@@@@@@@@@%@@@%%@%%%@%@@@@@@@@%%%%##**++++++++=++++++*####%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// @@@@@@@@@@@@@@@@@@@%@%@@@@@@@%%@%#%%#**+++-----=-=======+++**###%#%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// @@@@@@@@@@@@@@@@@%%@%@@@@@@@%%%##**++---::------==-======++***####%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// @@@@@@@@@@@@@@@@@@@@%@@@@@@%#**+=----:::::::------=======+*++**###%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// @@@%%@@@@@@@@@@@@@@%@@@@@%*+=---:-:::::::::::-----=======++++**##%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// %%%%@@@@@@@@@@@@@@%@@@@@#+=-------::::::::::::----======++*++**##%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// %%%%%%%@@@@@@@@@@@@@@@@#+===-------::::::::::::-----=-==++*++**#%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// %%%%%@@@@@@@@@@@@@@@@@@++==--------:::::::::-:--------===+*++*###%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// %%%%%@@@@@@@@@@@@@@@@@*+===---------:::::::::---------===++++*##%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// %%@%@%%@@@@@@@@@@@@@@@*+===------:::::::::::::---------===+*****%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%%@
// %%%%%%@@%@@@@@@@@@@@@@*++==------::::::::::::----------==+****###%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%@@@
// %%%%%%@@@@@@@@@@@@@@@@*+===-------::::::::::::---------=++*****##%%@@@@@@@@@@%@@@@@@@@@@@@@@@@@@%%@%
// %%@%%%@@@@@@@@@@@@@@@@#++====-:--:::::::::::::::------==+++******#%@@@@@@@@@@@%@@@@@@@@@@@@@@@@@%%@@
// %%%%%@@@@@@@@@@@@@@@@@@*+===--::----::::::::::---------==+++***+**#%@@@@@@@@%@@#@@@@@@@@@@@@@@@@@@@@
// %%%%%%@@@@%@@@@@@@@@@@@*+==----:::::::::::::-----------===++*******#%%%@@@%%%%%*%@@@@@@@@@@@@@@@@@@%
// %%%%%%%%%%%@@@@@@@@@@@@#+=----:--::::::::---------=++++*********++**#%%%%%%%#####@@@@@@@@@@@@@@@%%%%
// %%%%%%@@@%%@@@@@@@@@@@@%+==----:----:----::--=++**##*##*####****+***##%#%%%%%#+*+@@@@@@@@@@@@@@@@%%%
// %%%%%%%%%%%%%@@@@@@@@@@%#%##**+====+--====+###%%%%######%%##***+*+***#%%%##%*+==+@@@%%@@@@@@@@@@@@@%
// %%%%%%%%%%%%%%@@@@@@@@@@%###%###%##*=-==*%%%%@@@%%%@@%##%%#**+=++***##%%@#*#+====@@@%%%%@@@@@@@@@@@@
// %%%%%%%%%%%%%%@@@@@@@@@@%#*%@%@@@@@%*--==*%%%#%##***+=**+==--==++*###%%@@%%#++--%@@%%%%%%@@@@@@@@@@@
// %%%%%%%%%%%%@@@@@@@@@@@@@#%#**+*+****----=*++*++==++++--------=+*##%##%@@%%**+-#@@@@@@@%%%@@@@@@@@@@
// %%@%%%%%%%%%@@@@@@@@@@@@@*+**+++**+++-:--=+=---===-----------=++*######%%%%*+=*@@@@@%%%%%@@@@@@@@@@@
// %%%%@@%%%%%%%@@@@@@@@==@@*=====-=====-::-+==--------------===++*##%%####%%%##@@@@@@%%%%%%%%%%%@@@@@@
// %%%%%%%%%%%%%%@@@@@*--=@%%+==========-::-+*=====--------===++**###%%#####%@@@@@@@@@%%%%%%%%%%%%@@@@@
// %%%%%%%%%%%%%@@@@%---=+%#%*=-------==-:--=+++++==--:::---=++*****#%#####%%@@@@@@@@@@@%%%%%%%%%%%%@@@
// %%#######%%%@@@@*---=*#%@%@*+==-----=-:--==++**+=---:---==+++*+**#%#####%%@@@@@@@@@%%%%%%%%%%%%%%@@@
// %%%%%%%%%%%@@@@-::-=*@@%%%%%*+=----==-:::=++++=+=--------=++++++*#######%@@@@@@@@@%%%%%%%%%%%%%%%%@@
// %%%%%@@@@%@%@-:::-=#@@@%%%%@#*+==-=-=-::-==++**#*-------==++++++*#####%%%@@@@@@@@%%%%%%%%%%%%%%%%%%%
// %%%%%%@@@@@=:::-=*#@@@@%%%%%@**+=======--=#%@@@@=:------===+++***####%%%@@@@@@@@%%%%%%%%%%##%%%%%%%@
// %%%%%%%@@@+-:::=+#@%%%%%@@@%%@**+=====*##%%@@%#=------====+++***##%%%%@@@@@@@@%%%%%%%%######%%%%%%%%
// %%#*#%%#*----::-=%%%%%%@@@@%%%%*++====+*%@@@%*+=-=====++*+++**#*##%%%@@@@@@@@@@#%%%%%%%##%%%%%%%%%%%
// --------::::::::-+%@@@@%%%%%%%%#*++++=++**##+=====++******++*#####%%@@@@@@@@@@%##%%%%%%%%%%##%%%%%%%
// =-----:-------:::=*@%%%%%%%%%%%%*++**++====---=++*#%@@%%%*=+*#%%##%@@@@@@@@@@@%###%%%%%%%##%%%%%%%%%
// -:::--=-::::=+-:::=#%%%%%%%%%%%%%*++#%###%#%%%@@@@@%**++++=+*#%###%@@@@@@@@@@@####%@%%%%%%%%%%%%%%%%
// ---:--=+-::-=++:::-+%%%%%%%%%%##%%*+=**#%%%%%%*=-==*#+++++++*#%%#%@@@@@@@@@@%##%%#%@@%%%%%%%%%%%%%%%
// **%--:--+::-==*:::-++%%%%%%%%#####%**+****+==--=+*%%****+==+*%%##%@@@@@@@@@###%%%%%@@@@@%%%%%%%%%%%%
// %@@*-::-==::-=%-::-+*%%%%%%%%%#%##%%#*+**#%%@@@@@@%***+==+++*%%%%@@@@@@@@@####%%%%@@@@@@@@@%%%%%%%%%
// @@@@@--=+#:-==%=::-=#%%%%%%%%#%%%%%@%*+++*##%%#%#*==+====+**%%@@@@@@@@@@#####%%#%%@@@@@@@%%@@%%%%%%%
// @@@@%==+##:-=+%+----*%%%%%##%%@@@@@@@%#*+==+============+*#%@@@@@@@@@@%#########%@@@@@@@@%%%%%%%%%%%
// @@%%@+=*#+--+*@@=--=*%##*++==+@@@@@@@@@#*+=======-==+=+**%@@@@@@@@@@@%#########%@@@@@@@@%%%%%%##%%%%
// %%%###**++-=+%@@%@@@@%%#*++==+%@@@@@@@@%##*+=+=++******%@@@@@@@@@@@@%%########%%@@@@@@@@%%%%%%%#%##%
// **=++***%--++%#%%%%%%%%###**#%%%@@@@@@@%**%%%##%#%%@@@@@@@@@@@@@@@@%%##*#***#%%%@@@@@@@@%%%%%%%%%%%#
// +--===+**===+%%%%%%%%%%##%%%@%%%%%%@@@@@++*##%@@@@@@@@@@@@@@@@@@@@%##*******#%%%%@@@@@@@%%%%%%%%%%%#
// =---===+*#**###%%%%%#%%%@%%%%%%#%%%%@@@@*+++**#%@@@@@@@@@@@@@@@@%%##****+**#%%%%%%@@@@@@%%%##%%%####
// ---==+=+**##%##%#%%%@@@@%%%%%%%%%%%%@@%@++++++**#%%@@@@@@@@@@%%%##*****+++*%%%%%%%%%@@@%%%###%%%%#%#
// =-====++**#####%%@@@@@@%%%%%%%%%%%%%%%%@===++==+**#@%%%%##%####******+++==%%%%%%%%%%%%@%%%##%%@@####
// +==++++**#%%%%@@@@@@@@@@@@@@@@@@@%%%%%%@======-==#%%#%*++++++++*++++==---%%%%%%%%%%%%%@#%@@%%%%%##%#
// *++***#%%%@@@@@@@@@@@@@@@@@@@@@%%%%%%%%#===----@+#=++%#=---=+=====-----:#%%%%%%%%%%%%%%%%%%%%%%#####
// #%#%%%@@@@@@@@@@@@@@@@@@@@@@@@%%%%%%%#%+=----=@*-:--+%#%%---------:-::-+#%%%%%%%%%%%%%%%#%##########
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@%%%%%%%###-=---=@@#==-*#@@%%@=-::--::::::-#%%%%%%%%%%%%##%%%%%%########
// @@@@@@@@@@@@@@@@@@@@@@@@@@@%%%%%%%%%#%+--:-=@:-*##=#@@@%@@-*-:::::::--##%%%%%%%%%#%%##%%%%%%%#%###%#
 
uint8_t cpu::BCS(){ 
    if(getFlag(C) == 1){
        cycles++;
        addr_absolute = progc + addr_relatvie;

        if((addr_absolute & 0xff00) != (progc & 0xff00)){
            cycles++;
        }
        progc = addr_absolute;
    }
    return 0;
}

uint8_t cpu::BEQ(){
    if(getFlag(Z) == 1){
        cycles++;
        addr_absolute = progc + addr_relatvie;

        if((addr_absolute & 0xff00) != (progc & 0xff00)){
            cycles++;
        }
        progc = addr_absolute;
    }
    return 0;
}

uint8_t cpu::BMI(){
    if(getFlag(N) == 1){
        cycles++;
        addr_absolute = progc + addr_relatvie;

        if((addr_absolute & 0xff00) != (progc & 0xff00)){
            cycles++;
        }
        progc = addr_absolute;
    }
    return 0;
}

uint8_t cpu::BNE(){
    if(getFlag(Z) == 0){
        cycles++;
        addr_absolute = progc + addr_relatvie;

        if((addr_absolute & 0xff00) != (progc & 0xff00)){
            cycles++;
        }
        progc = addr_absolute;
    }
    return 0;
}

uint8_t cpu::BPL(){
    if(getFlag(N) == 0){
        cycles++;
        addr_absolute = progc + addr_relatvie;

        if((addr_absolute & 0xff00) != (progc & 0xff00)){
            cycles++;
        }
        progc = addr_absolute;
    }
    return 0;
}

uint8_t cpu::BVC(){
    if(getFlag(V) == 0){
        cycles++;
        addr_absolute = progc + addr_relatvie;

        if((addr_absolute & 0xff00) != (progc & 0xff00)){
            cycles++;
        }
        progc = addr_absolute;
    }
    return 0;
}

uint8_t cpu::BVS(){
    if(getFlag(V) == 1){
        cycles++;
        addr_absolute = progc + addr_relatvie;

        if((addr_absolute & 0xff00) != (progc & 0xff00)){
            cycles++;
        }
        progc = addr_absolute;
    }
    return 0;
}

uint8_t cpu::CLC(){
    setFlag(C, false);
    return 0;
}

uint8_t cpu::CLD(){
    setFlag(D, false);
    return 0;
}

uint8_t cpu::CLI(){
    setFlag(I, false);
    return 0;
}

uint8_t cpu::CLV(){
    setFlag(V, false);
    return 0;
}

uint8_t cpu::CMP(){
    fetch();
    setFlag(C, accum >= fetched);
    setFlag(Z, accum == fetched);
    setFlag(N, (accum - fetched) >> 7 );
    return 1;
}

uint8_t cpu::ORA(){
    fetch();
    accum = accum | fetched;
    setFlag(Z, accum == 0x00);
    setFlag(N, (accum & 0b10000000) != 0);
    return 1;
}

uint8_t cpu::NOP(){ //No operation instruction
    return 1;
}

uint8_t cpu::PHA(){ //Push accumulator onto the stack.
    bus->cpuWrite(0x0100 + stackp, accum);
    stackp--;
    return 0;
}


uint8_t cpu::JMP(){
    progc = addr_absolute;
    return 0;
}


uint8_t cpu::JSR(){
    progc--;
    write(stackp + 0x0100, (progc >> 8));   //save upper half of the progc
    stackp--;
    write(stackp + 0x0100, progc & 0x00ff); //save lower half of the progc
    stackp--;
    progc = addr_absolute;
    return 0;
}

uint8_t cpu::PHP(){
    uint8_t data = status | 0b00110000;
    write(stackp + 0x0100,data);
    stackp--;
    return 0;
}

uint8_t cpu::PLA(){
    stackp++;
    accum = read(0x0100 + stackp);
    setFlag(Z, accum == 0);
    setFlag(N,accum >> 7);
    return 0;
}

uint8_t cpu::PLP(){
    stackp++;
    uint8_t temp = read(0x0100 + stackp); 
    temp &= 0b11001111;
    status = temp | (status & 0b00110000);
    return 0;
}

uint8_t cpu::CPY(){
    fetch();
    setFlag(C, y >= fetched);   //These are kinda lazy implementations instead of doing the subtraction like how the cpu would actuall do it
    setFlag(Z, y == fetched);
    setFlag(N, (y - fetched) >> 7);
    return 0; 
}

uint8_t cpu::CPX(){
    fetch();
    setFlag(C, x >= fetched);
    setFlag(Z, x == fetched);
    setFlag(N, (x - fetched) >> 7);
    return 0;
}

uint8_t cpu::ROR(){
    fetch();
    uint16_t temp = (uint16_t)(getFlag(C) << 7) | (fetched >> 1); // prepare the rotated value
    setFlag(C,fetched & 0x01);
    setFlag(Z, (temp & 0x00ff) == 0x00);
    setFlag(N, temp & 0x0080);   //Checks the value of the msb to check sign
    if(lookup[opcode].addrmode == &cpu::IMP){   //if the addressing mode is applied, act on the accumulator instead
        accum = temp & 0x00ff;
    }
    else{
        write(addr_absolute, temp & 0x00ff);
    }
    return 0;
}

uint8_t cpu::ROL(){
    fetch();
    setFlag(C, fetched >> 7);
    fetched = fetched << 1;
    if(getFlag(C) == 1){fetched |= 0x01;}
    setFlag(Z, fetched == 0);
    setFlag(N, fetched >> 7);
    if(lookup[opcode].addrmode == &cpu::IMP){
        accum = fetched;
    }
    else{
        write(addr_absolute, fetched);
    }
    return 0;
}

uint8_t cpu::EOR(){ //exclusive or
    fetch();
    accum = accum ^ fetched;
    setFlag(Z, accum == 0x00);
    setFlag(N, (accum >> 7 != 0));

    return 1;
}

uint8_t cpu::LSR(){ //Logical Shift Right implementation
    fetch();
    setFlag(C, fetched & 0x0001);
    fetched = fetched >> 1;
    setFlag(Z, fetched == 0);
    setFlag(N, 0);
    if(lookup[opcode].addrmode == &cpu::IMP){
        accum = fetched;
    }
    else{
        write(addr_absolute, fetched);
    }
    return 0;
}

uint8_t cpu::LDY(){
    fetch();
    y = fetched;
    setFlag(Z, y == 0x00);
    setFlag(N, (y >> 7) != 0);
    return 1;
}

uint8_t cpu::LDA(){
    fetch();
    accum = fetched;
    setFlag(Z, accum == 0);
    setFlag(N, (accum >> 7) != 0);
    return 1;
}

uint8_t cpu::LDX(){
    fetch();
    x = fetched;
    setFlag(Z, x == 0);
    setFlag(N, (x >> 7) != 0);
    return 1;
}

uint8_t cpu::BRK(){ //Break instruction implementation
    progc++;
    setFlag(I, true);
    return 0;
}

uint8_t cpu::BIT(){
    fetch();
    uint8_t temp = accum & fetched;
    setFlag(Z, temp == 0x00);
    setFlag(N, (fetched >> 7 != 0)); 
    setFlag(V, ((fetched >> 6) & 0b000001) != 0);
    return 0;
}

uint8_t cpu::RTI(){
    //restore status register
    stackp++;
    uint8_t temp = read(0x0100 + stackp); 
    temp &= 0b11001111;
    status = temp | (status & 0b00110000);
    //restore progc
    stackp++;
    progc = (uint16_t)read(0x0100 + stackp); //stack lives on the first page/ get the low byte of the address.
    stackp++;
    progc |= (uint16_t)read(0x0100 + stackp) << 8; //get the high byte of the adddress
    return 0;
}

uint8_t cpu::INX(){
    x++;
    setFlag(Z, x == 0);
    setFlag(N, x >> 7);
    return 0;
}

uint8_t cpu::INY(){
    y++;
    setFlag(Z, y == 0);
    setFlag(N, y >> 7);
    return 0;
}

/*
    Since the bus is only 8-bits wide you are only able to read the hi or lo seperately.
*/

uint8_t cpu::RTS(){
    stackp++;
    progc = (uint16_t)read(0x0100 + stackp); //stack lives on the first page/ get the low byte of the address.
    stackp++;
    progc |= (uint16_t)read(0x0100 + stackp) << 8; //get the high byte of the adddress
    progc++;
    return 0;
}

uint8_t cpu::SEC(){ //Set carry flag to high
    setFlag(C, 1);
    return 0;
}

uint8_t cpu::SED(){ //Set decimal flag to high
    setFlag(D, true);
    return 0;
}

uint8_t cpu::SEI(){ //Set interrupt disable status to high
    setFlag(I,true);
    return 0;
}

uint8_t cpu::STA(){ //Store accumulator in Memory
    bus->cpuWrite(addr_absolute, accum);
    return 0;
}

uint8_t cpu::STX(){ //Store register X @ a memory location  
    bus->cpuWrite(addr_absolute, x);
    printf("Data @ addr: %x\n", this->bus->cpuMem[addr_absolute]);
    return 0;
}

uint8_t cpu::STY(){ //Store register Y @ a memory location
    bus->cpuWrite(addr_absolute, y);
    return 0;
}

uint8_t cpu::TAX(){ //Transfer accumulator to X
    x = accum;
    setFlag(Z, x == 0);
    setFlag(N, x >> 7);
    return 0;
}

uint8_t cpu::TAY(){ //Transfer Accumulator to Y
    y = accum;
    setFlag(Z, y == 0);
    setFlag(N, y >> 7);
    return 0;
}

uint8_t cpu::TSX(){ //Transfer stack pointer register to X
    x = stackp;
    setFlag(Z, x == 0);
    setFlag(N, x >> 7);
    return 0;
}

uint8_t cpu::TXA(){ //Transfer register X to accumulator
    accum = x;
    setFlag(Z, accum == 0);
    setFlag(N, accum >> 7);
    return 0;
}

uint8_t cpu::TXS(){ //Transfer X to Stack pointer register
    stackp = x;
    return 0;
}

uint8_t cpu::TYA(){ //Transfer Index Y to Accumulator
    accum = y;
    setFlag(Z, accum == 0);
    setFlag(N, accum >> 7);
    return 0;
}