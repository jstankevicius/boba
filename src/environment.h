#pragma once

#include <any>
#include <unordered_map>
#include <string>

enum class ValueType { Int, Float, Str, Bool, Ref };

struct Value {
    ValueType type;
    std::any value;

    template <typename T> inline T as() {
        return std::any_cast<T>(value);
    }
};


struct Environment {

    // "Memory" for getting the value of a variable based on its index.
    std::unordered_map<int, Value> memory;
    std::unordered_map<std::string, int> var_indices;
};
