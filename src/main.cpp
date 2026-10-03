#include "cpu6502.h"
#include "Bus.h"
#include <array>
#include <chrono>
#include <cstdio>
#include <exception>
#include <iostream>
#include <thread>
#include <vector>
#include <SDL.h>

namespace {
constexpr int kFrameWidth = 256;
constexpr int kFrameHeight = 240;
constexpr int kWindowScale = 3;
// Status bar drawn above the PPU output, in NES pixels.
constexpr int kBarHeight = 12;
constexpr int kCanvasHeight = kFrameHeight + kBarHeight;

// 5x7 bitmap font: one byte per row, bit 4 is the leftmost column.
constexpr int kGlyphWidth = 5;
constexpr int kGlyphHeight = 7;
using Glyph = std::array<uint8_t, kGlyphHeight>;

constexpr std::array<Glyph, 10> kDigitGlyphs = {{
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, // 0
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, // 1
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}, // 2
    {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}, // 3
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, // 4
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, // 5
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, // 6
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, // 7
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, // 8
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}, // 9
}};
constexpr Glyph kDotGlyph = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C};
constexpr Glyph kFGlyph = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
constexpr Glyph kPGlyph = {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
constexpr Glyph kSGlyph = {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
constexpr Glyph kBlankGlyph = {};

const Glyph &glyphFor(char c) {
    if (c >= '0' && c <= '9') return kDigitGlyphs[c - '0'];
    switch (c) {
        case '.': return kDotGlyph;
        case 'F': return kFGlyph;
        case 'P': return kPGlyph;
        case 'S': return kSGlyph;
        default: return kBlankGlyph;
    }
}

// Measures the real presented frame rate, averaged over short windows so the
// readout doesn't flicker every frame.
struct FpsCounter {
    using clock = std::chrono::steady_clock;
    clock::time_point windowStart = clock::now();
    int frames = 0;
    double fps = 0.0;

    void tick() {
        ++frames;
        const auto now = clock::now();
        const std::chrono::duration<double> elapsed = now - windowStart;
        if (elapsed.count() >= 0.5) {
            fps = frames / elapsed.count();
            frames = 0;
            windowStart = now;
        }
    }
};

struct SdlFrontend {
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;
    SDL_AudioDeviceID audioDevice = 0;
    std::vector<SDL_Rect> textRects;

    ~SdlFrontend() {
        shutdown();
    }

    bool initialize() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
            std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
            return false;
        }

        window = SDL_CreateWindow(
            "NES Emulator",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            kFrameWidth * kWindowScale,
            kCanvasHeight * kWindowScale,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
        );
        if (!window) {
            std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
            shutdown();
            return false;
        }

        // We pace ourselves to the NES's native frame rate (see frame_period
        // below), so we deliberately don't enable SDL_RENDERER_PRESENTVSYNC --
        // letting the host display refresh gate us would warp emulation speed
        // on anything that isn't a 60 Hz panel (75/120/144/ProMotion, etc.).
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
            shutdown();
            return false;
        }

        // Keep NES framebuffer (plus status bar) aspect ratio while allowing
        // window resizing.
        SDL_RenderSetLogicalSize(renderer, kFrameWidth, kCanvasHeight);

        texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            kFrameWidth,
            kFrameHeight
        );

        if (!texture) {
            std::cerr << "Texture creation failed: " << SDL_GetError() << std::endl;
            shutdown();
            return false;
        }

        // Reserve an audio device now so the frontend can grow into real APU
        // output without changing the overall emulator loop structure.
        SDL_AudioSpec desired{};
        desired.freq = 44100;
        desired.format = AUDIO_F32;
        desired.channels = 1;
        desired.samples = 512;

        SDL_AudioSpec obtained{};
        audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
        if (!audioDevice) {
            std::cerr << "Audio device unavailable: " << SDL_GetError() << std::endl;
        } else {
            SDL_PauseAudioDevice(audioDevice, 0);
        }

        return true;
    }

    template <typename FrameBuffer>
    void presentFrame(const FrameBuffer &frameBuffer, double fps) {
        SDL_UpdateTexture(texture, nullptr, frameBuffer.data(), kFrameWidth * static_cast<int>(sizeof(uint32_t)));
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);

        const SDL_Rect barRect{0, 0, kFrameWidth, kBarHeight};
        SDL_SetRenderDrawColor(renderer, 0x20, 0x20, 0x20, 0xFF);
        SDL_RenderFillRect(renderer, &barRect);

        char fpsText[16];
        std::snprintf(fpsText, sizeof fpsText, "FPS %.1f", fps);
        SDL_SetRenderDrawColor(renderer, 0xE0, 0xE0, 0xE0, 0xFF);
        drawText(2, (kBarHeight - kGlyphHeight) / 2, fpsText);

        const SDL_Rect gameRect{0, kBarHeight, kFrameWidth, kFrameHeight};
        SDL_RenderCopy(renderer, texture, nullptr, &gameRect);
        SDL_RenderPresent(renderer);
    }

    // Draws text in logical (NES-pixel) coordinates using the current draw
    // color. Each lit font pixel becomes a 1x1 rect, which SDL scales up with
    // the rest of the canvas.
    void drawText(int x, int y, const char *text) {
        textRects.clear();
        for (int i = 0; text[i] != '\0'; ++i) {
            const Glyph &glyph = glyphFor(text[i]);
            const int glyphX = x + i * (kGlyphWidth + 1);
            for (int row = 0; row < kGlyphHeight; ++row) {
                for (int col = 0; col < kGlyphWidth; ++col) {
                    if (glyph[row] & (0x10 >> col)) {
                        textRects.push_back(SDL_Rect{glyphX + col, y + row, 1, 1});
                    }
                }
            }
        }
        SDL_RenderFillRects(renderer, textRects.data(), static_cast<int>(textRects.size()));
    }

    void shutdown() {
        if (audioDevice) {
            SDL_CloseAudioDevice(audioDevice);
            audioDevice = 0;
        }

        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }

        if (renderer) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }

        if (window) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }

        SDL_Quit();
    }
};

uint8_t readController1State() {
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

    return controller1;
}
} // namespace

int main(int argc, char* argv[]) {
    if (argc <= 1){
        std::cout << "How to use: *executable name* {path to rom}" << std::endl;
        return 0;
    }

    try{
        cpu6502 cpu(argv[1]);
        cpu.reset();

        SdlFrontend frontend;
        if (!frontend.initialize()) {
            return 1;
        }

        bool running = true;
        SDL_Event event;
        FpsCounter fpsCounter;

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

                cpu.bus->setControllerState(0, readController1State());

                const auto &fb = cpu.bus->ppu.getFrameBuffer();
                fpsCounter.tick();
                frontend.presentFrame(fb, fpsCounter.fps);

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

    } catch(std::exception &e){
        std::cerr << e.what() << " - Aborting process" << std::endl;
        return 1;
    }
    return 0;
}