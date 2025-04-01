#include "cpu6502.h"
#include "Bus.h"
using cpu = cpu6502; //Creates a temporary naming variable to make the table more simple.

cpu::cpu6502(){
    this->lookup = {
                            //Horizontal (LSB) 0x0_
    /*Vertical 0x_0 (MSB)*/{"BRK", &cpu::BRK, &cpu::IMP, 7}, {"ORA", &cpu::ORA, &cpu::INX, 6}, {"ORA", &cpu::ORA, &cpu::ZP0, 3}, {"ASL", &cpu::ASL, &cpu::ZP0, 5}, {"Dummy", &cpu::DUM, &cpu::IMM, 5}, {"PHP", &cpu::PHP, &cpu::IMP, 3}, {"ORA", &cpu::ORA, &cpu::IMM, 2}, {"ASL", &cpu::ASL, &cpu::IMP, 2}, {"Dummy", &cpu::DUM, &cpu::IMM, 2}, {"Dummy", &cpu::DUM, &cpu::IMM, 2}, {"ORA", &cpu::ORA, &cpu::ABS, 4}, {"ASL", &cpu::ASL, &cpu::ABS, 6}, {"Dummy", &cpu::DUM, &cpu::IMM, 6}
    };
}

cpu::~cpu6502(){

}

uint8_t cpu::fetch(){

}

uint8_t cpu::read(uint16_t addr){
    return bus->read(addr, false);
}

void cpu::write(uint16_t addr, uint8_t dest){
    bus->write(addr, dest);
}
void cpu::clock(){
    if(cycles == 0){
        opcode = read(progc);
        progc++;
        cycles = lookup[opcode].cycles; //Gets the # of cycles that given operation needs to execute. *BEST CASE SCENARIO*

        (this->*lookup[opcode].addrmode)();
        (this->*lookup[opcode].operate)();
    }
    
    cycles--;
}
/*
* All the addressing modes are a WOP. 
* Make sure to review them when you 1000% understand what is going on.
*/
// Implicit Addressing Mode
uint8_t cpu::IMP() {
    // Use the accumulator to store the fetched dat
    fetched = accum;
    return 0;
}

// Immediate Addressing Mode
uint8_t cpu::IMM() {
    // Directly load a byte from memory
    uint16_t addr = 0x0000; // Hardcoded address for demo purposes
    uint8_t data = bus->read(addr, true); // Read as an immediate value
    return data;
}

// Zero Page 0 Addressing Mode
uint8_t cpu::ZP0() {
    // Use the X and Y registers to access memory locations in the zero page
    uint16_t addr = x * 16; // Zero page location
    return bus->read(addr, true); // Read as a byte from the zero page
}

// Zero Page X Addressing Mode
uint8_t cpu::ZPX() {
    // Use the X register to access memory locations in the zero page
    uint16_t addr = x * 16; // Zero page location
    return bus->read(addr, true); // Read as a byte from the zero page
}

// Zero Page Y Addressing Mode
uint8_t cpu::ZPY() {
    // Use the X register to access memory locations in the zero page
    uint16_t addr = y * 16; // Zero page location
    return bus->read(addr, true); // Read as a byte from the zero page
}

// Absolute Addressing Mode
uint8_t cpu::ABS() {
    // Use an absolute address to access memory
    uint16_t addr = addr_absolute;
    return bus->read(addr, true); // Read as a byte from memory
}

// Relative Addressing Mode
uint8_t cpu::REL() {
    // Use a relative address to access memory
    uint16_t addr = addr_relatvie;
    return bus->read(addr, true); // Read as a byte from memory
}


uint8_t cpu::NOP(){ //No operation instruction
    return 1;
}

uint8_t cpu::BPL(){ //Branch if positive insruction implementation
    if(getFlag(FLAGS6502::N) == 0){
        cycles++;
        addr_absolute = progc++;
        if((addr_absolute & 0xFF00) != (progc & 0xFF00)){
            cycles++;
        }
        progc = addr_absolute;
    }
    return 0;
}

uint8_t cpu::PHA(){ //Push accumulator onto the stack.
    bus->write(0x0100 + stackp, accum);
    stackp--;
}

uint8_t cpu::JMP(){
    progc = addr_absolute;
}

uint8_t cpu::PHP(){
    uint8_t data;
    data = getFlag(N) | getFlag(V) | (1 << 5) 
    | (1 << 4) | getFlag(D) | getFlag(I) 
    | getFlag(Z) | getFlag(C);
    write(stackp--,data);
}

uint8_t cpu::PLA(){
    stackp++;
    accum = bus->read(stackp, false);
    if(accum == 0x00){
        setFlag(Z,0);
    }
    setFlag(N,accum >> 7);
}

uint8_t cpu::PLP(){
    stackp++;
    uint8_t temp = read(stackp);
    setFlag(N, (temp >> 7));
    setFlag(V, ((temp >> 6) & 0x01));
    setFlag(D, ((temp >> 3) & 0x01));
    setFlag(I, ((temp >> 2) & 0x01));
    setFlag(Z, ((temp >> 1) & 0x01));
    setFlag(C, (temp & 0x01));
}

uint8_t cpu::ROR(){
    fetch();
    uint8_t temp = fetched & 0x01;
    temp = temp << 7;
    fetched = fetched >> 1;
    fetched = fetched & temp;
}

uint8_t cpu::ROL(){
    fetch();
    uint8_t temp = fetched & 0x80;
    temp = temp >> 7;
    fetched = fetched << 1;
    fetched = fetched & temp;
}

uint8_t cpu::LSR(){ //Logical Shift Right implementation
    fetch();
    setFlag(C, fetched & 0x0001);
    fetched = fetched >> 1;
    if(fetched == 0x00){
        setFlag(Z, true);
    }
    if(fetched & 0x80){
        setFlag(N, true);
    }
}

uint8_t cpu::BRK(){ //Break instruction implementation
    progc++;
    setFlag(I, true);
}

uint8_t cpu::RTI(){
    uint8_t temp = read(stackp--);
    setFlag(N, (temp >> 7));
    setFlag(V, ((temp >> 6) & 0x01));
    setFlag(D, ((temp >> 3) & 0x01));
    setFlag(I, ((temp >> 2) & 0x01));
    setFlag(Z, ((temp >> 1) & 0x01));
    setFlag(C, (temp & 0x01));
    temp = read(stackp--);
}

uint8_t cpu::RTS(){
    uint8_t popped = read(stackp);
    stackp -= 0x1;
    popped++;
    progc = popped;
    return 0;
}

uint8_t cpu::SBC(){//Subtract memory value from accumulator with Borrow (aka carry)
    fetch();
    accum = accum - fetched - (uint16_t)~getFlag(C);
}

uint8_t cpu::SEC(){ //Set carry flag to high
    setFlag(C, true);
}

uint8_t cpu::SED(){ //Set decimal flag to high
    setFlag(D, true);
}

uint8_t cpu::SEI(){ //Set interrupt disable status to high
    setFlag(I,true);
}

uint8_t cpu::STA(){ //Store accumulator in Memory
    bus->write(addr_absolute, accum);
}

uint8_t cpu::STX(){ //Store register X @ a memory location
    bus->write(addr_absolute, x);
}

uint8_t cpu::STY(){ //Store register Y @ a memory location
    bus->write(addr_absolute, y);
}

uint8_t cpu::TAX(){ //Transfer accumulator to X
    x = accum;
}

uint8_t cpu::TAY(){ //Transfer Accumulator to Y
    y = accum;
}

uint8_t cpu::TSX(){ //Transfer stack pointer register to X
    x = stackp;
}

uint8_t cpu::TXA(){ //Transfer register X to accumulator
    accum = x;
}

uint8_t cpu::TXS(){ //Transfer X to Stack pointer register
    stackp = x;
}

uint8_t cpu::TXA(){ //Transfer Index X to Accumulator
    accum = x;
}

uint8_t cpu::TYA(){ //Transfer Index Y to Accumulator
    accum = y;
}
