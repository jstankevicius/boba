#include "processor.h"

#include <memory>

#include "environment.h"

#define INST_ENTRY(id, fun) (jump_table[(unsigned long) id] = fun)

// Pushes an integer onto the stack.
void push_int(Processor &proc) {
    proc.ip += sizeof(Instruction);
    
    int i = mem_get<int>(proc.ip);
    proc.stack.push_back(std::make_shared<Value>(i));
    proc.ip += sizeof(int);
}

void push_ref(Processor &proc) {
    proc.ip += sizeof(Instruction);
    
    int var_index = mem_get<int>(proc.ip);
    proc.stack.push_back(proc.envs.back()[var_index]);
    proc.ip += sizeof(int);
}

void store(Processor &proc) {
    proc.ip += sizeof(Instruction);
    int var = mem_get<int>(proc.ip);

    auto value = proc.stack.back();
    
    // If we are storing a closure, then the closure also needs to
    // receive a copy of itself in its environment.
    if (value->type == ValueType::Closure) {
        auto closure = value->as<std::shared_ptr<Closure>>();
        closure->env[var] = value;
    }
    
    proc.envs.back()[var] = proc.stack.back();
    proc.stack.pop_back();
    proc.ip += sizeof(int);
}

void jmp(Processor &proc) {
    int offset = mem_get<int>(proc.ip + sizeof(Instruction));
    proc.ip += offset;
}

void jmp_true(Processor &proc) {
    bool is_true = proc.pop_as<bool>();
    int offset = mem_get<int>(proc.ip + sizeof(Instruction));
    
    if (is_true) {
        proc.ip += offset;
        return;
    }
    proc.ip += sizeof(Instruction) + sizeof(int);
}

void jmp_false(Processor &proc) {
    bool is_true = proc.pop_as<bool>();
    int offset = mem_get<int>(proc.ip + sizeof(Instruction));

    if (!is_true) {
        proc.ip += offset;
        return;
    }

    proc.ip += sizeof(Instruction) + sizeof(int);
}

void call(Processor &proc) {

    // Push the ip after the call instruction onto the call stack:
    proc.call_stack.push_back(proc.ip
                              + sizeof(Instruction)
                              + sizeof(int));

    // Move ip to the first byte of the argument
    proc.ip += sizeof(Instruction);
           
    // Get index of the function we're calling
    int var_index = mem_get<int>(proc.ip);

    if (proc.envs.back().find(var_index) == proc.envs.back().end()) {
        printf("No entry for %d in current environment\n", var_index);
        exit(-1);
    }
    
    auto closure = proc.envs.back()[var_index]->as<std::shared_ptr<Closure>>();
    
    // Create a new environment (invokes copy constructor):
    proc.envs.push_back(closure->env);
    proc.ip = closure->instructions;
}


void create_closure(Processor& proc) {

    // offset denotes how many bytes from the beginning of this
    // instruction the processor would have to jump backwards to get
    // to the first code byte in the closure. Essentially, offset
    // denotes the size of the bytecode in the closure.
    proc.ip += sizeof(Instruction);
    
    int offset = mem_get<int>(proc.ip);
    proc.ip += sizeof(int);
    
    unsigned char* code_begin = proc.ip - sizeof(Instruction)
        - sizeof(int) - offset;

    auto closure = std::make_shared<Closure>();
    std::memset(closure->instructions, 0, CLOSURE_INSTRUCTION_SIZE);
    std::memcpy(closure->instructions, code_begin, offset);
    
    closure->env = proc.envs.back();

    Value v;
    v.type = ValueType::Closure;
    v.value = closure;

    proc.stack.push_back(std::make_shared<Value>(v));
}

void ret(Processor &proc) {
    if (proc.call_stack.size() == 0) {
        printf("ERROR: No return address on call stack\n");
        exit(-1);
    }
    
    unsigned char* ret_ip = proc.call_stack.back();
    proc.call_stack.pop_back();
    
    proc.ip = ret_ip;
    proc.envs.pop_back();
}

void add(Processor &proc) {
    proc.ip += sizeof(Instruction);
    
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(std::make_shared<Value>(b + a));
}

void sub(Processor &proc) {
    proc.ip += sizeof(Instruction);
    
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(std::make_shared<Value>(b - a));
}

void mul(Processor &proc) {
    proc.ip += sizeof(Instruction);
    
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(std::make_shared<Value>(a * b));
}

void div(Processor &proc) {
    proc.ip += sizeof(Instruction);
    
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(std::make_shared<Value>(b / a));
}

void neg(Processor &proc) {
    proc.ip += sizeof(Instruction);
    
    int a = proc.pop_as<int>();
    proc.stack.push_back(std::make_shared<Value>(-a));
}

void eq(Processor &proc) {
    proc.ip += sizeof(Instruction);
    
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(std::make_shared<Value>(a == b));
}

void greater(Processor &proc) {
    proc.ip += sizeof(Instruction);
    
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(std::make_shared<Value>(b > a));
}

void greater_eq(Processor &proc) {
    proc.ip += sizeof(Instruction);
    
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(std::make_shared<Value>(b >= a));
}

void less(Processor &proc) {
    proc.ip += sizeof(Instruction);
    
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(std::make_shared<Value>(b < a));
}

void less_eq(Processor &proc) {
    proc.ip += sizeof(Instruction);
    
    int a = proc.pop_as<int>();
    int b = proc.pop_as<int>();
    proc.stack.push_back(std::make_shared<Value>(b <= a));
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
    INST_ENTRY(Instruction::CreateClosure, create_closure);
    INST_ENTRY(Instruction::Ret, ret);
    INST_ENTRY(Instruction::Eq, eq);
    INST_ENTRY(Instruction::Greater, greater);
    INST_ENTRY(Instruction::GreaterEq, greater_eq);
    INST_ENTRY(Instruction::Less, less);
    INST_ENTRY(Instruction::LessEq, less_eq);
}


template <typename T>
inline T Processor::pop_as() {
    T a = stack.back()->as<T>();
    stack.pop_back();
    return a;
}
