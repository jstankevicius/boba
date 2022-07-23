#pragma once

#include <any>
#include <unordered_map>
#include <string>

enum class ValueType { Nil, Int, Float, Str, Bool, Closure };

struct Value {
    ValueType type;
    std::any value;

    Value() {
        type = ValueType::Nil;
    }
    
    Value(int v) {
        type = ValueType::Int;
        value = v;
    }
   
    Value(bool v) {
        type = ValueType::Bool;
        value = v;
    }

    inline bool is_nil() {
        return type == ValueType::Nil;
    }

    template <typename T> inline T as() {
        return std::any_cast<T>(value);
    }

    std::string to_string() {
        switch (type) {
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


struct Closure {
    int n_args;

    // TODO: byte instructions[SOME_CONSTANT];
    unsigned char instructions[8192];

    // Environment that gets loaded onto the environment stack upon
    // this closure's call.
    std::unordered_map<int, std::shared_ptr<Value>> env;
};


