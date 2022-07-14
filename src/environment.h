#pragma once

#include <any>
#include <unordered_map>

enum class ValueType { INT, FLOAT, STR, BOOL, REF };

struct Value {
    ValueType type;
    std::any value;

    template <typename T> inline T as() {
        return std::any_cast<T>(value);
    }
};


class Environment {
private:

    // "Memory" for getting the value of a variable based on its index.
    std::unordered_map<int, Value> memory;

    
        
public:
    
};
