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


class Runtime {

private:
    Processor proc;

    int var_counter = 0;

    void emit_push_int(int i);
    void emit_push_ref(std::string &name);
    void emit_push(std::shared_ptr<AST> ast);
    void emit_if(std::shared_ptr<AST> ast);
    void emit_def(std::shared_ptr<AST> ast);
    void emit_defn(std::shared_ptr<AST> ast);
    void emit_function(std::shared_ptr<AST> ast);
    void emit_expr(std::shared_ptr<AST> ast);

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
        proc.envs.push_back(Environment());
        std::memset(proc.instructions, 0, PROC_INSTRUCTION_SIZE);
    }

    void eval_ast(std::shared_ptr<AST> ast);

    template<typename T>
    inline T get_stack_top() {
        if (proc.stack.size() > 0)
            return proc.stack.back().as<T>();
        else {
            printf("Runtime stack is empty!\n");
            exit(-1);
        }
    }
};
