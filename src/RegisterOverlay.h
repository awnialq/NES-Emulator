#pragma once

#include "cpu6502.h"
#include <cstdint>
#include <SDL.h>

namespace RegisterOverlay {

constexpr int HeaderHeight = 32;

struct CpuRegisterSnapshot {
    uint16_t pc = 0x0000;
    uint8_t a = 0x00;
    uint8_t x = 0x00;
    uint8_t y = 0x00;
    uint8_t sp = 0x00;
    uint8_t status = 0x00;
};

CpuRegisterSnapshot capture(const cpu6502 &cpu);
void render(SDL_Renderer *renderer, const CpuRegisterSnapshot &registers);

}
