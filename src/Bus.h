#pragma once
#include <cstdint>
#include "cpu6502.h"
#include "cartridge.h"
#include "ppu2C02.h"
#include <array>
#include <memory>
#include <vector>

class Bus{
    public:
        enum ControllerButton : uint8_t {
            BUTTON_A      = 1 << 7,
            BUTTON_B      = 1 << 6,
            BUTTON_SELECT = 1 << 5,
            BUTTON_START  = 1 << 4,
            BUTTON_UP     = 1 << 3,
            BUTTON_DOWN   = 1 << 2,
            BUTTON_LEFT   = 1 << 1,
            BUTTON_RIGHT  = 1 << 0
        };

        Bus(char *);
        ~Bus();

    public:
        std::array<uint8_t, 2048> cpuMem; 
        ppu2C02 ppu;
        void cpuWrite(uint16_t addr, uint8_t data);
        uint8_t cpuRead(uint16_t addr, bool readOnly = false);
        void clock();
        void setControllerState(uint8_t port, uint8_t state);
        //void reset();
        //void clock();
        cartridge *cart = nullptr;
    private:
        std::array<uint8_t, 2> controller{};
        std::array<uint8_t, 2> controllerState{};
        bool controllerStrobe = false;
        uint32_t clockCntr = 0;
};