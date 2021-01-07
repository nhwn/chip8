#include <cstdint>
#include <fstream>
#include <array>
#include <string_view>
#include <random>
#include "screen.h"
#include "stack.h"

#ifndef CHIP8_H
#define CHIP8_H

constexpr auto START_ADDRESS = 0x200;

class Chip8 {
public:
    Chip8();
    void cycle();
    void load_rom(const std::string_view filename);
    void reset();
    Screen<64, 32> screen{};
    std::array<bool, 16> keys_pressed = {};
private:
    std::array<uint8_t, 4096> memory = {};
    std::array<uint8_t, 16> registers = {};
    Stack<16> stack{};

    uint16_t index = 0;
    uint16_t pc = START_ADDRESS;
    uint8_t sound_timer = 0;
    uint8_t delay_timer = 0;
    uint16_t instruction = 0;
    
    // helpers

    void fetch_instruction();
    void increment_pc();
    void decrement_pc();
    void execute_instruction();
    void decrement_timers();
    uint16_t extract_nnn();
    uint8_t extract_nn();
    uint8_t extract_n();
    uint8_t extract_x();
    uint8_t extract_y();
    uint8_t& vx();
    uint8_t& vy();
    uint8_t& vf();
    void skip_if(const bool condition);
    void illegal();

    // instructions

    void cls();
    void ret();
    void jp_nnn();
    void call_nnn();
    void se_vx_nn();
    void sne_vx_nn();
    void se_vx_vy();
    void ld_vx_nn();
    void add_vx_nn();
    void ld_vx_vy();
    void or_vx_vy();
    void and_vx_vy();
    void xor_vx_vy();
    void add_vx_vy();
    void sub_vx_vy();
    void shr_vx();
    void subn_vx_vy();
    void shl_vx();
    void sne_vx_vy();
    void ld_i_nnn();
    void jp_v0_nnn();
    void rnd_vx_nn();
    void drw_vx_vy_n();
    void skp_vx();
    void sknp_vx();
    void ld_vx_dt();
    void ld_vx_k();
    void ld_dt_vx();
    void ld_st_vx();
    void add_i_vx();
    void ld_f_vx();
    void ld_b_vx();
    void ld_mem_vx();
    void ld_vx_mem();
};

#endif
