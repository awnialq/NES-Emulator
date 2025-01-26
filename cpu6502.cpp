#include "cpu6502.h"
#include "Bus.h"

cpu6502::cpu6502(){
    using cpu = cpu6502; //Creates a temporary naming variable to make the table more simple.
    this->lookup = {{}, {}};
}

cpu6502::~cpu6502(){

}



uint8_t cpu6502::read(uint16_t addr){
    return bus->read(addr, false);
}

void cpu6502::write(uint16_t addr, uint8_t dest){
    bus->write(addr, dest);
}
void cpu6502::clock(){
    if(cycles == 0){
        opcode = read(progc);
        progc++;
        cycles = lookup[opcode].cycles; //Gets the # of cycles that given operation needs to execute. *BEST CASE SCENARIO*
    }
    //Function not finished.
}

uint8_t cpu6502::BPL(){ //Branch if positive insruction implementation
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

uint8_t cpu6502::BRK(){ //Break instruction implementation
    progc++;
    setFlag(I, true);
    
}