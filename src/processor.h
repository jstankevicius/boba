#pragma once

#include <vector>

#include "bytecode.h"
#include "environment.h"

struct Processor {
    // Instruction pointer.
    long long ip;

    // Instruction bytes.
    unsigned char instructions[1024];

    // Offset into the instruction buffer.
    long long write_offset = 0;

    // Program stack.
    std::vector<Value> stack;

    // Environment stack.
    std::vector<Environment> envs;

    // Table of functions to jump to on each instruction.
    void (*jump_table[256])(Processor &proc);

    // Stack methods:
    template <typename T>
    inline void pop_as(T &a);

    template <typename T>
    inline void pop2_as(T &a, T &b);

    Processor();
};
