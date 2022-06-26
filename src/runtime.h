#pragma once

#include "bytecode.h"

#include <unordered_map>
#include <vector>
#include <optional>

class Runtime {

private:

    // Basically just a wrapper class around an unordered_map, renamed so it
    // makes a little more sense contextually.
    struct Scope {
        std::unordered_map<std::string, Value> symbols;
        inline void add_symbol(std::string name, Value &value);
        inline bool exists_symbol(std::string name);
    };

    std::vector<Scope> scopes;
    std::vector<Value> stack;

public:

    inline void enter_scope();
    inline void exit_scope();
    inline void add_symbol(std::string name, Value &value);
    inline bool exists_symbol(std::string name);

    std::optional<Value> get_value(std::string name);
    std::vector<Value> get_stack();
    void execute(std::vector<Instruction> &instructions);
};