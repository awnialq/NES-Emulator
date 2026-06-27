#include "RegisterOverlay.h"
#include <array>
#include <cstdio>
#include <string>

namespace {

constexpr int char_width = 3;
constexpr int char_height = 5;
constexpr int glyph_scale = 2;
constexpr int char_spacing = 1;
constexpr int text_x = 2;
constexpr int first_row_y = 4;
constexpr int second_row_y = 18;

using Glyph = std::array<uint8_t, char_height>;

Glyph glyphFor(char c){
    switch(c){
        case '0': return {0b111, 0b101, 0b101, 0b101, 0b111};
        case '1': return {0b010, 0b110, 0b010, 0b010, 0b111};
        case '2': return {0b111, 0b001, 0b111, 0b100, 0b111};
        case '3': return {0b111, 0b001, 0b111, 0b001, 0b111};
        case '4': return {0b101, 0b101, 0b111, 0b001, 0b001};
        case '5': return {0b111, 0b100, 0b111, 0b001, 0b111};
        case '6': return {0b111, 0b100, 0b111, 0b101, 0b111};
        case '7': return {0b111, 0b001, 0b010, 0b010, 0b010};
        case '8': return {0b111, 0b101, 0b111, 0b101, 0b111};
        case '9': return {0b111, 0b101, 0b111, 0b001, 0b111};
        case 'A': return {0b010, 0b101, 0b111, 0b101, 0b101};
        case 'B': return {0b110, 0b101, 0b110, 0b101, 0b110};
        case 'C': return {0b111, 0b100, 0b100, 0b100, 0b111};
        case 'D': return {0b110, 0b101, 0b101, 0b101, 0b110};
        case 'E': return {0b111, 0b100, 0b110, 0b100, 0b111};
        case 'F': return {0b111, 0b100, 0b110, 0b100, 0b100};
        case 'I': return {0b111, 0b010, 0b010, 0b010, 0b111};
        case 'N': return {0b101, 0b111, 0b111, 0b111, 0b101};
        case 'P': return {0b110, 0b101, 0b110, 0b100, 0b100};
        case 'S': return {0b111, 0b100, 0b111, 0b001, 0b111};
        case 'U': return {0b101, 0b101, 0b101, 0b101, 0b111};
        case 'V': return {0b101, 0b101, 0b101, 0b101, 0b010};
        case 'X': return {0b101, 0b101, 0b010, 0b101, 0b101};
        case 'Y': return {0b101, 0b101, 0b010, 0b010, 0b010};
        case 'Z': return {0b111, 0b001, 0b010, 0b100, 0b111};
        case ':': return {0b000, 0b010, 0b000, 0b010, 0b000};
        default: return {0b000, 0b000, 0b000, 0b000, 0b000};
    }
}

void drawText(SDL_Renderer *renderer, const std::string &text, int x, int y){
    for(char c : text){
        if(c == ' '){
            x += (char_width * glyph_scale) + char_spacing;
            continue;
        }

        const Glyph glyph = glyphFor(c);
        for(int row = 0; row < char_height; row++){
            for(int col = 0; col < char_width; col++){
                if((glyph[row] & (1 << (char_width - 1 - col))) != 0){
                    SDL_Rect pixel{
                        x + (col * glyph_scale),
                        y + (row * glyph_scale),
                        glyph_scale,
                        glyph_scale
                    };
                    SDL_RenderFillRect(renderer, &pixel);
                }
            }
        }

        x += (char_width * glyph_scale) + char_spacing;
    }
}

std::string formatCoreRegisterLine(const RegisterOverlay::CpuRegisterSnapshot &registers){
    char buffer[48];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "PC:%04X A:%02X X:%02X Y:%02X SP:%02X P:%02X",
        static_cast<unsigned>(registers.pc),
        static_cast<unsigned>(registers.a),
        static_cast<unsigned>(registers.x),
        static_cast<unsigned>(registers.y),
        static_cast<unsigned>(registers.sp),
        static_cast<unsigned>(registers.status)
    );
    return buffer;
}

std::string formatStatusFlagLine(const RegisterOverlay::CpuRegisterSnapshot &registers){
    char buffer[32];
    const uint8_t status = registers.status;
    std::snprintf(
        buffer,
        sizeof(buffer),
        "NVUBDIZC:%u%u%u%u%u%u%u%u",
        static_cast<unsigned>((status & cpu6502::N) != 0),
        static_cast<unsigned>((status & cpu6502::V) != 0),
        static_cast<unsigned>((status & cpu6502::U) != 0),
        static_cast<unsigned>((status & cpu6502::B) != 0),
        static_cast<unsigned>((status & cpu6502::D) != 0),
        static_cast<unsigned>((status & cpu6502::I) != 0),
        static_cast<unsigned>((status & cpu6502::Z) != 0),
        static_cast<unsigned>((status & cpu6502::C) != 0)
    );
    return buffer;
}

}

namespace RegisterOverlay {

CpuRegisterSnapshot capture(const cpu6502 &cpu){
    return {
        cpu.progc,
        cpu.accum,
        cpu.x,
        cpu.y,
        cpu.stackp,
        cpu.status
    };
}

void render(SDL_Renderer *renderer, const CpuRegisterSnapshot &registers){
    SDL_SetRenderDrawColor(renderer, 16, 16, 24, 255);
    SDL_Rect background{0, 0, 256, HeaderHeight};
    SDL_RenderFillRect(renderer, &background);

    SDL_SetRenderDrawColor(renderer, 84, 84, 96, 255);
    SDL_Rect divider{0, HeaderHeight - 1, 256, 1};
    SDL_RenderFillRect(renderer, &divider);

    SDL_SetRenderDrawColor(renderer, 232, 232, 232, 255);
    drawText(renderer, formatCoreRegisterLine(registers), text_x, first_row_y);
    drawText(renderer, formatStatusFlagLine(registers), text_x, second_row_y);
}

}
