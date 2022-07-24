#pragma once

#include <cstring>
#include <memory>
#include <vector>

#include "environment.h"

#define PROC_INSTRUCTION_SIZE 1 << 16

enum class Instruction : unsigned char {

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

struct Processor {

    // Instruction bytes.
    unsigned char instructions[PROC_INSTRUCTION_SIZE];

    // Instruction pointer.
    unsigned char* ip = instructions;

    // Offset into the instruction buffer.
    unsigned int write_offset = 0;

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
    template <typename T>
    inline T pop_as();

    Processor();

    inline unsigned char* write_head() {
        return instructions + write_offset;
    }

    void print_instructions(int start) {
        printf("===========================================\n");
        
        for (unsigned int i = start; i < write_offset;) {
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
                if (int_val > 0)
                    printf("jmp +%d (%d @ %p)\n",
                           int_val,
                           instructions[i + int_val],
                           &instructions[i + int_val]);
                else
                    printf("jmp %d (%d @ %p)\n",
                           int_val,
                           instructions[i + int_val],
                           &instructions[i + int_val]);
                
                i += sizeof(Instruction) + sizeof(int);
                break;
                
            case Instruction::JmpTrue:
                if (int_val >= 0)
                    printf("jmp_true +%d (%d @ %p)\n",
                           int_val,
                           instructions[i + int_val],
                           &instructions[i + int_val]);
                else
                    printf("jmp_true %d (%d @ %p)\n",
                           int_val,
                           instructions[i + int_val],
                           &instructions[i + int_val]);
                
                i += sizeof(Instruction) + sizeof(int);
                break;
                
            case Instruction::JmpFalse:
                if (int_val >= 0)
                    printf("jmp_false +%d (%d @ %p)\n",
                           int_val,
                           instructions[i + int_val],
                           &instructions[i + int_val]);
                else
                    printf("jmp_false %d (%d @ %p)\n",
                           int_val,
                           instructions[i + int_val],
                           &instructions[i + int_val]);
                
                i += sizeof(Instruction) + sizeof(int);
                break;
                
            case Instruction::Call:
                printf("call %d\n", int_val);
                i += sizeof(Instruction) + sizeof(int);
                break;

            case Instruction::CreateClosure:
                printf("create_closure %d\n", int_val);
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
                printf("(unknown) %02x\n", instructions[i]);
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
