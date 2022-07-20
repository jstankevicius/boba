#include "processor.h"

#define INST_ENTRY(id, fun) (jump_table[(unsigned long) id] = fun)

void push_int(Processor &proc) {
    int i = mem_get<int>(proc.ip);
    proc.stack.push_back(Value(i));
    proc.ip += sizeof(int);
}

void push_ref(Processor &proc) {
    int var_index = mem_get<int>(proc.ip);
    proc.stack.push_back(proc.envs.back().memory[var_index]);
    proc.ip += sizeof(int);
}

void store(Processor &proc) {
    int var = mem_get<int>(proc.ip);
    proc.envs.back().memory[var] = proc.stack.back();
    proc.stack.pop_back();
    proc.ip += sizeof(int);
}

void jmp(Processor &proc) {
    int offset = mem_get<int>(proc.ip);
    proc.ip += offset;
}

void jmp_true(Processor &proc) {
    bool is_true = proc.pop_as<bool>();
    int offset = mem_get<int>(proc.ip);
    
    if (is_true) {
        proc.ip += offset;
        return;
    }

    proc.ip += sizeof(int);
}

void jmp_false(Processor &proc) {
    bool is_true = proc.pop_as<bool>();
    int offset = mem_get<int>(proc.ip);

    if (!is_true) {
        proc.ip += offset;
        return;
    }

    proc.ip += sizeof(int);
}

void call(Processor &proc) {

    // Push the ip after the call instruction onto the call stack:
    proc.call_stack.push_back(proc.ip
                              + sizeof(Instruction) + sizeof(int));

    // Get index of the function we're calling
    int var_index = mem_get<int>(proc.ip);

    // TODO: Don't look only in the global environment.
    // Get the function's ip:
    byte* ip = proc.envs[0].memory[var_index].as<byte*>();

    // Create a new environment:
    proc.envs.push_back(Environment());
    proc.ip = ip;
}

void ret(Processor &proc) {
    byte* ret_ip = proc.call_stack.back();
    proc.call_stack.pop_back();
    
    proc.ip = ret_ip;
    proc.envs.pop_back();
}

void add(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    printf("a=%d, b=%d\n", a, b);
    proc.stack.push_back(Value(b + a));
}

void sub(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(b - a));
}

void mul(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(a * b));
}

void div(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(b / a));
}

void neg(Processor &proc) {
    int a = proc.pop_as<int>();
    proc.stack.push_back(Value(-a));
}

void eq(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(a == b));
}

void greater(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(b > a));
}

void greater_eq(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(b >= a));
}

void less(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(b < a));
}

void less_eq(Processor &proc) {
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(Value(b <= a));
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
    INST_ENTRY(Instruction::Call, call);
    INST_ENTRY(Instruction::Ret, ret);
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
