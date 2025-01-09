#include <iostream>
#include <SDL2/SDL.h>

void displayMessage() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Window* window = SDL_CreateWindow("NES Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    SDL_Surface* screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, nullptr, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
    SDL_UpdateWindowSurface(window);

    SDL_Surface* messageSurface = SDL_CreateRGBSurface(0, 640, 480, 32, 0, 0, 0, 0);
    SDL_FillRect(messageSurface, nullptr, SDL_MapRGB(messageSurface->format, 0xFF, 0xFF, 0xFF));
    SDL_Color textColor = {0, 0, 0, 0};
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (font == nullptr) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        SDL_FreeSurface(messageSurface);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Work in progress", textColor);
    if (textSurface == nullptr) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        TTF_CloseFont(font);
        SDL_FreeSurface(messageSurface);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    SDL_BlitSurface(textSurface, nullptr, messageSurface, nullptr);
    SDL_BlitSurface(messageSurface, nullptr, screenSurface, nullptr);
    SDL_UpdateWindowSurface(window);

    SDL_Delay(2000);

    SDL_FreeSurface(textSurface);
    TTF_CloseFont(font);
    SDL_FreeSurface(messageSurface);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main() {
    displayMessage();
    return 0;
}
