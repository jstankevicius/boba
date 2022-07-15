#include "runtime.h"

#include <cstdarg>
#include <iostream>
#include <math.h>

#include "processor.h"

// Relative emit - emits an int exactly at write_offset, then
// advances write_offset.
inline void Runtime::emit_push_int(int i) {

    // For now, we'll actually only consider one size. It may be useful down
    // the line to break up push_i into a push_i where the arguments are
    // different sizes to save space. Right now we have a lot of empty bytes
    // in the instruction buffer.
    mem_put<Instruction>(Instruction::PushInt, proc.write_head());
    proc.write_offset += sizeof(unsigned char);

    mem_put<int>(i, proc.write_head());
    proc.write_offset += sizeof(int);
}


// Emite a push_ref instruction for a symbol with name `name`.
inline void Runtime::emit_push_ref(std::string &name) {
    // Figure out this ref's index. Start at the current environment and go
    // back up the stack, looking for the name. If not found, error out.
    int var_index = -1;
    for (int i = proc.envs.size() - 1; i >= 0; i--) {
        auto& var_indices = proc.envs[i].var_indices;
        if (var_indices.count(name) > 0) {
            var_index = var_indices[name];
            break;
        }
    }

    if (var_index < 0) {
        // TODO: fail here.
        std::cout << "Error: undefined variable: " << name << std::endl;
        exit(-1);
    }

    mem_put<Instruction>(Instruction::PushRef, proc.write_head());
    proc.write_offset += sizeof(Instruction);
    mem_put<int>(var_index, proc.write_head());
    proc.write_offset += sizeof(int);
}


void Runtime::emit_push(std::shared_ptr<AST> ast) {
    assert(ast->children.size() == 0);
    switch (ast->type) {
    case ASTType::IntLiteral:
        emit_push_int(std::stoi(ast->string_value));
        break;
    case ASTType::Symbol:
        emit_push_ref(ast->string_value);
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
    // This needs to be expandable to more builtin functions.
    if (ast_val == "+")
        mem_put<Instruction>(Instruction::Add, proc.write_head());
    else if (ast_val == "-") {
        if (ast->children.size() == 1)
            mem_put<Instruction>(Instruction::Neg, proc.write_head());
        else
            mem_put<Instruction>(Instruction::Sub, proc.write_head());
    }
    else if (ast_val == "*")
        mem_put<Instruction>(Instruction::Mul, proc.write_head());
    else if (ast_val == "/")
        mem_put<Instruction>(Instruction::Div, proc.write_head());

    proc.write_offset += sizeof(Instruction);

}

void Runtime::emit_def(std::shared_ptr<AST> ast) {
    // Leftmost child is always the symbol name
    // TODO: error handling here, like for having too many child nodes
    auto left = ast->children[0];
    auto right = ast->children[1];
    std::string symbol_name = left->string_value;

    if (right->type == ASTType::Expr)
        emit_expr(right);

    else
        emit_push(right);

    mem_put<Instruction>(Instruction::Store, proc.write_head());
    proc.write_offset += sizeof(Instruction);

    // Now figure out the variable number
    int var_number = var_counter;

    // Look for symbol in this environment
    if (proc.envs.back().var_indices.count(symbol_name) > 0) {
        // Symbol is already defined in this environent; fail.
    }

    proc.envs.back().var_indices[symbol_name] = var_number;
    mem_put<int>(var_counter, proc.write_head());
    var_counter++;
    proc.write_offset += sizeof(int);
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
    while (proc.cur_byte()) {
        unsigned char inst = proc.cur_byte();
        // Normally we'd expect the instruction pointer to be incremented after
        // the instruction is executed. Doing it this way lets the jumped-to
        // function immediately read any arguments from memory without having to
        // increment ip.
        proc.ip++;
        proc.jump_table[inst](proc);
    }

    //printf("Instruction size: %lld bytes\n", proc.write_offset - old_woff);
}
