#pragma once

#include <any>
#include <unordered_map>
#include <string>

#define CLOSURE_INSTRUCTION_SIZE 4096

enum class BoxType
{
    Nil,
    Int,
    Float,
    Str,
    Bool,
    Closure,
    Symbol,
    ConsCell,
};

// A "boxed" (heap-allocated) value container along with a type label. This
// provides a more generic wrapper around shared_ptr. Although a Box can be
// instantiated with a shared_ptr to any kind of value, it is impossible to know
// what kind of value is being pointed to unless the `type` field is also
// set. It is up to the instantiating function to correctly set up the `type`
// field for a Box.
struct Box
{
    BoxType type;
    std::shared_ptr<void> ptr;

    Box()
    {
        
    }
    
    template<class T, class... Args> void alloc(Args&&... args)
    {
        ptr = std::make_shared<T>(args...);
    }

    template<typename T> inline std::shared_ptr<T> get_ptr()
    {
        return std::static_pointer_cast<T>(ptr);
    }
};


struct Nil
{
    
};



struct ConsCell
{
    Box car;
    Box cdr;

    ConsCell()
    {

    }

    // Value v;
    // v.value = std::make_shared<ConsCell>();
    // v.type = ValueType::ConsCell;
    // return std::make_shared<Value>(v);

};

struct Symbol
{
    std::string str;

    Symbol(std::string s) : str(s) {}
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
    std::unordered_map<int, Box> env;

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
