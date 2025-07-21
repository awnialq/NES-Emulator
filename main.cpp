#include "cpu6502.h"
#include <SDL2/SDL.h>
#include <iostream>

int main(){
if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow("Hello SDL", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (!win) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Delay(2000);  // Display window for 2 seconds
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
    /*
    cpu6502 *cpu = new cpu6502();
    cpu -> ~cpu6502();
    */
}

