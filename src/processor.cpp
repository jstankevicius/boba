#include "processor.h"

#define INST_ENTRY(id, fun) (jump_table[static_cast<unsigned long>(id)] = fun)

void push_int(Processor &proc) {
    int i = mem_get<int>(proc.inst_head());
    proc.stack.push_back(Value(i));
    proc.ip += sizeof(int);

    printf("push_int %d\n", i);
}

void push_ref(Processor &proc) {
    int var_index = mem_get<int>(proc.inst_head());
    proc.stack.push_back(proc.envs.back().memory[var_index]);
    proc.ip += sizeof(int);
    
    printf("push_ref %d\n", var_index);
}

void store(Processor &proc) {
    int var = mem_get<int>(proc.inst_head());
    proc.envs.back().memory[var] = proc.stack.back();
    proc.stack.pop_back();
    proc.ip += sizeof(int);

    printf("store %d\n", var);
}

void jmp(Processor &proc) {
    int ip = mem_get<int>(proc.inst_head());
    proc.ip = ip;

    printf("jmp %d\n", ip);
}

void jmp_true(Processor &proc) {
    bool is_true = proc.pop_as<bool>();
    int ip = mem_get<int>(proc.inst_head());
    
    proc.ip += sizeof(int);
    
    if (is_true) {
        printf("jmp_true %d\n", ip);
        proc.ip = ip;
    }
}

void jmp_false(Processor &proc) {
    bool is_true = proc.pop_as<bool>();
    int ip = mem_get<int>(proc.inst_head());

    proc.ip += sizeof(int);
    if (!is_true) {
        printf("jmp_false %d\n", ip);
        proc.ip = ip;
    }
}

void add(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(b + a));

    printf("add\n");
}

void sub(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(b - a));

    printf("add\n");
}

void mul(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(a * b));

    printf("add\n");
}

void div(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(b / a));

    printf("add\n");
}

void neg(Processor &proc) {
    int a = proc.pop_as<int>();
    proc.stack.push_back(Value(-a));

    printf("neg\n");
}

void eq(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(a == b));

    printf("eq\n");
}

void greater(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(b > a));

    printf("greater\n");
}

void greater_eq(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(b >= a));

    printf("greater_eq\n");
}

void less(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(b < a));

    printf("less\n");
}

void less_eq(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(b <= a));

    printf("less_eq\n");
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
    INST_ENTRY(Instruction::Jmp, jmp);
    INST_ENTRY(Instruction::JmpTrue, jmp_true);
    INST_ENTRY(Instruction::JmpFalse, jmp_false);
    INST_ENTRY(Instruction::Eq, eq);
    INST_ENTRY(Instruction::Greater, greater);
    INST_ENTRY(Instruction::GreaterEq, greater_eq);
    INST_ENTRY(Instruction::Less, less);
    INST_ENTRY(Instruction::LessEq, less_eq);
}


template <typename T>
inline T Processor::pop_as() {
    T a = stack.back().as<T>();
    stack.pop_back();
    return a;
}
