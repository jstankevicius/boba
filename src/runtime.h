#pragma once

#include <any>
#include <cstdint>
#include <cstring>
#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "environment.h"
#include "processor.h"

// Container for information about a builtin function - what
// instruction it corresponds to, how many arguments it requires, and
// whether it needs an integer argument supplied in memory.
struct BuiltinEntry {
    Instruction inst;

    // If nargs is -1, the runtime will know to supply the number of
    // arguments it received.
    int n_args;

    BuiltinEntry(Instruction _inst, int _n_args) {
        inst = _inst;
        n_args = _n_args;
    }
};

struct Scope {
    std::unordered_map<std::string, int> var_indices;
};


class Runtime {

private:
    Processor proc;

    std::vector<Scope> scopes;
    int var_counter = 0;

    void emit_push_int(int i);
    void emit_push_ref(std::string& name);
    void emit_push(std::unique_ptr<AST>& ast);
    void emit_do(std::unique_ptr<AST>& ast);
    void emit_if(std::unique_ptr<AST>& ast);
    void emit_cond(std::unique_ptr<AST>& ast);
    void emit_def(std::unique_ptr<AST>& ast);
    void emit_fn(std::unique_ptr<AST>& ast);
    void emit_call(std::unique_ptr<AST>& ast);
    void emit_expr(std::unique_ptr<AST>& ast);

    const std::unordered_map<std::string, BuiltinEntry> builtins = {
        { "+", BuiltinEntry(Instruction::Add, 2) },

        // NOTE: Special case. "-" actually has two different meanings
        // depending on how many arguments are passed in. If there is
        // only one argument, it's actually mapped to
        // Instruction::Neg. This should be overridden during runtime.
        { "-", BuiltinEntry(Instruction::Sub, 2) },
        { "*", BuiltinEntry(Instruction::Mul, 2) },
        { "/", BuiltinEntry(Instruction::Div, 2) },
        { "=", BuiltinEntry(Instruction::Eq, 2) },
        { ">", BuiltinEntry(Instruction::Greater, 2) },
        { ">=", BuiltinEntry(Instruction::GreaterEq, 2) },
        { "<", BuiltinEntry(Instruction::Less, 2) },
        { "<=", BuiltinEntry(Instruction::LessEq, 2) }
    };

public:

    Runtime() {
        scopes.push_back(Scope());
        proc.envs.push_back(std::unordered_map<int, std::shared_ptr<Value>>());
        std::memset(proc.instructions, 0, PROC_INSTRUCTION_SIZE);
    }

    std::shared_ptr<Value> eval_ast(std::unique_ptr<AST>& ast);
};
