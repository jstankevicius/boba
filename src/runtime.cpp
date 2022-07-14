#include "runtime.h"

#include <iostream>
#include <math.h>

#include "bytecode.h"

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

    mem_put<Instruction>(Instruction::PUSH_I, instructions + write_offset);
    write_offset += sizeof(unsigned char);

    mem_put<int>(i, instructions + write_offset);
    write_offset += sizeof(int);

}


void Runtime::emit_push(std::shared_ptr<AST> ast) {
    assert(ast->children.size() == 0);
    switch (ast->type) {
    case ASTType::INT_LITERAL:
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
        if (child->type == ASTType::EXPR)
             emit_expr(child);
        else
            emit_push(child);
    }

    const std::string ast_val = ast->string_value;

    // TODO: make this more robust. We'll need to handle a lot more
    // cases of function calls. It'll also need to work on stuff like floats.

    if (ast_val == "+") {
        mem_put<Instruction>(Instruction::ADD_I, instructions + write_offset);
        write_offset += sizeof(unsigned char);
    }

    else if (ast_val == "-") {
        if (ast->children.size() == 1) {
            mem_put<Instruction>(Instruction::NEG_I, instructions + write_offset);
            write_offset += sizeof(unsigned char);
        }

        else {
            mem_put<Instruction>(Instruction::SUB_I, instructions + write_offset);
            write_offset += sizeof(unsigned char);
        }
    }


    else if (ast_val == "*") {
        mem_put<Instruction>(Instruction::MUL_I, instructions + write_offset);
        write_offset += sizeof(unsigned char);
    }

    else if (ast_val == "/") {
        mem_put<Instruction>(Instruction::DIV_I, instructions + write_offset);
        write_offset += sizeof(unsigned char);
    }
}

void Runtime::emit_def(std::shared_ptr<AST> ast) {

}

void Runtime::eval_ast(std::shared_ptr<AST> ast) {
    //std::cout << "eval_ast" << std::endl;
    // Determine the type of AST we're evaluating.
    if (ast->string_value == "def") {
        emit_def(ast);
    }

    // For now, everything that isn't a def will just be assumed to be
    // an arithmetic expression. We'll need to start handling defuns soon.
    else {
        emit_expr(ast);
    }

    for (int i = 0; i < 1024; i++) {
        printf("%x ", instructions[i]);
    }
    printf("\n");
}
