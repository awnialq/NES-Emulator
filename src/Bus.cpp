#include "Bus.h"
#include "cartridge.h"
#include <cstdint>
#include <cstdio>
#include <stdexcept>


 

Bus::Bus(char *romPath){
    for(auto &i : cpuMem){i = 0x00;}
    cart = new cartridge(romPath);
    if(cart->initCart() == 1){
        printf("Cart succesfully loaded\n");
        ppu.connectCart(cart);
    }
    else{
        throw std::runtime_error("Cartridge could not be loaded");
    }
}

Bus::~Bus(){
    delete cart;
}

void Bus::cpuWrite(uint16_t addr, uint8_t data){
    if(cart->cpuWrite(addr, data)){
        return;
    }

    if(addr >= 0x0000 && addr <= 0x1FFF){
        cpuMem[addr & 0x1FFF] = data;
    }
    else if(addr >= 0x2000 && addr <= 0x3FFF){
        ppu.cpuWrite(addr & 0x0007, data);  //implement addr mirroring
    }
    else if(addr == 0x4016){
        controllerStrobe = (data & 0x01) != 0;
        if(controllerStrobe){
            controllerState[0] = controller[0];
            controllerState[1] = controller[1];
        }
        else{
            controllerState[0] = controller[0];
            controllerState[1] = controller[1];
        }
    }
}

uint8_t Bus::cpuRead(uint16_t addr, bool readOnly){
    uint8_t data = 0x00;

    if(cart->cpuRead(addr, data)){
        return data;
    }

    if(addr >= 0x0000 && addr <= 0x1FFF){
        data = cpuMem[addr & 0x07FF];  //Implemented the addr mirroring by anding with 2kb
    }
    else if(addr >= 0x2000 && addr <= 0x3FFF){
        data = ppu.cpuRead(addr & 0x0007, readOnly);
    }
    else if(addr == 0x4016 || addr == 0x4017){
        uint8_t port = static_cast<uint8_t>(addr & 0x0001);
        if(controllerStrobe){
            data = (controller[port] & 0x80) > 0;
        }
        else{
            data = (controllerState[port] & 0x80) > 0;
            controllerState[port] <<= 1;
        }
    }
    return data;
}

void Bus::clock(){
    ppu.clock();
    clockCntr++;
}

void Bus::setControllerState(uint8_t port, uint8_t state){
    if(port < 2){
        controller[port] = state;
    }
}

