#include "runtime.h"

#include <cstring>
#include <cstdarg>
#include <iostream>
#include <math.h>

#include "processor.h"


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


// Emit a push_ref instruction for a symbol with name `name`.
inline void Runtime::emit_push_ref(std::string &name) {
    // Figure out this ref's index. Start at the current environment
    // and go back up the stack, looking for the name. If not found,
    // error out.
    int var_index = -1;
    for (int i = scopes.size() - 1; i >= 0; i--) {
        auto& var_indices = scopes[i].var_indices;
        if (var_indices.count(name) > 0) {
            var_index = var_indices[name];
            break;
        }
    }

    if (var_index < 0) {
        // TODO: fail here.
        exit(-1);
    }

    mem_put<Instruction>(Instruction::PushRef, proc.write_head());
    proc.write_offset += sizeof(Instruction);
    mem_put<int>(var_index, proc.write_head());
    proc.write_offset += sizeof(int);
}


void Runtime::emit_push(std::unique_ptr<AST>& ast) {
    switch (ast->type) {
    case ASTType::IntLiteral:
        emit_push_int(std::stoi(ast->token->string_value));
        break;
    case ASTType::Symbol:
        emit_push_ref(ast->token->string_value);
        break;
    default:
        break;
    }
}

void Runtime::emit_expr(std::unique_ptr<AST>& ast) {

    if (ast->children.size() == 0) {
        if (ast->type != ASTType::Expr)
            emit_push(ast);
        
        // If the AST is an expression but has 0 children, we won't
        // emit any code for it at all.
        return;
    }
    
    auto& first = ast->children[0]->token->string_value;
    if (first == "def") emit_def(ast);
    else if (first == "do") emit_do(ast);
    else if (first == "if") emit_if(ast);
    else if (first == "fn") emit_fn(ast);
    else emit_call(ast);
}


// Emit a function call.
void Runtime::emit_call(std::unique_ptr<AST>& ast) {

    std::string& fn_name = ast->children[0]->token->string_value;

    const int n_args = ast->children.size() - 1;
    bool is_builtin = builtins.count(fn_name) > 0;
    long old_woff = proc.write_offset;

    // First, add all the operands to the stack:
    for (int i = 1; i < ast->children.size(); i++) {
        auto &child = ast->children[i];
        if (child->type == ASTType::Expr)
            emit_expr(child);
        else {
            emit_push(child);
        }
    }


    // Builtin functions have their own entry in builtins.
    if (is_builtin) {

        // Won't insert a new element because we know the entry exists
        // already.

        BuiltinEntry entry = builtins.find(fn_name)->second;
        
        // Check special case for "-":
        if (fn_name == "-") {
            if (n_args == 1) {
                mem_put<Instruction>(Instruction::Neg,
                                     proc.write_head());

                proc.write_offset += sizeof(Instruction);
            }
            else if (n_args == 2) {
                mem_put<Instruction>(Instruction::Sub,
                                     proc.write_head());
                proc.write_offset += sizeof(Instruction);
            }
            else {
                // TODO: error here
            }
            return;
        }

        // Handle variadic arguments:
        if (entry.n_args == -1) {
            mem_put<Instruction>(entry.inst, proc.write_head());
            proc.write_offset += sizeof(Instruction);

            mem_put<int>(n_args, proc.write_head());
            proc.write_offset += sizeof(int);
            return;
            
        }

        // Check if the program supplied a correct number of
        // arguments:
        if (entry.n_args != n_args) {

            // TODO: make this an actual error function.
            printf("Error: '%s' expects %d arguments, got %d\n",
                   fn_name.c_str(),
                   entry.n_args,
                   n_args);
            exit(-1);
        }

        // Insert function opcode into the instruction array:
        mem_put<Instruction>(entry.inst, proc.write_head());
        proc.write_offset += sizeof(Instruction);     
    }

    // Not a builtin function; look it up and insert a call
    // instruction.
    else {
        // Get the function's index and instruction pointer:
        int var_index = -1;
        
        for (int i = scopes.size() - 1; i >= 0; i--) {
            if (scopes[i].var_indices.count(fn_name) > 0) {
                var_index = scopes[i].var_indices.find(fn_name)->second;
                break;
            }
        }
        
        if (var_index == -1) {
            printf("Undefined function '%s'\n", fn_name.c_str());
            exit(-1);
        }

        // TODO: Check if correct number of arguments is supplied        
        mem_put<Instruction>(Instruction::Call, proc.write_head());
        proc.write_offset += sizeof(Instruction);
        mem_put<int>(var_index, proc.write_head());
        proc.write_offset += sizeof(int);
    }
}

void Runtime::emit_do(std::unique_ptr<AST>& ast) {
    for (int i = 1; i < ast->children.size(); i++) {
        emit_expr(ast->children[i]);
    }
}

void Runtime::emit_if(std::unique_ptr<AST>& ast) {
    // Structure of an if statement in bytecode:
    //
    // /* bytecode for condition expr */
    // jmp_false to after bytecode in if block
    // / *bytecode for if block */
    // jmp to after bytecode in else block
    // /*bytecode for else block */

    auto& condition = ast->children[1];
    auto& if_part = ast->children[2];
    auto& else_part = ast->children[3];

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

    // Add number of bytes emitted between else_woff and old_woff as
    // an argument to jmp_false.
    mem_put<int>(else_woff - old_woff,
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

    // TODO: I feel like this is broken. Why is this not an off by 1
    // error?
    mem_put<int>(proc.write_offset - old_woff,
                 proc.instructions + old_woff + sizeof(Instruction));
}

void Runtime::emit_def(std::unique_ptr<AST>& ast) {
    // Leftmost child is always the symbol name
    // TODO: error handling here, like for having too many child nodes
    auto& left = ast->children[1];
    auto& right = ast->children[2];
    std::string symbol_name = left->token->string_value;
    
    // Look for symbol in this environment
    if (scopes.back().var_indices.count(symbol_name) > 0) {
        // TODO: Symbol is already defined in this environent; fail.
    }
    int var_number = var_counter;

    // We will consider the symbol "defined" before we even figure out
    // what the symbol is bound to. This avoids bugs when parsing
    // recursive functions.
    scopes.back().var_indices[symbol_name] = var_number;
    
    var_counter++;
    
    emit_expr(right);
    
    mem_put<Instruction>(Instruction::Store, proc.write_head());
    proc.write_offset += sizeof(Instruction);

    mem_put<int>(var_number, proc.write_head());
    proc.write_offset += sizeof(int);
}

void Runtime::emit_fn(std::unique_ptr<AST>& ast) {
    
    // Bytecode for emitting an fn:
    // jmp to create_closure instruction
    // load arg1 <- actual closure code begins here
    // load arg2
    // ...exprs...
    // ret
    // create_closure -- copies the entire current environment (NOT
    // the pointer!), makes a shared_ptr to it, and creates a closure
    // that possesses the shared_Ptr
    
    // This creates a new scope.
    scopes.push_back(Scope());

    auto& param_list = ast->children[1];

    long old_woff = proc.write_offset;

    // Allocate space for jump instruction
    proc.write_offset += sizeof(Instruction) + sizeof(int);
        
    // It is assumed that when we make a function call, all the
    // arguments are pushed onto the stack before the jump. Here we
    // emit store instructions for those arguments and store their
    // values into the new environment.
    for (int i = param_list->children.size() - 1; i >= 0; i--) {
        auto& child = param_list->children[i];
        
        // TODO: better error handling here
        if (child->type != ASTType::Symbol) {
            printf("Parameter %d must be a symbol\n", i+1);
            exit(-1);
        }

        std::string &param_name = child->token->string_value;
        scopes.back().var_indices[param_name] = var_counter;
        
        mem_put<Instruction>(Instruction::Store, proc.write_head());
        proc.write_offset += sizeof(Instruction);
        
        mem_put<int>(var_counter, proc.write_head());
        proc.write_offset += sizeof(int);
        
        var_counter++;
    }
    
    // Now go through the rest of the expressions in the function and
    // emit bytecode for them.
    for (int i = 2; i < ast->children.size(); i++)
        emit_expr(ast->children[i]);
    
    // Lastly, emit the ret instruction:
    mem_put<Instruction>(Instruction::Ret, proc.write_head());
    proc.write_offset += sizeof(Instruction);

    mem_put<Instruction>(Instruction::CreateClosure,
                         proc.write_head());
    proc.write_offset += sizeof(Instruction);

    int code_beg_woff = old_woff + sizeof(Instruction)
        + sizeof(int);

    int code_end_woff = proc.write_offset - sizeof(Instruction);
    
    mem_put<int>(code_end_woff - code_beg_woff,
                 proc.write_head());
    
    proc.write_offset += sizeof(int);
    
    // Now go back to the beginning and add the jmp that skips over
    // the function body.
    mem_put<Instruction>(Instruction::Jmp,
                         proc.instructions + old_woff);
    
    mem_put<int>(proc.write_offset
                 - old_woff
                 - sizeof(Instruction)
                 - sizeof(int),
                 proc.instructions + old_woff + sizeof(Instruction));
    
    // Destroy current scope:
    scopes.pop_back();
}

std::shared_ptr<Value> Runtime::eval_ast(std::unique_ptr<AST>& ast) {
    long long old_woff = proc.write_offset;
    emit_expr(ast);
    
    // TODO: Implement some kind of error flag that we can set during
    // bytecode generation. At this point, we should check the error
    // flag and potentially zero out all the bytecode we just
    // generated if we know it is invalid.

    // proc.print_instructions(old_woff);
    
    // Run until we hit a 0 byte
    //long inst_count = 0;
    
    try {
        while (*proc.ip) {
            unsigned char inst = *proc.ip;
            proc.jump_table[inst](proc);
        }
    }

    catch (...) {
        printf("RUNTIME ERROR\n");
        printf("Stack state:\n");
        for (int i = proc.stack.size() - 1; i >= 0; i--) {
            printf("%d ", proc.stack.back()->as<int>());
            if (i == proc.stack.size() - 1)
                printf("<-------- top of stack");

            printf("\n");
        }
    }

    //printf("Executed %ld instructions\n", inst_count);
    // proc.print_instructions();
    if (proc.stack.size() > 0) {
        auto result = proc.stack.back();
        proc.stack.clear();
        return result;
    }
    else {
        proc.stack.clear();

        // Default constructor initializes type to nil
        return std::make_shared<Value>();
    }
}
