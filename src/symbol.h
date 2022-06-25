#pragma once

#include <string>

enum class SymbolType {
    CONSTANT,
    FUNCTION
};

struct Symbol {
    std::string name;

    // Though some functions will take 0 or more arguments and variables will
    // always have 0 arguments, we'd also like to specify a variable number that
    // may be set by the programmer. This is for functions like print and some
    // builtin arithmetic functions, among others.
    // tl;dr: (+ 1 2 3 4) and (* 4 5 6) should be valid expressions.
    int num_args;
    SymbolType type;

    Symbol(std::string name, int num_args, SymbolType type) {
        this->name = name;
        this->num_args = num_args;
        this->type = type;
    }
};