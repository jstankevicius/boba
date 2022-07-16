#include "runtime.h"

#include <cstdarg>
#include <iostream>
#include <math.h>

#include "processor.h"

#define BUILTIN_ENTRY

// Relative emit - emits an int exactly at write_offset, then advances
// write_offset.
inline void Runtime::emit_push_int(int i) {

    // For now, we'll actually only consider one size. It may be
    // useful down the line to break up push_i into a push_i where the
    // arguments are different sizes to save space. Right now we have
    // a lot of empty bytes in the instruction buffer.
    mem_put<Instruction>(Instruction::PushInt, proc.write_head());
    proc.write_offset += sizeof(unsigned char);

    mem_put<int>(i, proc.write_head());
    proc.write_offset += sizeof(int);
}


// Emite a push_ref instruction for a symbol with name `name`.
inline void Runtime::emit_push_ref(std::string &name) {
    // Figure out this ref's index. Start at the current environment
    // and go back up the stack, looking for the name. If not found,
    // error out.
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

void Runtime::emit_expr(std::shared_ptr<AST> ast) {


    if (ast->children.size() == 0) {
        if (ast->type == ASTType::Expr)
            return;
        else {
            emit_push(ast);
            return;
        }
    }
    auto& first = ast->children[0]->string_value;
    if (first == "def") {
        emit_def(ast);
    }

    else if (first == "if") {
        emit_if(ast);
    }

    // For now, everything that isn't a def will just be assumed to be
    // an arithmetic expression. We'll need to start handling defuns
    // soon.
    else {
        emit_function(ast);
    }
}


// Emit a function call.
void Runtime::emit_function(std::shared_ptr<AST> ast) {

    // First, add all the operands to the stack:
    for (int i = 1; i < ast->children.size(); i++) {
        auto &child = ast->children[i];
        if (child->type == ASTType::Expr)
            emit_expr(child);
        else {
            //            printf("ast child is not an expression\n");
            emit_push(child);
        }
    }

    const std::string& ast_val = ast->children[0]->string_value;

    // TODO: make this more robust. We'll need to handle a lot more
    // cases of function calls. It'll also need to work on stuff like
    // floats.  This needs to be expandable to more builtin functions.

    // Arithmetic:
    if (ast_val == "+")
        mem_put<Instruction>(Instruction::Add, proc.write_head());
    else if (ast_val == "-") {
        if (ast->children.size() == 2)
            mem_put<Instruction>(Instruction::Neg, proc.write_head());
        else
            mem_put<Instruction>(Instruction::Sub, proc.write_head());
    }
    else if (ast_val == "*")
        mem_put<Instruction>(Instruction::Mul, proc.write_head());
    else if (ast_val == "/")
        mem_put<Instruction>(Instruction::Div, proc.write_head());

    // Comparison:
    else if (ast_val == "=")
        mem_put<Instruction>(Instruction::Eq, proc.write_head());
    else if (ast_val == ">")
        mem_put<Instruction>(Instruction::Greater, proc.write_head());
    else if (ast_val == ">=")
        mem_put<Instruction>(Instruction::GreaterEq,
                             proc.write_head());
    else if (ast_val == "<")
        mem_put<Instruction>(Instruction::Less, proc.write_head());
    else if (ast_val == "<=")
        mem_put<Instruction>(Instruction::LessEq, proc.write_head());

    proc.write_offset += sizeof(Instruction);
}

void Runtime::emit_if(std::shared_ptr<AST> ast) {

    // Structure of an if statement in bytecode:
    //
    // /* bytecode for condition expr */
    // jmp_false to after bytecode in if block
    // / *bytecode for if block */
    // jmp to after bytecode in else block
    // /*bytecode for else block */

    auto condition = ast->children[1];
    auto if_part = ast->children[2];
    auto else_part = ast->children[3];

    // Emit bytecode for the condition:
    emit_expr(condition);
    
    // Save current write_offset and allocate space for the jmp_false
    // instruction that runs before the if block. We will later use
    // old_woff as an offset into proc.instructions to write the
    // jmp_false instruction.
    long old_woff = proc.write_offset;
    proc.write_offset += sizeof(Instruction);
    proc.write_offset += sizeof(int);

    // Emit if-part's bytecode.
    emit_expr(if_part);

    // else_woff is where the else bytecode will begin, accounting for
    // the additional jump instruction we're going to insert at the
    // end of the if block.
    long else_woff = proc.write_offset + sizeof(Instruction)
        + sizeof(int);
    
    // At old_woff, insert a JmpFalse with the address of the byte
    // after the if block.
    mem_put<Instruction>(Instruction::JmpFalse,
                         proc.instructions + old_woff);

    // Add else_woff as the argument to the JmpFalse.
    mem_put<int>(else_woff,
                 proc.instructions + old_woff + sizeof(Instruction));

    // Now save the write_offset again as old_woff. We're going to use
    // this to write the jmp instruction which jumps to the byte after
    // the else block.
    old_woff = proc.write_offset;

    // Move write_offset past our allocated space:
    proc.write_offset += sizeof(Instruction);
    proc.write_offset += sizeof(int);

    // Emit else-part's bytecode.
    emit_expr(else_part);

    // At old_woff (the end of the if block), insert an unconditional
    // jump to skip over the else block if it ever gets executed.
    mem_put<Instruction>(Instruction::Jmp,
                         proc.instructions + old_woff);
    
    mem_put<int>(proc.write_offset,
                 proc.instructions + old_woff + sizeof(Instruction));
}

void Runtime::emit_def(std::shared_ptr<AST> ast) {
    // Leftmost child is always the symbol name
    // TODO: error handling here, like for having too many child nodes
    auto left = ast->children[1];
    auto right = ast->children[2];
    std::string symbol_name = left->string_value;

    emit_expr(right);
    
    mem_put<Instruction>(Instruction::Store, proc.write_head());
    proc.write_offset += sizeof(Instruction);

    // Now figure out the variable number
    int var_number = var_counter;

    // Look for symbol in this environment
    if (proc.envs.back().var_indices.count(symbol_name) > 0) {
        // TODO: Symbol is already defined in this environent; fail.
    }

    proc.envs.back().var_indices[symbol_name] = var_number;
    mem_put<int>(var_counter, proc.write_head());
    var_counter++;
    proc.write_offset += sizeof(int);
}

void Runtime::eval_ast(std::shared_ptr<AST> ast) {
    long long old_woff = proc.write_offset;
    emit_expr(ast);

    for (int i = old_woff; i < proc.write_offset; i++) {
        printf("%d: %02x\n", i, proc.instructions[i]);
    }

    // Run until we hit a 0 byte
    while (proc.cur_byte()) {
        unsigned char inst = proc.cur_byte();
        // Normally we'd expect the instruction pointer to be
        // incremented after the instruction is executed. Doing it
        // this way lets the jumped-to function immediately read any
        // arguments from memory without having to increment ip.
        proc.ip++;
        proc.jump_table[inst](proc);
    }

    //printf("Instruction size: %lld bytes\n", proc.write_offset - old_woff);
    //if (proc.stack.size() > 0)
    //    printf("Stack top: %d\n", proc.stack.back().as<int>());
}
