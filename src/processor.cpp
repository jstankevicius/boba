#include "processor.h"

#define INST_ENTRY(id, fun) (jump_table[static_cast<unsigned long>(id)] = fun)

//------------------------------------------------------------------------------
// Instruction execution:
void push_i(Processor &proc) {
    int i;
    std::memcpy(&i, proc.instructions + proc.ip, sizeof(int));
    proc.stack.push_back((Value){.type = ValueType::Int, .value = i});
    proc.ip += sizeof(int);
}

void add(Processor &proc) {
    int a, b;

    // TODO: typecheck here
    proc.pop2_as<int>(a, b);
    proc.stack.push_back((Value){.type = ValueType::Int, .value = a + b});
}

void sub(Processor &proc) {
    int a, b;

    // TODO: typecheck here
    proc.pop2_as<int>(a, b);
    proc.stack.push_back((Value){.type = ValueType::Int, .value = b - a});
}

void mul(Processor &proc) {
    int a, b;

    // TODO: typecheck here
    proc.pop2_as<int>(a, b);
    proc.stack.push_back((Value){.type = ValueType::Int, .value = a * b});
}

void div(Processor &proc) {
    int a, b;

    proc.pop2_as<int>(a, b);
    proc.stack.push_back((Value){.type = ValueType::Int, .value = b / a});
}

void neg(Processor &proc) {
    int a;

    proc.pop_as<int>(a);
    proc.stack.push_back((Value){.type = ValueType::Int, .value = -a});
}

Processor::Processor() {
    INST_ENTRY(Instruction::PushInt, push_i);
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
