#include "cpu6502.h"
#include "Bus.h"
#include <exception>
#include <iostream>
#include <SDL.h>

int main(int argc, char* argv[]) {
    if (argc <= 1){
        std::cout << "How to use: *executable name* {path to rom}" << std::endl;
        return 0;
    }

    try{
        cpu6502 cpu(argv[1]);
        cpu.reset();

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
            return 1;
        }

        SDL_Window *window = SDL_CreateWindow(
            "NES Emulator",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            256 * 3,
            240 * 3,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
        );
        if(!window){
            std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return 1;
        }

        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if(!renderer){
            std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }

        // Keep NES framebuffer aspect ratio while allowing window resizing.
        SDL_RenderSetLogicalSize(renderer, 256, 240);

        SDL_Texture *texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            256,
            240
        );

        if(!texture){
            std::cerr << "Texture creation failed: " << SDL_GetError() << std::endl;
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }

        bool running = true;
        SDL_Event event;

        while(running){
            while(SDL_PollEvent(&event)){
                if(event.type == SDL_QUIT){
                    running = false;
                }
            }

            const uint8_t *keys = SDL_GetKeyboardState(nullptr);
            uint8_t controller1 = 0x00;

            if(keys[SDL_SCANCODE_A]) controller1 |= Bus::BUTTON_A;          // NES A
            if(keys[SDL_SCANCODE_D]) controller1 |= Bus::BUTTON_B;          // NES B
            if(keys[SDL_SCANCODE_MINUS] || keys[SDL_SCANCODE_KP_MINUS]) controller1 |= Bus::BUTTON_SELECT;
            if(keys[SDL_SCANCODE_EQUALS] || keys[SDL_SCANCODE_KP_PLUS]) controller1 |= Bus::BUTTON_START;
            if(keys[SDL_SCANCODE_UP]) controller1 |= Bus::BUTTON_UP;
            if(keys[SDL_SCANCODE_DOWN]) controller1 |= Bus::BUTTON_DOWN;
            if(keys[SDL_SCANCODE_LEFT]) controller1 |= Bus::BUTTON_LEFT;
            if(keys[SDL_SCANCODE_RIGHT]) controller1 |= Bus::BUTTON_RIGHT;

            cpu.bus->setControllerState(0, controller1);

            // 3 PPU clocks for each CPU clock.
            cpu.clock();
            cpu.bus->clock();
            cpu.bus->clock();
            cpu.bus->clock();

            if(cpu.bus->ppu.frameComplete){
                cpu.bus->ppu.frameComplete = false;
                const auto &fb = cpu.bus->ppu.getFrameBuffer();
                SDL_UpdateTexture(texture, nullptr, fb.data(), 256 * static_cast<int>(sizeof(uint32_t)));
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, nullptr, nullptr);
                SDL_RenderPresent(renderer);
            }
        }

        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    } catch(std::exception &e){
        std::cerr << e.what() << " - Aborting process" << std::endl;
        return 1;
    }
    return 0;
}

