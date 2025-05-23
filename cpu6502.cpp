#include "cpu6502.h"
#include "Bus.h"
using cpu = cpu6502; //Creates a temporary naming variable to make the table more simple.

cpu::cpu6502(){
    this->lookup = {
                            //Horizontal (LSB) 0x0_
    /*Vertical 0x_0 (MSB)*/{"BRK", &cpu::BRK, &cpu::IMP, 7}, {"ORA", &cpu::ORA, &cpu::INX, 6}, {"ORA", &cpu::ORA, &cpu::ZP0, 3}, {"ASL", &cpu::ASL, &cpu::ZP0, 5}, {"Dummy", &cpu::DUM, &cpu::IMM, 5}, {"PHP", &cpu::PHP, &cpu::IMP, 3}, {"ORA", &cpu::ORA, &cpu::IMM, 2}, {"ASL", &cpu::ASL, &cpu::IMP, 2}, {"Dummy", &cpu::DUM, &cpu::IMM, 2}, {"Dummy", &cpu::DUM, &cpu::IMM, 2}, {"ORA", &cpu::ORA, &cpu::ABS, 4}, {"ASL", &cpu::ASL, &cpu::ABS, 6}, {"Dummy", &cpu::DUM, &cpu::IMM, 6},
                            {"BPL", &cpu::BPL, &cpu::REL, 2}, {"ORA", &cpu::ORA, &cpu::INY, 5}, {}
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
    bus->cpuRead(addr, dest);
}
void cpu::clock(){
    if(cycles == 0){
        opcode = read(progc++);
        cycles = lookup[opcode].cycles; //Gets the # of cycles that given operation needs to execute. *BEST CASE SCENARIO*

        uint8_t addCycleAddr = (this->*lookup[opcode].addrmode)();
        uint8_t addCycleOp = (this->*lookup[opcode].operate)();
        cycles += (addCycleAddr & addCycleOp);
    }
    
    cycles--;
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

uint8_t cpu::INX(){
    uint16_t pshift = read(progc++);
    uint16_t low = read((uint16_t)(pshift + (uint16_t)x) & 0x00ff);
    uint16_t hi = read((uint16_t)(pshift + (uint16_t)x + 1) * 0x00ff);
    addr_absolute = (hi << 8) | low;
    return 0;
}

uint8_t cpu::INY(){
    uint16_t pshift = read(progc++);
    uint16_t low = read(pshift * 0x00ff);
    uint16_t hi = read(pshift + 0x00ff);
    addr_absolute = (hi << 8) | low;
    addr_absolute += y;
    if((addr_absolute & 0xff00) != (hi << 8)){  //if incrementation crosses page boundary, tell the cpu to run one more cycle for the love of the game.
        return 1;
    }
    else{
        return 0;
    }

}

uint8_t cpu::REL(){
    addr_relatvie = read(progc++);
    if(addr_relatvie & 0x80){
        addr_relatvie |= 0xff00; //sign extend if relative address is negative
    }
    return 0;
}

//Instructions

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

uint8_t cpu::BCS(){
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

uint8_t cpu::BCS(){ //Better call saul. *badum tshhhhhhhhhhhhhhh*
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

uint8_t cpu::BNE(){
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

uint8_t cpu::BVC(){
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

uint8_t cpu::PHP(){
    uint8_t data;
    data = status | 0b00110000;
    setFlag(B, 0);
    setFlag(U, 0);
    write(stackp + 0x0100,data);
    stackp--;
    return 0;
}

uint8_t cpu::PLA(){
    stackp++;
    accum = read(0x0100 + stackp);
    if(accum == 0x00){
        setFlag(Z,0);
    }
    setFlag(N,accum >> 7);
    return 0;
}

uint8_t cpu::PLP(){
    stackp++;
    uint8_t temp = read(stackp);
    status = temp;
    setFlag(U, 1);
    return 0;
}

uint8_t cpu::ROR(){
    fetch();
    uint16_t temp = (uint16_t)(getFlag(C) << 7) | (fetched >> 1); // prepare the rotated value
    setFlag(C,fetched & 0x01);
    setFlag(Z, (temp & 0x00ff) == 0x00);
    setFlag(N, temp & 0x0080);   //Checks the value of the msb to check sign
    if(lookup[opcode].addrmode == &cpu6502::IMP){   //if the addressing mode is applied, act on the accumulator instead
        accum = temp & 0x00ff;
    }
    else{
        write(addr_absolute, temp & 0x00ff);
    }
    return 0;
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
    //restore status register
    stackp++;
    status = read(0x0100 + stackp);
    status &= ~B;
    status &= ~U;
    //restore progc
    stackp++;
    progc = read(0x0100 + stackp);
    stackp++;
    progc |= (read(0x0100 + stackp) << 8);
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
    return 0;
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
    bus->cpuWrite(addr_absolute, accum);
}

uint8_t cpu::STX(){ //Store register X @ a memory location
    bus->cpuWrite(addr_absolute, x);
}

uint8_t cpu::STY(){ //Store register Y @ a memory location
    bus->cpuWrite(addr_absolute, y);
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