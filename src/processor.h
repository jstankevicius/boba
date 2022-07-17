#pragma once

#include <vector>
#include <cstring>
#include "environment.h"

#define PROC_INSTRUCTION_SIZE 1024

enum class Instruction : unsigned char {

    // Pushing stuff onto the stack:
    PushInt = 1,
    PushStr,
    PushFloat,
    PushRef,
    
    PushTrue,
    PushFalse,
    
    PushNil,

    // Store:
    Store,

    // Jumps:
    Jmp, // Absolute jump
    JmpTrue,
    JmpFalse,
    Call,
    
    // Return:
    // NOTE: Ret is a special function because it's the only one that
    // doesn't operate on the top of the stack. Since ret is only
    // called after a function has finished execution (and the
    // function probably returns something, leaving its result on the
    // top of the stack), it needs access to the element directly
    // below the top of the stack (its return address).
    Ret,

    // Logic:
    Not,
    And,
    Or,

    // Comparison:
    Eq,
    Greater,
    GreaterEq,
    Less,
    LessEq,

    // Arithmetic:
    Add,
    Sub,
    Mul,
    Div,
    Neg,
};

struct Processor {
    // Instruction pointer.
    long ip = 0;

    // Instruction bytes.
    unsigned char instructions[PROC_INSTRUCTION_SIZE];

    // Offset into the instruction buffer.
    long write_offset = 0;

    // Program stack.
    std::vector<Value> stack;

    // Environment stack.
    std::vector<Environment> envs;

    // Table of functions to jump to on each instruction.
    void (*jump_table[256])(Processor &proc);

    // Stack methods:
    template <typename T>
    inline T pop_as();

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

    void print_instructions() {
        printf("===========================================\n");
        int i = 0;
        while (instructions[i] > 0) {
            auto inst = static_cast<Instruction>(instructions[i]);
            printf("%08x | %02x ", i, instructions[i]);
            i++;
            
            switch (inst) {
            case Instruction::PushInt:
            case Instruction::Store:
            case Instruction::Jmp:
            case Instruction::JmpTrue:
            case Instruction::JmpFalse:
            case Instruction::Call:
                i+=4;
                for (int j = i; j < i + 4; j++) {
                    printf("%02x ", instructions[j]);
                }
                break;
            default:
                break;
            }
            printf("\n");
        }
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
