#include "platform.h"
#include "SDL2/SDL.h"
#include "chip8.h"

Platform::Platform(int width, int height, int scale, int pitch) : pitch{pitch} {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "chip8 emulator", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        width * scale, 
        height * scale, 
        SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(
        window, 
        -1, 
        SDL_RENDERER_ACCELERATED);

    texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_ABGR8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        width, 
        height);
}

Platform::~Platform() {
    // NOTE: the order is important here
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Platform::update_display(uint32_t* screen) {
	SDL_UpdateTexture(texture, nullptr, screen, pitch);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}

constexpr uint32_t keymap[] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v
};

bool Platform::update_keys(bool *keys) {

    bool quit = false;

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: quit = true; break;
            case SDL_KEYUP: 
                switch (event.key.keysym.sym) {
                    case keymap[0]: keys[0] = false; break;
                    case keymap[1]: keys[1] = false; break;
                    case keymap[2]: keys[2] = false; break;
                    case keymap[3]: keys[3] = false; break;
                    case keymap[4]: keys[4] = false; break;
                    case keymap[5]: keys[5] = false; break;
                    case keymap[6]: keys[6] = false; break;
                    case keymap[7]: keys[7] = false; break;
                    case keymap[8]: keys[8] = false; break;
                    case keymap[9]: keys[9] = false; break;
                    case keymap[10]: keys[10] = false; break;
                    case keymap[11]: keys[11] = false; break;
                    case keymap[12]: keys[12] = false; break;
                    case keymap[13]: keys[13] = false; break;
                    case keymap[14]: keys[14] = false; break;
                    case keymap[15]: keys[15] = false; break;
                }
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: quit = true; break;
                    case keymap[0]: keys[0] = true; break;
                    case keymap[1]: keys[1] = true; break;
                    case keymap[2]: keys[2] = true; break;
                    case keymap[3]: keys[3] = true; break;
                    case keymap[4]: keys[4] = true; break;
                    case keymap[5]: keys[5] = true; break;
                    case keymap[6]: keys[6] = true; break;
                    case keymap[7]: keys[7] = true; break;
                    case keymap[8]: keys[8] = true; break;
                    case keymap[9]: keys[9] = true; break;
                    case keymap[10]: keys[10] = true; break;
                    case keymap[11]: keys[11] = true; break;
                    case keymap[12]: keys[12] = true; break;
                    case keymap[13]: keys[13] = true; break;
                    case keymap[14]: keys[14] = true; break;
                    case keymap[15]: keys[15] = true; break;
                }
                break;
        }
    }

    return quit;
}
