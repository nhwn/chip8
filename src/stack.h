#include <array>
#include <algorithm>

#ifndef CHIP8_STACK_H
#define CHIP8_STACK_H

template<size_t N>
class Stack {
public:
    uint16_t pop() {
        --stack_pointer;
        return stack.at(stack_pointer);
    }
    void push(const uint16_t address) {
        stack.at(stack_pointer) = address;
        ++stack_pointer;
    }
    void clear() {
        std::fill(stack.begin(), stack.end(), 0);
        stack_pointer = 0;
    }
private: 
    size_t stack_pointer = 0;
    std::array<uint16_t, N> stack = {};
};

#endif
