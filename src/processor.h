#pragma once

#include <vector>
#include "environment.h"

#define PROC_INSTRUCTION_SIZE 1024

enum class Instruction : unsigned char {
    PushInt = 1,
    PushStr,
    PushFloat,
    PushRef,
    
    PushTrue,
    PushFalse,
    
    PushNil,
    
    Store,
    
    Add,
    Sub,
    Mul,
    Div,
    Neg,
};

struct Processor {
    // Instruction pointer.
    long long ip;

    // Instruction bytes.
    unsigned char instructions[PROC_INSTRUCTION_SIZE];

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

    inline unsigned char* write_head() {
        return instructions + write_offset;
    }

    inline unsigned char* inst_head() {
        return instructions + ip;
    }

    inline unsigned char cur_byte() {
        return instructions[ip];
    }
};

template <typename T>
inline void mem_put(T value, unsigned char* arr) {
    std::memcpy(arr, &value, sizeof(T));
}

template <typename T>
inline T mem_get(unsigned char* arr) {
    T value;
    std::memcpy(&value, arr, sizeof(T));
    return value;
}
