// The Boba runtime.

// Object types:
// 1. booleans
// 2. integers
// 3. floats (doubles)
// 4. lists
// 5. closures
// 6. io (files, etc)
// 7. strings
// 8. nil

#include "runtime.h"
#include <stddef.h>

#include <cstring>
#include <iostream>
#include <math.h>

#include "processor.h"
#include "error.h"

Runtime::Runtime()
{
    scopes.push_back(Scope());

    // Initialize default runtime environment:
    auto& env = proc.envs.back();
    auto& scope = scopes.back();

    // This should exist outside the constructor or something
    const std::tuple<std::string, int, bool, Instruction> builtins[] = {

        // Function name, # args, variadic, inst
        std::make_tuple("+", 2, false, Instruction::Add),
        std::make_tuple("-", 2, false, Instruction::Sub),
        std::make_tuple("*", 2, false, Instruction::Mul),
        std::make_tuple("/", 2, false, Instruction::Div),
        std::make_tuple("=", 2, false, Instruction::Eq),
        std::make_tuple(">", 2, false, Instruction::Greater),
        std::make_tuple(">=", 2, false, Instruction::GreaterEq),
        std::make_tuple("<", 2, false, Instruction::Less),
        std::make_tuple("<=", 2, false, Instruction::LessEq),
    };

    // Insert builtin information into the global scope and the
    // global processor environment.
    for (const auto& builtin : builtins)
    {
            
        auto& fn_name = std::get<0>(builtin);
        int n_args = std::get<1>(builtin);
        bool variadic = std::get<2>(builtin);
        Instruction instruction = std::get<3>(builtin);

        // TODO: Maybe constructing closures shouldn't be this annoying.
        auto c = std::make_shared<Closure>(n_args,
                                           variadic,
                                           static_cast<unsigned char>(instruction));
        
        Value v;
        v.type = ValueType::Closure;
        v.value = c;

        scope.var_indices[fn_name] = var_counter;
        env[var_counter] = std::make_shared<Value>(v);

        var_counter++;
        builtin_counter++;
    }
}

// Relative emit - emits an int exactly at write_offset, then advances
// write_offset.
inline void
Runtime::emit_push_int(int i)
{
    mem_put<Instruction>(Instruction::PushInt, proc.write_head);
    proc.write_head += sizeof(Instruction);

    mem_put<int>(i, proc.write_head);
    proc.write_head += sizeof(int);
}
                                                                                  
// Emit a push_ref instruction for a symbol.
inline void Runtime::emit_push_ref(std::unique_ptr<AST>& ast)
{
    // Figure out this ref's index. Start at the current environment and go back
    // up the stack, looking for the name. If not found, error out.
    auto& name = ast->token->string_value;
    
    int var_index = -1;
    for (int i = scopes.size() - 1; i >= 0; i--)
    {
        auto& var_indices = scopes[i].var_indices;
        if (var_indices.count(name) > 0)
        {
            var_index = var_indices[name];
            break;
        }
    }

    if (var_index < 0)
    {
        err_token(ast->token, "Undefined symbol '" + name + "'");
    }

    mem_put<Instruction>(Instruction::PushRef, proc.write_head);
    proc.write_head += sizeof(Instruction);
    mem_put<int>(var_index, proc.write_head);
    proc.write_head += sizeof(int);
}

void Runtime::emit_push(std::unique_ptr<AST>& ast)
{
    switch (ast->type)
    {
    case ASTType::IntLiteral:
        emit_push_int(std::stoi(ast->token->string_value));
        break;
    case ASTType::Symbol:
        emit_push_ref(ast);
        break;
    default:
        break;
    }
}

// Emit bytecode for an expression (or simply a symbol/literal).
void Runtime::emit_expr(std::unique_ptr<AST>& ast)
{
    if (ast->children.size() == 0)
    {
        if (ast->type != ASTType::Expr)
        {
            emit_push(ast);
        }
        
        // If the AST is an expression but has 0 children, we won't emit any
        // code for it at all.
        return;
    }
    
    auto& first = ast->children[0]->token->string_value;
    
    if (first == "def")
    {
        emit_def(ast);
    }
    else if (first == "do")
    {
        emit_do(ast);
    }
    else if (first == "if")
    {
        emit_if(ast);
    }
    else if (first == "fn")
    {
        emit_fn(ast);
    }
    else
    {
        emit_call(ast);
    }
}

// Emit a function call.
void Runtime::emit_call(std::unique_ptr<AST>& ast)
{
    auto& first = ast->children[0];
    std::string& fn_name = first->token->string_value;

    // First, add all the operands to the stack:
    for (unsigned long i = 1; i < ast->children.size(); i++)
    {
        auto &child = ast->children[i];
        if (child->type == ASTType::Expr)
        {
            emit_expr(child);
        }
        else
        {
            emit_push(child);
        }
    }

    // Get the function's index and instruction pointer:
    int var_index = -1;

    for (int i = scopes.size() - 1; i >= 0; i--)
    {
        auto& var_indices = scopes[i].var_indices;
        if (var_indices.count(fn_name) > 0)
        {
            var_index = var_indices[fn_name];
            break;
        }
    }

    if (var_index < 0)
    {
        err_token(first->token, "Undefined function '" + fn_name + "'");
    }

    // If this is a builtin function, just inline it. It is guaranteed to be
    // just one instruction.
    if (var_index < builtin_counter)
    {
        // Using operator[] is okay, since the closure is guaranteed to be
        // located in the first environment.
        auto value = proc.envs.front()[var_index];
        auto closure = value->as<std::shared_ptr<Closure>>();
        mem_put<unsigned char>(closure->instructions[0], proc.write_head);
        proc.write_head += sizeof(unsigned char);
    }
    else
    {
        mem_put<Instruction>(Instruction::Call, proc.write_head);
        proc.write_head += sizeof(Instruction);
        mem_put<int>(var_index, proc.write_head);
        proc.write_head += sizeof(int);
    }
}

// Emit the bytecode for a do statement.
void Runtime::emit_do(std::unique_ptr<AST>& ast)
{
    for (size_t i = 1; i < ast->children.size(); i++)
    {
        emit_expr(ast->children[i]);
    }
}

// Emit the bytecode for an if statement.
void Runtime::emit_if(std::unique_ptr<AST>& ast)
{
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
    // instruction that runs before the if block. We will later use old_woff as
    // an offset into proc.instructions to write the jmp_false instruction.
    unsigned char* old_head = proc.write_head;
    proc.write_head += sizeof(Instruction);
    proc.write_head += sizeof(int);

    // Emit if-part's bytecode.
    emit_expr(if_part);

    // else_woff is where the else bytecode will begin, accounting for the
    // additional jump instruction we're going to insert at the end of the if
    // block.
    unsigned char* else_head = proc.write_head + sizeof(Instruction) + sizeof(int);
    
    // At old_woff, insert a JmpFalse with the address of the byte after the if
    // block.
    mem_put<Instruction>(Instruction::JmpFalse, old_head);

    // Add number of bytes emitted between else_woff and old_woff as an argument
    // to jmp_false.
    mem_put<int>(else_head - old_head, old_head + sizeof(Instruction));

    // Now save the write_offset again as old_woff. We're going to use this to
    // write the jmp instruction which jumps to the byte after the else block.
    old_head = proc.write_head;

    // Move write_offset past our allocated space:
    proc.write_head += sizeof(Instruction);
    proc.write_head += sizeof(int);

    // Emit else-part's bytecode.
    emit_expr(else_part);

    // At old_woff (the end of the if block), insert an unconditional jump to
    // skip over the else block if it ever gets executed.
    mem_put<Instruction>(Instruction::Jmp, old_head);
    mem_put<int>(proc.write_head - old_head, old_head + sizeof(Instruction));
}

// Emit the bytecode for a def.
void Runtime::emit_def(std::unique_ptr<AST>& ast)
{
    // Leftmost child is always the symbol name
    // TODO: error handling here, like for having too many child nodes
    auto& left = ast->children[1];
    auto& right = ast->children[2];
    std::string symbol_name = left->token->string_value;
    
    // Look for symbol in this environment
    if (scopes.back().var_indices.count(symbol_name) > 0)
    {
        err_token(left->token, "redefinition of variable '" + symbol_name + "'");
    }

    int var_number = var_counter;

    // We will consider the symbol "defined" before we even figure out what the
    // symbol is bound to. This avoids bugs when parsing recursive functions.
    scopes.back().var_indices[symbol_name] = var_number;
    
    var_counter++;
    
    emit_expr(right);
    
    mem_put<Instruction>(Instruction::Store, proc.write_head);
    proc.write_head += sizeof(Instruction);

    mem_put<int>(var_number, proc.write_head);
    proc.write_head += sizeof(int);
}

// Emit the bytecode to generate a lambda.
void Runtime::emit_fn(std::unique_ptr<AST>& ast)
{

    // This creates a new scope.
    scopes.push_back(Scope());

    auto& param_list = ast->children[1];

    unsigned char* old_head = proc.write_head;

    // Allocate space for jump instruction
    proc.write_head += sizeof(Instruction) + sizeof(int);
        
    // It is assumed that when we make a function call, all the arguments are
    // pushed onto the stack before the jump. Here we emit store instructions
    // for those arguments and store their values into the new environment.
    for (int i = param_list->children.size() - 1; i >= 0; i--)
    {
        auto& child = param_list->children[i];
        
        // TODO: better error handling here
        if (child->type != ASTType::Symbol)
        {
            err_token(child->token, "parameter must be a symbol");
        }

        std::string &param_name = child->token->string_value;
        scopes.back().var_indices[param_name] = var_counter;
        
        mem_put<Instruction>(Instruction::Store, proc.write_head);
        proc.write_head += sizeof(Instruction);
        
        mem_put<int>(var_counter, proc.write_head);
        proc.write_head += sizeof(int);
        
        var_counter++;
    }

    // Now go through the rest of the expressions in the function and emit
    // bytecode for them.
    for (size_t i = 2; i < ast->children.size(); i++)
    {
        emit_expr(ast->children[i]);
    }

    // Lastly, emit the ret instruction:
    mem_put<Instruction>(Instruction::Ret, proc.write_head);
    proc.write_head += sizeof(Instruction);

    // The closure body begins `sizeof(Instruction) + sizeof(int)` bytes after
    // the jump.
    unsigned char* code_begin = old_head + sizeof(Instruction) + sizeof(int);
    unsigned char* code_end = proc.write_head;

    mem_put<Instruction>(Instruction::CreateClosure, proc.write_head);
    proc.write_head += sizeof(Instruction);

    mem_put<int>(code_end - code_begin, proc.write_head);
    proc.write_head += sizeof(int);
    
    // Now go back to the beginning and add the jmp that skips over the function
    // body.
    mem_put<Instruction>(Instruction::Jmp, old_head);
    
    mem_put<int>(proc.write_head - old_head - sizeof(Instruction) - sizeof(int),
                 old_head + sizeof(Instruction));
    
    // Destroy current scope:
    scopes.pop_back();
}

std::shared_ptr<Value> Runtime::eval_ast(std::unique_ptr<AST>& ast)
{
    unsigned char* old_head = proc.write_head;
    emit_expr(ast);

    // TODO: Implement some kind of error flag that we can set during bytecode
    // generation. At this point, we should check the error flag and potentially
    // zero out all the bytecode we just generated if we know it is invalid.

    // Run until we hit a 0 byte
    while (*proc.ip)
    {
        unsigned char inst = *proc.ip;
        proc.jump_table[inst](proc);
    }

    // Expressions that cannot possibly be referenced later in the program
    // (i.e. literally anything that is not a def or defn (possibly others) can
    // simply have their instructions zeroed out to free up space.
    if (ast->children.size() > 0)
    {
        auto form = ast->children[0]->token->string_value;
        if (form != "def")
        {
            std::memset(old_head, 0, proc.write_head - old_head);

            // Reset instruction pointer:
            proc.write_head = old_head;
            proc.ip = old_head;
        }
    }

    if (proc.stack.size() > 0)
    {
        auto result = proc.stack.back();
        proc.stack.clear();
        return result;
    }
    else
    {
        proc.stack.clear();

        // Default constructor initializes type to nil
        return std::make_shared<Value>();
    }
}
