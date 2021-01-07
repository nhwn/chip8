#include <array>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#ifndef CHIP8_SCREEN_H
#define CHIP8_SCREEN_H

constexpr uint32_t ACTIVE_COLOR = 0xFFFFFFFF; // white

template <size_t WIDTH, size_t HEIGHT> 
class Screen {
public:
    constexpr size_t width() noexcept { 
        return WIDTH; 
    }

    constexpr size_t height() noexcept { 
        return HEIGHT; 
    }

    uint32_t* data() { 
        return screen.data(); 
    }

    void clear() noexcept { 
        std::fill(screen.begin(), screen.end(), 0); 
    }

    // true if the pixel at (x, y) was already on, false otherwise
    bool draw(const uint8_t x, const uint8_t y) noexcept {
        uint8_t wrapped_x = x % WIDTH;
        uint8_t wrapped_y = y % HEIGHT;
        size_t offset = wrapped_x + wrapped_y * WIDTH;
        bool is_on = screen[offset] == ACTIVE_COLOR;

        // NOTE: unchecked because we wrapped, which guarantees valid offsets
        screen[offset] = is_on ? 0 : ACTIVE_COLOR;

        return is_on;
    }
private:
    // NOTE: I'm not sure if it's UB to pass a pointer to a multidimensional
    // array to SDL, so we're gonna use a 1D array with a stride
    std::array<uint32_t, WIDTH * HEIGHT> screen = {};
};

#endif
