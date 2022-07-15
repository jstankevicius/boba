#include "processor.h"

#define INST_ENTRY(id, fun) (jump_table[static_cast<unsigned long>(id)] = fun)

//------------------------------------------------------------------------------
// Instruction execution:
void push_int(Processor &proc) {
    int i = mem_get<int>(proc.inst_head());
    printf("push_int %d\n", i);
    proc.stack.push_back((Value){.type = ValueType::Int, .value = i});
    proc.ip += sizeof(int);
}

void push_ref(Processor &proc) {
    int var_index = mem_get<int>(proc.inst_head());
    printf("push_ref %d\n", var_index);
    proc.stack.push_back(proc.envs.back().memory[var_index]);
    proc.ip += sizeof(int);
}

void store(Processor &proc) {
    int var = mem_get<int>(proc.inst_head());
    printf("store %d\n", var);
    proc.envs.back().memory[var] = proc.stack.back();
    proc.stack.pop_back();
    proc.ip += sizeof(int);
}

void add(Processor &proc) {
    int a, b;
    printf("add\n");
    // TODO: typecheck here
    proc.pop2_as<int>(a, b);
    proc.stack.push_back((Value){.type = ValueType::Int, .value = a + b});
    printf("Result: %d\n", proc.stack.back().as<int>());
}

void sub(Processor &proc) {
    int a, b;
    printf("sub\n");
    // TODO: typecheck here
    proc.pop2_as<int>(a, b);
    proc.stack.push_back((Value){.type = ValueType::Int, .value = b - a});
}

void mul(Processor &proc) {
    int a, b;
    printf("mul\n");
    // TODO: typecheck here
    proc.pop2_as<int>(a, b);
    proc.stack.push_back((Value){.type = ValueType::Int, .value = a * b});
}

void div(Processor &proc) {
    int a, b;
    printf("div\n");
    proc.pop2_as<int>(a, b);
    proc.stack.push_back((Value){.type = ValueType::Int, .value = b / a});
}

void neg(Processor &proc) {
    int a;
    printf("neg\n");
    proc.pop_as<int>(a);
    proc.stack.push_back((Value){.type = ValueType::Int, .value = -a});
}

Processor::Processor() {

    // Initialize instruction table
    INST_ENTRY(Instruction::PushInt, push_int);
    INST_ENTRY(Instruction::PushRef, push_ref);
    INST_ENTRY(Instruction::Store, store);
    INST_ENTRY(Instruction::Add, add);
    INST_ENTRY(Instruction::Sub, sub);
    INST_ENTRY(Instruction::Mul, mul);
    INST_ENTRY(Instruction::Div, div);
    INST_ENTRY(Instruction::Neg, neg);
}


template <typename T>
inline void Processor::pop_as(T &a) {
    a = stack.back().as<T>();
    stack.pop_back();
}

template <typename T>
inline void Processor::pop2_as(T &a, T &b) {
    pop_as<T>(a);
    pop_as<T>(b);
}
