#include "runtime.h"

#include <cstdarg>
#include <iostream>
#include <math.h>

#include "bytecode.h"
#include "processor.h"

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


// Relative emit - emits an int exactly at write_offset, then
// advances write_offset.
inline void Runtime::emit_push_int(int i) {

    // For now, we'll actually only consider one size. It may be useful down
    // the line to break up push_i into a push_i where the arguments are
    // different sizes to save space. Right now we have a lot of empty bytes
    // in the instruction buffer.

    mem_put<Instruction>(Instruction::PushInt, proc.instructions + proc.write_offset);
    proc.write_offset += sizeof(unsigned char);

    mem_put<int>(i, proc.instructions + proc.write_offset);
    proc.write_offset += sizeof(int);
}


void Runtime::emit_push(std::shared_ptr<AST> ast) {
    assert(ast->children.size() == 0);
    switch (ast->type) {
    case ASTType::IntLiteral:
        emit_push_int(std::stoi(ast->string_value));
        break;
    default:
        break;
    }
}


// Emit an "expression" (a function call).
void Runtime::emit_expr(std::shared_ptr<AST> ast) {

    // First, add all the operands to the stack:
    for (auto& child : ast->children) {
        if (child->type == ASTType::Expr)
            emit_expr(child);
        else
            emit_push(child);
    }

    const std::string ast_val = ast->string_value;

    // TODO: make this more robust. We'll need to handle a lot more
    // cases of function calls. It'll also need to work on stuff like floats.

    if (ast_val == "+") {
        mem_put<Instruction>(Instruction::Add, proc.instructions + proc.write_offset);
        proc.write_offset += sizeof(unsigned char);
    }

    else if (ast_val == "-") {
        if (ast->children.size() == 1) {
            mem_put<Instruction>(Instruction::Neg, proc.instructions + proc.write_offset);
            proc.write_offset += sizeof(unsigned char);
        }

        else {
            mem_put<Instruction>(Instruction::Sub, proc.instructions + proc.write_offset);
            proc.write_offset += sizeof(unsigned char);
        }
    }

    else if (ast_val == "*") {
        mem_put<Instruction>(Instruction::Mul, proc.instructions + proc.write_offset);
        proc.write_offset += sizeof(unsigned char);
    }

    else if (ast_val == "/") {
        mem_put<Instruction>(Instruction::Div, proc.instructions + proc.write_offset);
        proc.write_offset += sizeof(unsigned char);
    }
}

void Runtime::emit_def(std::shared_ptr<AST>) {

}

void Runtime::eval_ast(std::shared_ptr<AST> ast) {

    long long old_woff = proc.write_offset;

    // Determine the type of AST we're evaluating.
    if (ast->string_value == "def") {
        emit_def(ast);
    }

    // For now, everything that isn't a def will just be assumed to be
    // an arithmetic expression. We'll need to start handling defuns soon.
    else {
        emit_expr(ast);
    }

    // Run until we hit a 0 byte
    while (proc.instructions[proc.ip]) {
        unsigned char inst = proc.instructions[proc.ip];

        // Normally we'd expect the instruction pointer to be incremented after
        // the instruction is executed. Doing it this way lets the jumped-to
        // function immediately read any arguments from memory without having to
        // increment ip.
        proc.ip++;
        proc.jump_table[inst](proc);
    }

    printf("Instruction size: %lld bytes\n", proc.write_offset - old_woff);
    printf("Result: %d\n", proc.stack.back().as<int>());
}
