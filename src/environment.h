#pragma once

#include <any>
#include <unordered_map>
#include <string>

enum class ValueType { Int, Float, Str, Bool, Closure };

struct Value {
    ValueType type;
    std::any value;

    Value() {
        
    }
    
    Value(int v) {
        type = ValueType::Int;
        value = v;
    }
   
    Value(bool v) {
        type = ValueType::Bool;
        value = v;
    }

    template <typename T> inline T as() {
        return std::any_cast<T>(value);
    }
};


struct Closure {
    int n_args;

    // TODO: byte instructions[SOME_CONSTANT];
    unsigned char instructions[1024];

    // Environment that gets loaded onto the environment stack upon
    // this closure's call.
    std::unordered_map<int, std::shared_ptr<Value>> env;
};


