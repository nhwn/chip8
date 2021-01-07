#include "chip8.h"
#include <cstdint>
#include <stdexcept>
#include <array>
#include <algorithm>
#include <string_view>
#include <iostream>
#include <iomanip>

constexpr auto FONT_STRIDE = 5;
constexpr auto FONT_ADDRESS = 0x50;

const std::array<uint8_t, FONT_STRIDE * 16> fontset = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

static bool bit_at(uint8_t byte, uint8_t offset) {
    return byte >> offset & 1;
}

Chip8::Chip8() {
    std::copy(fontset.begin(), fontset.end(), memory.begin() + FONT_ADDRESS);
}

void Chip8::load_rom(const std::string_view filename) {
    std::ifstream rom{filename.data(), std::ios::binary | std::ios::in};

    if (rom.is_open()) {
        auto space = memory.size() - START_ADDRESS;
        // NOTE: this cast is necessary since `read` only takes signed char; the
        // Standard guarantees that this is not UB due to special-casing for
        // byte-like types
        rom.read(reinterpret_cast<char*>(memory.data() + START_ADDRESS), space);

        /* if (rom.peek()) { */
        /*     throw std::runtime_error{"ROM file is too large"}; */
        /* } */
    } else {
        throw std::runtime_error{"error opening ROM file"};
    }
}

void Chip8::cycle() {
    fetch_instruction();
    increment_pc();
    execute_instruction();
    decrement_timers();
}

void Chip8::reset() {
    std::fill(memory.begin(), memory.end(), 0);
    std::fill(registers.begin(), registers.end(), 0);

    stack.clear();
    screen.clear();

    std::copy(fontset.begin(), fontset.end(), memory.begin() + FONT_ADDRESS);

    index = 0;
    pc = START_ADDRESS;
    sound_timer = 0;
    delay_timer = 0;
    instruction = 0;
}

void Chip8::fetch_instruction() {
    instruction = (memory.at(pc) << 8) | memory.at(pc + 1);
}

void Chip8::increment_pc() {
    pc += sizeof(instruction);
}

void Chip8::decrement_pc() {
    pc -= sizeof(instruction);
}

void Chip8::execute_instruction() {
    // we could do a jump table, but let's keep things simple(ish)
    switch (instruction >> 12) {
        case 0x0:
            switch (extract_nn()) {
                case 0xE0: cls(); break;
                case 0xEE: ret(); break;
                default: illegal(); break;
            }
            break;
        case 0x1: jp_nnn(); break;
        case 0x2: call_nnn(); break;
        case 0x3: se_vx_nn(); break;
        case 0x4: sne_vx_nn(); break;
        case 0x5: extract_n() ? illegal() : se_vx_vy(); break;
        case 0x6: ld_vx_nn(); break;
        case 0x7: add_vx_nn(); break;
        case 0x8:
            switch (extract_n()) {
                case 0x0: ld_vx_vy(); break;
                case 0x1: or_vx_vy(); break;
                case 0x2: and_vx_vy(); break;
                case 0x3: xor_vx_vy(); break;
                case 0x4: add_vx_vy(); break;
                case 0x5: sub_vx_vy(); break;
                case 0x6: shr_vx(); break;
                case 0x7: subn_vx_vy(); break;
                case 0x8: shl_vx(); break;
                default: illegal(); break;
            }
            break;
        case 0x9: extract_n() ? illegal() : sne_vx_vy(); break;
        case 0xA: ld_i_nnn(); break;
        case 0xB: jp_v0_nnn(); break;
        case 0xC: rnd_vx_nn(); break;
        case 0xD: drw_vx_vy_n(); break;
        case 0xE:
            switch (extract_nn()) {
                case 0x9E: skp_vx(); break;
                case 0xA1: sknp_vx(); break;
                default: illegal(); break;
            }
            break;
        case 0xF:
            switch (extract_nn()) {
                case 0x07: ld_vx_dt(); break;
                case 0x0A: ld_vx_k(); break;
                case 0x15: ld_dt_vx(); break;
                case 0x18: ld_st_vx(); break;
                case 0x1E: add_i_vx(); break;
                case 0x29: ld_f_vx(); break;
                case 0x33: ld_b_vx(); break;
                case 0x55: ld_mem_vx(); break;
                case 0x65: ld_vx_mem(); break;
            }
            break;
        default: illegal(); break;
    }
}

void Chip8::decrement_timers() {
    if (delay_timer > 0) {
        --delay_timer;
    }

    if (sound_timer > 0) {
        --sound_timer;
    } else {
        // TODO: beep (isn't that really annoying though?)
    }
}

// convenient alias for accessing the contents of `VX` (bounds are unchecked 
// since it's impossible for an instruction to provide an argument larger than 
// the number of registers (16))
uint8_t& Chip8::vx() {
    return registers[extract_x()];
}

// convenient alias for accessing the contents of `VY` (bounds are unchecked 
// since it's impossible for an instruction to provide an argument larger than 
// the number of registers (16))
uint8_t& Chip8::vy() {
    return registers[extract_y()];
}

// convenient alias for accessing the contents of `VF` (bounds are unchecked 
// since the index is hardcoded)
uint8_t& Chip8::vf() {
    return registers[0xF];
}

// convenient alias to throw an exception when we find an illegal instruction
void Chip8::illegal() {
    throw std::runtime_error{"encountered illegal instruction"};
}

// convenient alias to skip the next instruction based on a given condition
void Chip8::skip_if(const bool condition) {
    if (condition) {
        increment_pc();
    }
}

// extracts the 3 least significant nibbles from the current instruction 
// (e.g. 0x1234 -> 0x0234)
uint16_t Chip8::extract_nnn() {
    return instruction & 0x0FFF;
}

// extracts the 2 least significant nibbles from the current instruction 
// (e.g. 0x1234 -> 0x34)
uint8_t Chip8::extract_nn() {
    return instruction & 0x00FF;
}

// extracts the least significant nibble from the current instruction 
// (e.g. 0x1234 -> 0x04)
uint8_t Chip8::extract_n() {
    return instruction & 0x000F;
}

// extracts the 2nd most significant nibble from the current instruction 
// (e.g. 0x1234 -> 0x02)
uint8_t Chip8::extract_x() {
    return (instruction & 0x0F00) >> 8;
}

// extracts the 3rd most significant nibble from the current instruction 
// (e.g. 0x1234 -> 0x03)
uint8_t Chip8::extract_y() {
    return (instruction & 0x00F0) >> 4;
}

// 0x00E0 - clear the screen
void Chip8::cls() {
    screen.clear();
}

// 0x00EE - return from a subroutine by jumping to a previously-saved address
void Chip8::ret() {
    pc = stack.pop();
}

// 0x1NNN - save address of the next instruction, then jump to address 0x0NNN
void Chip8::jp_nnn() {
    pc = extract_nnn();
}

// 0x2NNN - directly jump to address 0x0NNN (don't save the current value of `pc`)
void Chip8::call_nnn() {
    stack.push(pc);
    jp_nnn();
}

// 0x3XNN - skip the next instruction if `VX` == 0xNN
void Chip8::se_vx_nn() {
    skip_if(vx() == extract_nn());
}

// 0x4XNN - skip the next instruction if `VX` != 0xNN
void Chip8::sne_vx_nn() {
    skip_if(vx() != extract_nn());
}

// 0x5XY0 - skip the next instruction if `VX` == `VY`
void Chip8::se_vx_vy() {
    skip_if(vx() == vy());
}

// 0x6XNN - store 0xNN in `VX`
void Chip8::ld_vx_nn() {
    vx() = extract_nn();
}

// 0x7XNN - add 0xNN to `VX` (works mod 256 because unsigned arithmetic)
void Chip8::add_vx_nn() {
    vx() += extract_nn();
}

// 0x8XY0 - store `VY` in `VX`
void Chip8::ld_vx_vy() {
    vx() = vy();
}

// 0x8XY1 - bitwise OR `VX` with `VY` in place
void Chip8::or_vx_vy() {
    vx() |= vy();
}

// 0x8XY2 - bitwise AND `VX` with `VY` in place
void Chip8::and_vx_vy() {
    vx() &= vy();
}

// 0x8XY3 - bitwise XOR `VX` with `VY` in place
void Chip8::xor_vx_vy() {
    vx() ^= vy();
}

// 0x8XY4 - add `VX` and `VY`, storing the result in `VX`; if the sum 
// exceeds 255 (i.e. overflow), then set `VF` to 1 (otherwise, 0)
void Chip8::add_vx_vy() {
    // NOTE: this is a cheapo way to implement this, but it's fairly legible
    const uint16_t sum = vx() + vy();

    vf() = sum > UINT8_MAX;
    vx() = sum;
}

// 0x8XY5 - subtract `VY` from `VX`, storing the result in `VX`;
// if `VX` is greater than `VY`, then set `VF` to 1 (otherwise, 0)
void Chip8::sub_vx_vy() {
    vf() = vx() > vy();
    vx() -= vy();
}

// 0x8XY6 - shift `VX` right 1 bit in place, storing the least significant bit 
// of `VX` in `VF` prior to the shift
//
// TODO: check if this is the correct implementation vs. the other specification
void Chip8::shr_vx() {
    vf() = bit_at(vx(), 0);
    vx() >>= 1;
}

// 0x8XY7 - set `VX` to `VY` - `VX`; if `Vy` > `Vx`, then `VF` 1, otherwise 0
// NOTE: this is very similar to `sub_vx_vy`, but the order is flipped
void Chip8::subn_vx_vy() {
    vf() = vy() > vx();
    vx() = vy() - vx();
}

// 0x8XYE - shift `VX` left 1 bit in place, storing the most significant bit 
// of `VX` in `VF` prior to the shift
//
// TODO: check if this is the correct implementation vs. the other specification
void Chip8::shl_vx() {
    vf() = bit_at(vx(), 7);
    vx() <<= 1;
}

// 0x9XY0 - skip the next instruction if `VX` != `VY`
void Chip8::sne_vx_vy() {
    skip_if(vx() != vy());
}

// 0xANNN - set `I` to address 0x0NNN.
void Chip8::ld_i_nnn() {
    index = extract_nnn();
}

// 0xBNNN - jump to address 0x0NNN + `V0`.
void Chip8::jp_v0_nnn() {
    pc = registers[0] + extract_nnn();
}

// 0xCXNN - set `VX` to a random number with a mask of 0xNN 
void Chip8::rnd_vx_nn() {
    // NOTE: we don't really care about the quality of our random integers,
    // so using c-style stuff is fine
    vx() = (std::rand() % UINT8_MAX) & extract_nn();
}

// 0xDXYN - draw a sprite starting at (`VX`, `VY`) with the bytes from `I` to
// `I` + 0x0N; if any set pixels are unset, set `VF` to 1 (otherwise, 0)
// NOTE: (0, 0) is at the top-left, +x goes right, +y goes down, the most
// significant bits are at lower x-values, and lower memory addresses are
// at lower y-values
void Chip8::drw_vx_vy_n() {
    uint8_t x = vx();
    uint8_t y = vy();
    uint8_t n = extract_n();

    vf() = 0;

    for (auto dy = 0; dy < n; ++dy) {
        uint8_t byte = memory.at(index + dy);

        for (auto dx = 0; dx < 8; ++dx) {
            if (bit_at(byte, 7 - dx)) { 
                vf() = screen.draw(x + dx, y + dy);
            }
        }
    }
}

// 0xEX9E - skip the next instruction if the key corresponding to `VX` is
// pressed
void Chip8::skp_vx() {
    skip_if(keys_pressed.at(vx()));
}

// 0xEXA1 - skip the next instruction if the key corresponding to `VX` is not
// pressed
void Chip8::sknp_vx() {
    skip_if(!keys_pressed.at(vx()));
}

// 0xFX07 - store the delay timer in `VX`
void Chip8::ld_vx_dt() {
    vx() = delay_timer;
}

// 0xFX0A - wait for a keypress, then store the key in `VX`
void Chip8::ld_vx_k() {
    auto it = std::find(keys_pressed.begin(), keys_pressed.end(), true);

    if (it != keys_pressed.end()) {
        vx() = std::distance(keys_pressed.begin(), it);
    } else {
        // repeat this instruction again until we actually get input
        decrement_pc();
    }
}

// 0xFX15 - set the delay timer to `VX`
void Chip8::ld_dt_vx() {
    delay_timer = vx();
}

// 0xFX18 - set the sound timer to `VX`
void Chip8::ld_st_vx() {
    sound_timer = vx();
}

// 0xFX1E - increment `I` by `VX`
void Chip8::add_i_vx() {
    index += vx();
}

// 0xFX29 - set `I` to the address of the sprite of the hexadecimal digit
// stored in `VX`
void Chip8::ld_f_vx() {
    index = vx() * FONT_STRIDE + FONT_ADDRESS; 
}

// 0xFX33 - store the base 10 representation of `VX` at `I`, `I` + 1, 
// and `I` + 2 (hundreds, tens, ones)
void Chip8::ld_b_vx() {
    // NOTE: we don't mod 10 for the hundreds place since UINT8_MAX < 1000
    // digit_at(vx(), 0);
    memory.at(index) = vx() / 100;     
    memory.at(index + 1) = vx() / 10 % 10;
    memory.at(index + 2) = vx() % 10;
}

// 0xFX55 - dump the values of `V0` to `VX` (inclusive) into memory at `I`
void Chip8::ld_mem_vx() {
    uint8_t x = extract_x();

    // NOTE: explicitly check bounds since `copy_n` doesn't 
    if (index + x >= memory.size()) {
        throw std::out_of_range("attempted to write outside memory");
    }

    std::copy_n(registers.begin(), x, memory.begin() + index);

    #ifdef INCREMENT_INDEX
    index += x + 1;
    #endif
}

// 0xFX55 - read memory at `I` into `V0` to `VX` (inclusive)
void Chip8::ld_vx_mem() {
    uint8_t x = extract_x();

    // NOTE: explicitly check bounds since `copy_n` doesn't
    if (index + x >= memory.size()) {
        throw std::out_of_range("attempted to read outside memory");
    }

    std::copy_n(memory.begin() + index, x, registers.begin());

    #ifdef INCREMENT_INDEX
    index += x + 1;
    #endif
}
