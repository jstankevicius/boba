#include "runtime.h"
#include <unordered_map>
#include <vector>
#include <optional>

inline bool Runtime::Scope::exists_symbol(std::string name) {
    return symbols.count(name) > 0;
}

inline void Runtime::Scope::add_symbol(std::string name, Value &value) {
    symbols.emplace(name, value);
}

inline void Runtime::enter_scope() {
    scopes.push_back(Scope());
}

inline void Runtime::exit_scope() {
    assert(scopes.size() > 0);
    scopes.pop_back();
}

inline void Runtime::add_symbol(std::string name, Value &value) {
    scopes.back().add_symbol(name, value);
};

inline bool Runtime::exists_symbol(std::string name) {
    for (int i = scopes.size() - 1; i --> 0;)
        if (scopes[i].exists_symbol(name))
            return true;
    
    return false;
}

std::optional<Value> Runtime::get_value(std::string name) {

    for (int i = scopes.size() - 1; i --> 0;)
        if (scopes[i].exists_symbol(name))
            return scopes[i].symbols[name];

    return {};
}

void Runtime::execute(std::vector<Instruction> &instructions) {
    int ip = 0;
    enter_scope();
    while (ip < instructions.size()) {
        auto& inst = instructions[ip];
        InstructionType type = inst.inst_type;

        if (type == InstructionType::PUSH) {
            if (inst.value.value().type == ValueType::SYMBOL) {
                auto name = std::any_cast<std::string>(inst.value.value().value);
                stack.push_back(scopes.back().symbols[name]);   
            } 
            else {
                stack.push_back(inst.value.value());    
            }
        }
        else if (type == InstructionType::STORE) {
            add_symbol(
                std::any_cast<std::string>(inst.value.value().value), 
                stack.back());
            stack.pop_back();
        }

        // TODO: make this work for floats and maybe strings
        else if (type == InstructionType::ADD) {
            int n = std::any_cast<int>(inst.value.value().value);
            int acc = 0;
            for (int i = 0; i < n; i++) {
                acc += std::any_cast<int>(stack.back().value);
                stack.pop_back();
            }
            stack.push_back(make_value(ValueType::INT, acc));
        }

        // TODO: make this work for floats
        else if (type == InstructionType::SUB) {
            int n = std::any_cast<int>(inst.value.value().value);
            int acc = 0;

            // unary negation
            if (n == 1) {
                acc -= std::any_cast<int>(stack.back().value);
                stack.pop_back();
                stack.push_back(make_value(ValueType::INT, acc));
            }
            else if (n == 2) {
                acc = std::any_cast<int>(stack.back().value);
                stack.pop_back();
                acc -= std::any_cast<int>(stack.back().value);
                acc *= -1;
                stack.pop_back();
                stack.push_back(make_value(ValueType::INT, acc));
            }
        }

        // TODO: make this work for floats
        else if (type == InstructionType::MUL) {
            int n = std::any_cast<int>(inst.value.value().value);
            int product = std::any_cast<int>(stack.back().value);
            stack.pop_back();
            for (int i = 1; i < n; i++) {
                product *= std::any_cast<int>(stack.back().value);
                stack.pop_back();
            }
            stack.push_back(make_value(ValueType::INT, product));
        }

        else if (type == InstructionType::DIV) {
            int divisor = std::any_cast<int>(stack.back().value);
            stack.pop_back();
            int quotient = std::any_cast<int>(stack.back().value) / divisor;
            stack.pop_back();
            stack.push_back(make_value(ValueType::INT, quotient));
        }
        ip++;
    }
    std::cout << std::endl;
    std::cout << "Result: " << std::any_cast<int>(stack.back().value) << std::endl;
}