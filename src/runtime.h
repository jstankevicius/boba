#pragma once

#include <any>
#include <cstring>
#include <deque>
#include <iostream>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "environment.h"
#include "processor.h"

struct Scope {
    std::unordered_map<std::string, int> var_indices;
};


class Runtime {

private:
    Processor proc;

    std::vector<Scope> scopes;
    int var_counter = 0;
    int builtin_counter = 0;

    void emit_push_int(int i);
    void emit_push_ref(std::unique_ptr<AST>& ast);
    void emit_push(std::unique_ptr<AST>& ast);
    void emit_do(std::unique_ptr<AST>& ast);
    void emit_if(std::unique_ptr<AST>& ast);
    void emit_cond(std::unique_ptr<AST>& ast);
    void emit_def(std::unique_ptr<AST>& ast);
    void emit_fn(std::unique_ptr<AST>& ast);
    void emit_call(std::unique_ptr<AST>& ast);
    void emit_expr(std::unique_ptr<AST>& ast);

public:

    Runtime() {
        scopes.push_back(Scope());

        // Initialize default runtime environment:
        auto& env = proc.envs.back();
        auto& scope = scopes.back();

        const std::tuple<std::string, int, bool, Instruction>
            builtins[] = {

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
        for (auto& builtin : builtins) {
            
            auto& fn_name = std::get<0>(builtin);
            int n_args = std::get<1>(builtin);
            bool variadic = std::get<2>(builtin);
            Instruction instruction = std::get<3>(builtin);

            auto c = std::make_shared<Closure>(n_args, variadic,
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

    std::shared_ptr<Value> eval_ast(std::unique_ptr<AST>& ast);
};
