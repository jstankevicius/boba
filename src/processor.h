#pragma once

#include <vector>
#include <cstring>
#include "environment.h"

#define PROC_INSTRUCTION_SIZE 1024

using byte = unsigned char;

enum class Instruction : byte {

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

    // Relative jumps:
    Jmp,
    JmpTrue,
    JmpFalse,

    // Load a closure from the current environment and set the
    // processor ip to the closure's ip.
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

    // Instruction bytes.
    byte instructions[PROC_INSTRUCTION_SIZE];

    // Instruction pointer.
    byte* ip = instructions;

    // Offset into the instruction buffer.
    long write_offset = 0;

    // Program stack.
    std::vector<Value> stack;

    // Environment stack.
    std::vector<Environment> envs;

    std::vector<unsigned char*> call_stack;

    // Table of functions to jump to on each instruction.
    void (*jump_table[256])(Processor &proc);

    // Stack methods:
    template <typename T>
    inline T pop_as();

    Processor();

    inline byte* write_head() {
        return instructions + write_offset;
    }

    void print_instructions() {
        printf("===========================================\n");
        int i = 0;
        
        while (instructions[i] > 0) {
            int int_val;

            // Eagerly copy instructions[i+1] to int_val. The value
            // might not make sense, but we'll only use it if we know
            // the value is an integer anyway.
            std::memcpy(&int_val, &instructions[i + 1], sizeof(int));
            
            auto inst = static_cast<Instruction>(instructions[i]);
            
            printf("%p | ", &instructions[i]);
            
            switch (inst) {
            case Instruction::PushInt:
                printf("push_int %d\n", int_val);
                i += sizeof(Instruction) + sizeof(int);
                break;

            case Instruction::PushRef:
                printf("push_ref %d\n", int_val);
                i += sizeof(Instruction) + sizeof(int);
                break;

            case Instruction::Store:
                printf("store %d\n", int_val);
                i += sizeof(Instruction) + sizeof(int);
                break;
                
            case Instruction::Jmp:
                if (int_val >= 0)
                    printf("jmp +%d (%p)\n", int_val, &instructions[i + int_val]);
                else
                    printf("jmp %d (%p)\n", int_val, &instructions[i + int_val]);
                i += sizeof(Instruction) + sizeof(int);
                break;
                
            case Instruction::JmpTrue:
                if (int_val >= 0)
                    printf("jmp_true +%d (%p)\n", int_val, &instructions[i + int_val]);
                else
                    printf("jmp_true %d (%p)\n", int_val, &instructions[i + int_val]);
                i += sizeof(Instruction) + sizeof(int);
                break;
                
            case Instruction::JmpFalse:
                if (int_val >= 0)
                    printf("jmp_false +%d (%p)\n", int_val, &instructions[i + int_val]);
                else
                    printf("jmp_false %d (%p)\n", int_val, &instructions[i + int_val]);
                i += sizeof(Instruction) + sizeof(int);
                break;
                
            case Instruction::Call:
                printf("call %d\n", int_val);
                i += sizeof(Instruction) + sizeof(int);
                break;

            case Instruction::Ret:
                printf("ret\n");
                i++;
                break;

            case Instruction::Eq:
                printf("eq\n");
                i++;
                break;

            case Instruction::Greater:
                printf("greater\n");
                i++;
                break;

            case Instruction::GreaterEq:
                printf("greater_eq\n");
                i++;
                break;

            case Instruction::Less:
                printf("less\n");
                i++;
                break;

            case Instruction::LessEq:
                printf("less_eq\n");
                i++;
                break;

            case Instruction::Add:
                printf("add\n");
                i++;
                break;

            case Instruction::Sub:
                printf("sub\n");
                i++;
                break;

            case Instruction::Mul:
                printf("mul\n");
                i++;
                break;

            case Instruction::Div:
                printf("div\n");
                i++;
                break;

            case Instruction::Neg:
                printf("neg\n");
                i++;
                break;
                
                
            default:
                printf("%02x\n", instructions[i]);
                i++;
                break;
            }
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
