#include "SDL2/SDL.h"
#include <string_view>
#include <cstdint>

class Platform {
public:
    Platform(int width, int height, int scale, int pitch);
    ~Platform();
    void update_display(uint32_t* screen);
    bool update_keys(bool* keys);

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    int pitch;
};
