#pragma once

#include <any>
#include <unordered_map>
#include <string>

#define CLOSURE_INSTRUCTION_SIZE 4096

enum class ValueType
{
    Nil,
    Symbol,
    Int,
    Float,
    Str,
    Bool,
    Closure,
    ConsCell,
    EmptyList
};

struct Value
{
    ValueType type;
    std::any value;

    Value() : type(ValueType::Nil) {}
    
    Value(int v) : type(ValueType::Int), value(v) {}
   
    Value(bool v) : type(ValueType::Bool), value(v) {}

    inline bool is_nil()
    {
        return type == ValueType::Nil;
    }

    template <typename T> inline T as()
    {
        return std::any_cast<T>(value);
    }

    template <typename T> inline std::shared_ptr<T> as_ptr()
    {
        return std::any_cast<std::shared_ptr<T>>(value);
    }

    std::string to_string()
    {
        switch (type)
        {
        case ValueType::Nil:
            return "nil";
        case ValueType::Int:
            return std::to_string(as<int>());
        default:
            break;
        }
        
        return "<unknown>";
    }
};

struct Closure
{

    // Number of arguments the closure takes.
    int n_args;

    // Whether the last argument should be treated as variadic.
    bool last_param_variadic = false;

    unsigned char instructions[CLOSURE_INSTRUCTION_SIZE];
    unsigned int inst_size = 0;

    // Environment that gets loaded onto the environment stack upon
    // this closure's call.
    std::unordered_map<int, std::shared_ptr<Value>> env;

    Closure()
    {
        std::memset(instructions, 0, CLOSURE_INSTRUCTION_SIZE);
    }


    Closure(int n_args, bool last_param_variadic, unsigned char inst)
        : n_args(n_args), last_param_variadic(last_param_variadic)
    {
        std::memset(instructions, 0, CLOSURE_INSTRUCTION_SIZE);
        instructions[0] = inst;
        inst_size++;
    }
};

struct Symbol
{
    std::string str;

    Symbol(std::string s) : str(s) {}
};

struct ConsCell
{
    std::shared_ptr<Value> car;
    std::shared_ptr<Value> cdr;

    size_t size()
    {
        if (cdr->type == ValueType::ConsCell)
        {
            return 1 + cdr->as_ptr<ConsCell>()->size();
        }

        else if (cdr->type == ValueType::Nil)
        {
            return 1;
        }

        else
        {
            return 2;
        }
    }
};
