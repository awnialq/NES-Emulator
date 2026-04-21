#include "cpu6502.h"
#include <exception>
#include <iostream>
#include "Bus.h"
#include <ostream>
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#elif __has_include(<SDL.h>)
#include <SDL.h>
#else
#error "SDL2 headers not found. Install SDL2 development headers and recompile."
#endif
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc <= 1){
        std::cout << "How to use: *executable name* {path to rom}" << std::endl;
        return 0;
    }

    try{
        cpu6502 cpu(argv[1]);

        // Initialize SDL for rendering
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
            return 1;
        }

        SDL_Window* window = SDL_CreateWindow(
            "NES Emulator",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            256 * 2,  // 2x scale
            240 * 2,
            SDL_WINDOW_SHOWN
        );

        if (window == nullptr) {
            SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
            SDL_Quit();
            return 1;
        }

        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            SDL_Log("Renderer could not be created! SDL error: %s\n", SDL_GetError());
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }

        // Create texture for frame buffer
        SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_INDEX8, SDL_TEXTUREACCESS_TARGET, 256, 240);

        // Main loop
        bool isRunning = true;
        uint32_t framesRendered = 0;
        uint32_t cpuCycles = 0;
        
        SDL_Event event;
        while (isRunning) {
            // Handle events
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    isRunning = false;
                }
            }

            // Run CPU for one frame (approximate - real emulation needs precise timing)
            // NES runs at ~60fps, CPU at 1.79MHz
            // ~29780 CPU cycles per frame
            for(int i = 0; i < 29780; i++){
                cpu.clock();
                cpuCycles++;
                
                // Check for vblank interrupt (simplified)
                if(cpuCycles % 29780 == 0) {
                    // Trigger render
                    cpu.bus->renderFrame();
                }
            }

            // Render frame
            if(cpu.bus->isFrameReady()) {
                SDL_UpdateTexture(texture, nullptr, cpu.bus->getFrameBuffer(), 256);
                
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, nullptr, nullptr);
                SDL_RenderPresent(renderer);
                
                framesRendered++;
            }

            // Frame pacing (60 FPS)
            SDL_Delay(16);
        }

        // Cleanup
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        printf("Rendered %u frames over %u CPU cycles\n", framesRendered, cpuCycles);
    } catch(std::exception &e){
        std::cerr << e.what() << " - Aborting process" << std::endl;
        return 1;
    }

    return 0;
}
