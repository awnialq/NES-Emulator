#include "cpu6502.h"
#include "Bus.h"
#include "RegisterOverlay.h"
#include <chrono>
#include <exception>
#include <iostream>
#include <thread>
#include <SDL.h>

int main(int argc, char* argv[]) {
    if (argc <= 1){
        std::cout << "How to use: *executable name* {path to rom}" << std::endl;
        return 0;
    }

    try{
        constexpr int nes_width = 256;
        constexpr int nes_height = 240;
        constexpr int window_scale = 3;
        constexpr int logical_height = nes_height + RegisterOverlay::HeaderHeight;

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
            nes_width * window_scale,
            logical_height * window_scale,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
        );
        if(!window){
            std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return 1;
        }

        // We pace ourselves to the NES's native frame rate (see frame_period
        // below), so we deliberately don't enable SDL_RENDERER_PRESENTVSYNC --
        // letting the host display refresh gate us would warp emulation speed
        // on anything that isn't a 60 Hz panel (75/120/144/ProMotion, etc.).
        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if(!renderer){
            std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }

        // Keep the header and NES framebuffer in one logical canvas while resizing.
        SDL_RenderSetLogicalSize(renderer, nes_width, logical_height);

        SDL_Texture *texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            nes_width,
            nes_height
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
        auto register_snapshot = RegisterOverlay::capture(cpu);

        // one frame every ~16.6391 ms
        using clock = std::chrono::steady_clock;
        constexpr auto frame_period =
            std::chrono::duration_cast<clock::duration>(
                std::chrono::duration<double, std::nano>(1e9 / 60.0988));

        auto next_frame_deadline = clock::now() + frame_period;

        while(running){
            // 3 PPU clocks for each CPU clock.
            cpu.clock();
            cpu.bus->clock();
            cpu.bus->clock();
            cpu.bus->clock();
            if(cpu.cycles == 0){
                register_snapshot = RegisterOverlay::capture(cpu);
            }

            if(cpu.bus->ppu.frameComplete){
                cpu.bus->ppu.frameComplete = false;

                // Input and window events are sampled once per frame (~60 Hz),
                // not once per CPU cycle (~1.79 MHz). Polling SDL on every CPU
                // tick was a major source of overhead.
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

                const auto &fb = cpu.bus->ppu.getFrameBuffer();
                SDL_UpdateTexture(texture, nullptr, fb.data(), nes_width * static_cast<int>(sizeof(uint32_t)));
                SDL_RenderClear(renderer);
                RegisterOverlay::render(renderer, register_snapshot);
                SDL_Rect game_viewport{0, RegisterOverlay::HeaderHeight, nes_width, nes_height};
                SDL_RenderCopy(renderer, texture, nullptr, &game_viewport);
                SDL_RenderPresent(renderer);

                // Real-time pacing: hold the loop until the next NES frame
                // tick. sleep_until handles the bulk of the wait cheaply; the
                // short spin afterwards absorbs OS sleep jitter (sleep
                // granularity on most desktops is ~1 ms).
                const auto now = clock::now();
                if(next_frame_deadline > now){
                    const auto sleep_until_point = next_frame_deadline - std::chrono::microseconds(500);
                    if(sleep_until_point > now){
                        std::this_thread::sleep_until(sleep_until_point);
                    }
                    while(clock::now() < next_frame_deadline){
                        // spin
                    }
                    next_frame_deadline += frame_period;
                } else {
                    // We're behind schedule. If we're only a little behind,
                    // just skip the wait and let the next frame catch up.
                    // If we're catastrophically behind (e.g. the window was
                    // dragged, we were suspended, a breakpoint was hit), don't
                    // try to "make up" hundreds of frames -- resync instead.
                    const auto lag = now - next_frame_deadline;
                    if(lag > frame_period * 5){
                        next_frame_deadline = now + frame_period;
                    } else {
                        next_frame_deadline += frame_period;
                    }
                }
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

