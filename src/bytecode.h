#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <optional>
#include <any>
#include <memory>

#include "ast.h"

enum class InstructionType {
    PUSH,
    ADD,
    SUB,
    MUL,
    DIV,
    STORE
};


enum class ValueType {
    INT,
    FLOAT,
    STRING,
    BOOL,
    SYMBOL, // TODO: rename. Maybe "ref" or something?
};


struct Value {
    ValueType type;
    std::any value;
};

template <typename T>
Value make_value(ValueType type, T val) {
    Value v;
    v.type = type;
    v.value = val;
    return v;
}

struct Instruction {
    InstructionType inst_type;
    std::optional<Value> value;

    Instruction(InstructionType inst_type) {
        this->inst_type = inst_type;
    }

    Instruction(InstructionType inst_type, Value value) {
        this->inst_type = inst_type;
        this->value = value;
    }
};

namespace bytecode {

    Instruction push(std::shared_ptr<AST> ast);
    Instruction add(int n_args);
    Instruction sub(int n_args);
    Instruction mul(int n_args);
    Instruction div();
    Instruction store(std::shared_ptr<AST> ast);
}

void print_value(Value &v);
void print_instructions(std::vector<Instruction> &instructions);

std::vector<Instruction> gen_bytecode(std::shared_ptr<AST> ast);