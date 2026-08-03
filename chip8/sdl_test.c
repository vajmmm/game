#include <SDL3/SDL.h>
#include <stdio.h>

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL3 init failed: %s\n", SDL_GetError());
        return 1;
    }
    printf("SDL3 initialized. Version: %d\n", SDL_GetVersion());
    SDL_Quit();
    return 0;
}