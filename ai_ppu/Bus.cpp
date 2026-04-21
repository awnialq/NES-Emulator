#include "Bus.h"
#include "cartridge.h"
#include "ppu2C02.h"
#include "Mapper000.h"
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <memory>

Bus::Bus(char *romPath){
    // Initialize CPU memory
    for(auto &i : cpuMem){
        i = 0x00;
    }
    
    // Load cartridge
    cart = new cartridge(romPath);
    if(cart->initCart() == 1){
        printf("Cart successfully loaded\n");
        printf("Mapper ID: %u\n", cart->mapperID);
        printf("PRG Banks: %u\n", cart->header[4]);
        printf("CHR Banks: %u\n", cart->header[5]);
    }
    else{
        throw std::runtime_error("Cartridge could not be loaded");
    }
    
    // Initialize PPU with CHR memory
    if(cart->chrMem.size() > 0){
        ppu.init(cart->chrMem.data(), cart->chrMem.size());
    }
    
    // Create mapper based on mapper ID
    switch(cart->mapperID) {
        case 0:
            mapper = std::make_unique<Mapper000>(cart->header[4], cart->header[5]);
            break;
        default:
            // Default to Mapper000 for now
            mapper = std::make_unique<Mapper000>(cart->header[4], cart->header[5]);
            break;
    }
}

Bus::~Bus(){
    delete cart;
}

// ============================================================================
// CPU READ
// ============================================================================

uint8_t Bus::cpuRead(uint16_t addr, bool readOnly){
    uint8_t data = 0xFF;  // Default return value
    
    // CPU RAM: 0x0000-0x1FFF (mirrored every 2KB)
    if(addr >= 0x0000 && addr <= 0x1FFF){
        data = cpuMem[addr & 0x07FF];  // 2KB mirror
        return data;
    }
    
    // PPU Registers: 0x2000-0x3FFF
    if(addr >= 0x2000 && addr <= 0x3FFF){
        data = ppu.cpuRead(addr & 0x0007, readOnly);
        return data;
    }
    
    // Cartridge PRG ROM: 0x8000-0xFFFF
    if(addr >= 0x8000 && addr <= 0xFFFF){
        data = readFromCartridge(addr, readOnly);
        return data;
    }
    
    // Return 0xFF for unmapped addresses
    return 0xFF;
}

// ============================================================================
// CPU WRITE
// ============================================================================

void Bus::cpuWrite(uint16_t addr, uint8_t data){
    // CPU RAM: 0x0000-0x1FFF (mirrored)
    if(addr >= 0x0000 && addr <= 0x1FFF){
        cpuMem[addr & 0x07FF] = data;
        return;
    }
    
    // PPU Registers: 0x2000-0x3FFF
    if(addr >= 0x2000 && addr <= 0x3FFF){
        ppu.cpuWrite(addr & 0x0007, data);
        return;
    }
    
    // Cartridge PRG ROM: 0x8000-0xFFFF (usually write-protected)
    if(addr >= 0x8000 && addr <= 0xFFFF){
        writeToCartridge(addr, data);
        return;
    }
    
    // Write to unmapped address (no effect)
}

// ============================================================================
// PPU MEMORY ACCESS
// ============================================================================

void Bus::ppuWrite(uint16_t addr, uint8_t data){
    // Route PPU memory access through PPU
    ppu.ppuWrite(addr, data);
}

uint8_t Bus::ppuRead(uint16_t addr, bool readOnly){
    // Route PPU memory access through PPU
    return ppu.ppuRead(addr, readOnly);
}

// ============================================================================
// CARTRIDGE MEMORY ROUTING (THROUGH MAPPER)
// ============================================================================

uint8_t Bus::readFromCartridge(uint16_t addr, bool readOnly){
    (void)readOnly;
    uint32_t mappedAddr = addr;
    
    // Let mapper handle address translation
    if(mapper && mapper->modCpuRead(addr, mappedAddr)){
        // Read from PRG ROM
        if(cart->prgMem.size() > 0){
            return cart->prgMem[mappedAddr % cart->prgMem.size()];
        }
    }
    
    return 0xFF;  // Unmapped
}

void Bus::writeToCartridge(uint16_t addr, uint8_t data){
    (void)data;
    uint32_t mappedAddr = addr;
    
    // Let mapper handle address translation
    if(mapper && mapper->modCpuWrite(addr, mappedAddr)){
        // For most carts, writes to PRG ROM are ignored
        // Some mappers use this for bank switching
    }
}

// ============================================================================
// RENDERING
// ============================================================================

void Bus::renderFrame(){
    ppu.renderFrame();
}

uint8_t* Bus::getFrameBuffer(){
    return ppu.getFrameBuffer();
}

bool Bus::isFrameReady(){
    return ppu.isFrameReady();
}

void Bus::setVBlankCallback(void (*callback)()){
    ppu.setVBlankCallback(callback);
}
