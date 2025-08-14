#include "cpu6502.h"
#include <SDL3/SDL.h>
#include <iostream>
#include "Bus.h"
#include <ostream>

int main(int argc, char* argv[]) {

    cpu6502 cpu = cpu6502();

    for(int i = 0; i <  16000; i++){
        cpu.clock();
    }
    printf("%d%d %d%d", cpu.bus->cpuMem[2], cpu.bus->cpuMem[3], cpu.bus->cpuMem[4], cpu.bus->cpuMem[5]);
    /*
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Test Window",
        640,
        480,
        0 // No special flags
    );

    if (window == nullptr) {
        SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_Log("Renderer could not be created! SDL error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Main loop
    bool isRunning = true;
    SDL_Event event;
    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
        }

        // Set draw color to cornflower blue
        SDL_SetRenderDrawColor(renderer, 100, 149, 237, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    */
    return 0;
}

