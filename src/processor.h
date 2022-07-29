#pragma once

#include <cstring>
#include <memory>
#include <vector>

#include "environment.h"

#define PROC_INSTRUCTION_SIZE 1 << 16

enum class Instruction : unsigned char
{
    // Pushing stuff onto the stack:
    PushInt = 1,
    PushStr = 2,
    PushFloat = 3,
    PushRef = 4,
    
    PushTrue = 5,
    PushFalse = 6,
    
    PushNil = 7,

    // Store:
    Store = 8,

    // Relative jumps:
    Jmp = 9,
    JmpTrue = 10,
    JmpFalse = 11,
    
    Call = 12,
    CreateClosure = 13,
    
    Ret = 14,

    // Logic:
    Not = 15,
    And = 16,
    Or = 17,

    // Comparison:
    Eq = 18,
    Greater = 19,
    GreaterEq = 20,
    Less = 21,
    LessEq = 22,

    // Arithmetic:
    Add = 23,
    Sub = 24,
    Mul = 25,
    Div = 26,
    Neg = 27,
};

struct Processor
{
    // Instruction bytes.
    unsigned char instructions[PROC_INSTRUCTION_SIZE];

    // Instruction pointer.
    unsigned char* ip = instructions;

    // Pointer to where the next instruction will be emitted.
    unsigned char* write_head = instructions;

    // Program stack.
    // TODO: Rename to value_stack or something.
    std::vector<std::shared_ptr<Value>> stack;

    // Environment stack.
    std::vector<std::unordered_map<int, std::shared_ptr<Value>>> envs;

    // All our instruction pointers are put here before we jump to
    // another function.
    std::vector<unsigned char*> call_stack;

    // Table of functions to jump to on each instruction.
    void (*jump_table[256])(Processor &proc);

    // Stack methods:
    template <typename T> inline T pop_as();

    Processor();
};

template <typename T>
inline void mem_put(T value, unsigned char* arr)
{
    std::memcpy(arr, &value, sizeof(T));
}

template <typename T>
inline T mem_get(unsigned char* arr)
{
    T value;
    std::memcpy(&value, arr, sizeof(T));
    return value;
}
