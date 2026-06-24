#include "cpu6502.h"
#include "Bus.h"
#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <thread>
#include <SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

struct EmulatorState {
    std::unique_ptr<cpu6502> cpu;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;
    bool running = true;
    bool sdl_ready = false;
#ifdef __EMSCRIPTEN__
    double next_frame_ms = 0.0;
#else
    std::chrono::steady_clock::time_point next_frame_deadline{};
#endif
};

namespace {
constexpr double NES_FRAME_PERIOD_MS = 1000.0 / 60.0988;
#ifndef __EMSCRIPTEN__
constexpr auto NES_FRAME_PERIOD =
    std::chrono::duration_cast<std::chrono::steady_clock::duration>(
        std::chrono::duration<double, std::nano>(1e9 / 60.0988));
#endif
}

static EmulatorState g_state;

static bool init_sdl(EmulatorState &state) {
    if (state.sdl_ready) {
        return true;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    state.window = SDL_CreateWindow(
        "NES Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        256 * 3,
        240 * 3,
#ifdef __EMSCRIPTEN__
        0
#else
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
#endif
    );
    if (!state.window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // We pace ourselves to the NES's native frame rate (see frame_period
    // below), so we deliberately don't enable SDL_RENDERER_PRESENTVSYNC --
    // letting the host display refresh gate us would warp emulation speed
    // on anything that isn't a 60 Hz panel (75/120/144/ProMotion, etc.).
    state.renderer = SDL_CreateRenderer(state.window, -1, SDL_RENDERER_ACCELERATED);
    if (!state.renderer) {
        state.renderer = SDL_CreateRenderer(state.window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!state.renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(state.window);
        state.window = nullptr;
        SDL_Quit();
        return false;
    }

    // Keep NES framebuffer aspect ratio while allowing window resizing.
    SDL_RenderSetLogicalSize(state.renderer, 256, 240);

    state.texture = SDL_CreateTexture(
        state.renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        256,
        240
    );
    if (!state.texture) {
        std::cerr << "Texture creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(state.renderer);
        SDL_DestroyWindow(state.window);
        state.renderer = nullptr;
        state.window = nullptr;
        SDL_Quit();
        return false;
    }

    state.sdl_ready = true;
    return true;
}

#ifndef __EMSCRIPTEN__
static void shutdown_sdl(EmulatorState &state) {
    if (state.texture) {
        SDL_DestroyTexture(state.texture);
        state.texture = nullptr;
    }
    if (state.renderer) {
        SDL_DestroyRenderer(state.renderer);
        state.renderer = nullptr;
    }
    if (state.window) {
        SDL_DestroyWindow(state.window);
        state.window = nullptr;
    }
    if (state.sdl_ready) {
        SDL_Quit();
        state.sdl_ready = false;
    }
}
#endif

static bool load_cpu(EmulatorState &state, const char *rom_path) {
    try {
        state.cpu = std::make_unique<cpu6502>(const_cast<char *>(rom_path));
        state.cpu->reset();
        return true;
    } catch (std::exception &e) {
        std::cerr << e.what() << " - Failed to load ROM" << std::endl;
        state.cpu.reset();
        return false;
    }
}

static void poll_input_and_present(EmulatorState &state, bool handle_quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (handle_quit && event.type == SDL_QUIT) {
            state.running = false;
        }
    }

    const uint8_t *keys = SDL_GetKeyboardState(nullptr);
    uint8_t controller1 = 0x00;

    if (keys[SDL_SCANCODE_A]) controller1 |= Bus::BUTTON_A;
    if (keys[SDL_SCANCODE_D]) controller1 |= Bus::BUTTON_B;
    if (keys[SDL_SCANCODE_MINUS] || keys[SDL_SCANCODE_KP_MINUS]) controller1 |= Bus::BUTTON_SELECT;
    if (keys[SDL_SCANCODE_EQUALS] || keys[SDL_SCANCODE_KP_PLUS]) controller1 |= Bus::BUTTON_START;
    if (keys[SDL_SCANCODE_UP]) controller1 |= Bus::BUTTON_UP;
    if (keys[SDL_SCANCODE_DOWN]) controller1 |= Bus::BUTTON_DOWN;
    if (keys[SDL_SCANCODE_LEFT]) controller1 |= Bus::BUTTON_LEFT;
    if (keys[SDL_SCANCODE_RIGHT]) controller1 |= Bus::BUTTON_RIGHT;

    state.cpu->bus->setControllerState(0, controller1);

    const auto &fb = state.cpu->bus->ppu.getFrameBuffer();
    SDL_UpdateTexture(
        state.texture,
        nullptr,
        fb.data(),
        256 * static_cast<int>(sizeof(uint32_t))
    );
    SDL_RenderClear(state.renderer);
    SDL_RenderCopy(state.renderer, state.texture, nullptr, nullptr);
    SDL_RenderPresent(state.renderer);
}

static void emulate_until_frame(EmulatorState &state) {
    while (!state.cpu->bus->ppu.frameComplete) {
        // 3 PPU clocks for each CPU clock.
        state.cpu->clock();
        state.cpu->bus->clock();
        state.cpu->bus->clock();
        state.cpu->bus->clock();
    }
    state.cpu->bus->ppu.frameComplete = false;
}

#ifndef __EMSCRIPTEN__
static void pace_frame(EmulatorState &state) {
    using clock = std::chrono::steady_clock;

    const auto now = clock::now();
    if (state.next_frame_deadline > now) {
        const auto sleep_until_point = state.next_frame_deadline - std::chrono::microseconds(500);
        if (sleep_until_point > now) {
            std::this_thread::sleep_until(sleep_until_point);
        }
        while (clock::now() < state.next_frame_deadline) {
            // spin
        }
        state.next_frame_deadline += NES_FRAME_PERIOD;
    } else {
        const auto lag = now - state.next_frame_deadline;
        if (lag > NES_FRAME_PERIOD * 5) {
            state.next_frame_deadline = now + NES_FRAME_PERIOD;
        } else {
            state.next_frame_deadline += NES_FRAME_PERIOD;
        }
    }
}
#endif

#ifdef __EMSCRIPTEN__
static bool pace_frame_web(EmulatorState &state) {
    const double now = emscripten_get_now();
    if (state.next_frame_ms == 0.0) {
        state.next_frame_ms = now;
    }
    if (now < state.next_frame_ms) {
        return false;
    }

    const double lag = now - state.next_frame_ms;
    if (lag > NES_FRAME_PERIOD_MS * 5.0) {
        state.next_frame_ms = now + NES_FRAME_PERIOD_MS;
    } else {
        state.next_frame_ms += NES_FRAME_PERIOD_MS;
    }
    return true;
}
#endif

#ifdef __EMSCRIPTEN__
static void run_frame(void *arg) {
    auto *state = static_cast<EmulatorState *>(arg);
    if (!state->cpu || !state->sdl_ready) {
        return;
    }
    if (!pace_frame_web(*state)) {
        return;
    }

    emulate_until_frame(*state);

    // Input and window events are sampled once per frame (~60 Hz),
    // not once per CPU cycle (~1.79 MHz). Polling SDL on every CPU
    // tick was a major source of overhead.
    poll_input_and_present(*state, false);
}

extern "C" int load_rom(const char *path) {
    if (!init_sdl(g_state)) {
        return 0;
    }
    if (!load_cpu(g_state, path)) {
        return 0;
    }
    g_state.next_frame_ms = 0.0;

    static bool loop_started = false;
    if (!loop_started) {
        emscripten_set_main_loop_arg(run_frame, &g_state, 0, 1);
        loop_started = true;
    }
    return 1;
}

int main(int /*argc*/, char * /*argv*/[]) {
    // SDL is initialized on first ROM upload so the browser has a user gesture
    // for canvas/WebGL setup.
    return 0;
}
#else
int main(int argc, char *argv[]) {
    if (argc <= 1) {
        std::cout << "How to use: *executable name* {path to rom}" << std::endl;
        return 0;
    }

    if (!init_sdl(g_state)) {
        return 1;
    }
    if (!load_cpu(g_state, argv[1])) {
        shutdown_sdl(g_state);
        return 1;
    }

    using clock = std::chrono::steady_clock;
    g_state.next_frame_deadline = clock::now() + NES_FRAME_PERIOD;

    while (g_state.running) {
        emulate_until_frame(g_state);
        poll_input_and_present(g_state, true);
        pace_frame(g_state);
    }

    g_state.cpu.reset();
    shutdown_sdl(g_state);
    return 0;
}
#endif
