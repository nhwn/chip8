#include "chip8.h"
#include "platform.h"
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <string>
#include "SDL2/SDL.h"

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "usage: chip8 <ROM> <video delay> <cycle delay>";
        return EXIT_FAILURE;
    }

    // these names are SO long
    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    char* filename = argv[1];
    int video_scale = std::stoi(argv[2]);
    int cycle_delay = std::stoi(argv[3]);
    bool quit = false;
    auto prev = high_resolution_clock::now();

    Chip8 emu{};

    int pitch = sizeof(uint32_t) * emu.screen.width();

    Platform platform{
        emu.screen.width(), 
        emu.screen.height(), 
        video_scale, 
        pitch};

    try {
        emu.load_rom(filename);

        while (!quit) {
            quit = platform.update_keys(emu.keys_pressed.data());

            auto now = high_resolution_clock::now();
            auto dt = duration<float, milliseconds::period>(now - prev).count();

            if (dt > cycle_delay) {
                prev = now;

                emu.cycle();
                platform.update_display(emu.screen.data());
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "chip8: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS; // unreachable
}
