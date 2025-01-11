#include "cpu6502.h"
#include "Bus.h"

olc6502::olc6502(){
    using olc = olc6502; //Creates a temporary naming variable to make the table more simple.
}

olc6502::~olc6502(){

}



uint8_t olc6502::read(uint16_t addr){
    return bus->read(addr, false);
}

void olc6502::write(uint16_t addr, uint8_t dest){
    bus->write(addr, dest);
}

void olc6502::clock(){
    if(cycles == 0){
        opcode = read(progc);
        progc++;
        cycles = lookup[opcode].cycles; //Gets the # of cycles that given operation needs to execute. *BEST CASE SCENARIO*
    }
    //Function not finished.
}