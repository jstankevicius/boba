#include "runtime.h"

#include <cstring>
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
        if (ast->type == ASTType::Expr)
            return;
        else {
            emit_push(ast);
            return;
        }
    }
    auto& first = ast->children[0]->token->string_value;
    if (first == "def") {
        emit_def(ast);
    }

    else if (first == "if") {
        emit_if(ast);
    }

    else if (first == "defn") {
        emit_defn(ast);
    }

    // For now, everything that isn't a def will just be assumed to be
    // an arithmetic expression. We'll need to start handling defuns
    // soon.
    else {
        emit_function(ast);
    }
}


// Emit a function call.
void Runtime::emit_function(std::unique_ptr<AST>& ast) {

    std::string& fn_name = ast->children[0]->token->string_value;
    const int n_args = ast->children.size() - 1;
    
    bool is_builtin = builtins.count(fn_name) > 0;
    long old_woff = proc.write_offset;

    // If the function is not built in, allocate space for the
    // instruction which pushes the current ip.
    if (!is_builtin) {
        proc.write_offset += sizeof(Instruction) + sizeof(int);
    }
    //First, add all the operands to the stack:
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
        int var_index = proc.envs.back().var_indices[fn_name];

        // TODO: Check if correct number of arguments is supplied

        // push arg1
        // push arg2
        // arg bytecode...
        // push 
        // call function
        // RETURN HERE

        mem_put<Instruction>(Instruction::Call, proc.write_head());
        proc.write_offset += sizeof(Instruction);
        mem_put<int>(var_index, proc.write_head());
        proc.write_offset += sizeof(int);

        // The "push ip" instruction: goes before all the arguments
        // are evaluated, and should point to the byte right after the
        // call instruction.
        mem_put<Instruction>(Instruction::PushInt,
                             proc.instructions + old_woff);
        
        mem_put<int>(proc.write_offset,
                     proc.instructions + old_woff + sizeof(Instruction));
       

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

void Runtime::emit_def(std::unique_ptr<AST>& ast) {
    // Leftmost child is always the symbol name
    // TODO: error handling here, like for having too many child nodes
    auto& left = ast->children[1];
    auto& right = ast->children[2];
    std::string symbol_name = left->token->string_value;

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

void Runtime::emit_defn(std::unique_ptr<AST>& ast) {
    // This creates a new environment.
    proc.envs.push_back(Environment());

    std::string &name = ast->children[1]->token->string_value;
    auto& param_list = ast->children[2];

    // Insert some bytecode instructions to store information about
    // the function. The first instruction stores the instruction
    // pointer of this function in the processor's memory. The second
    // stores the number of arguments this instruction takes in the
    // next entry in memory. The third is a jump instruction that
    // allows us to jump past the function definition without
    // executing it.
    long old_woff = proc.write_offset;
    long jmp_woff = proc.write_offset
        + 4*(sizeof(Instruction) + sizeof(int));

    mem_put<Instruction>(Instruction::PushInt, proc.write_head());
    proc.write_offset += sizeof(Instruction);

    // The ip to which we jump to will be exactly 6 instructions ahead
    // of old_woff.
    mem_put<int>(old_woff + 5*(sizeof(Instruction) + sizeof(int)),
                 proc.write_head());

    // Store reference to this function in outer environment:
    proc.envs[proc.envs.size() - 1].var_indices[name] = var_counter;
    proc.write_offset += sizeof(int);
    
    // Now store the ip in memory:
    mem_put<Instruction>(Instruction::Store, proc.write_head());
    proc.write_offset += sizeof(Instruction);
    mem_put<int>(var_counter, proc.write_head());
    proc.write_offset += sizeof(int);
    var_counter++;
    
    // Now push the number of arguments:
    mem_put<Instruction>(Instruction::PushInt, proc.write_head());
    proc.write_offset += sizeof(Instruction);
    mem_put<int>(param_list->children.size(), proc.write_head());
    proc.write_offset += sizeof(int);

    // Store n_args in memory location right after the ip
    mem_put<Instruction>(Instruction::Store, proc.write_head());
    proc.write_offset += sizeof(Instruction);
    mem_put<int>(var_counter, proc.write_head());
    proc.write_offset += sizeof(int);
    var_counter++;

    // Allocate space for the jmp instruction:
    proc.write_offset += sizeof(Instruction);
    proc.write_offset += sizeof(int);
    
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
        proc.envs.back().var_indices[param_name] = var_counter;
        
        mem_put<Instruction>(Instruction::Store, proc.write_head());
        proc.write_offset += sizeof(Instruction);
        
        mem_put<int>(var_counter, proc.write_head());
        proc.write_offset += sizeof(int);
        
        var_counter++;
    }

    // Now go through the rest of the expressions in the function and
    // emit bytecode for them.
    for (int i = 3; i < ast->children.size(); i++)
        emit_expr(ast->children[i]);

    // Lastly, emit the ret instruction:
    mem_put<Instruction>(Instruction::Ret, proc.write_head());
    proc.write_offset += sizeof(Instruction);

    // Now go back to the beginning and add the jmp that skips over
    // the function body.
    mem_put<Instruction>(Instruction::Jmp,
                         proc.instructions + jmp_woff);
    
    mem_put<int>(proc.write_offset,
                 proc.instructions + jmp_woff + sizeof(Instruction));

    // Destroy current environment:
    proc.envs.pop_back();
}

void Runtime::eval_ast(std::unique_ptr<AST>& ast) {
    long long old_woff = proc.write_offset;
    emit_expr(ast);

    // TODO: Implement some kind of error flag that we can set during
    // bytecode generation. At this point, we should check the error
    // flag and potentially zero out all the bytecode we just
    // generated if we know it is invalid.

    // Print instructions:

    /*
    for (int i = old_woff; i < proc.write_offset; i++) {
        printf("%02x: %02x\n", i, proc.instructions[i]);
    }
    printf("\n");
    */

    // Run until we hit a 0 byte
    try {
        while (proc.cur_byte()) {
            unsigned char inst = proc.cur_byte();
            // Normally we'd expect the instruction pointer to be
            // incremented after the instruction is executed. Doing it
            // this way lets the jumped-to function immediately read any
            // arguments from memory without having to increment ip.
            proc.ip++;
            proc.jump_table[inst](proc);
        }
    }

    catch (...) {
        printf("RUNTIME ERROR\n");
        printf("Stack state:\n");
        for (int i = proc.stack.size() - 1; i >= 0; i--) {
            printf("%d ", proc.stack.back().as<int>());
            if (i == proc.stack.size() - 1)
                printf("<-------- top of stack");

            printf("\n");
        }
    }

    //printf("Instruction size: %lld bytes\n", proc.write_offset - old_woff);

    //proc.print_instructions();
    if (proc.stack.size() > 0)
        printf("Stack top: %d\n", proc.stack.back().as<int>());
    

    //    proc.stack.clear();
}
