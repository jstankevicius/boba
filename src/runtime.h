#pragma once

#include <any>
#include <cstdint>
#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "environment.h"
#include "processor.h"


class Runtime {

private:
    Processor proc;

    int var_counter = 0;

    void emit_push_int(int i);
    void emit_push_ref(std::string &name);
    void emit_push(std::shared_ptr<AST> ast);
    void emit_if(std::shared_ptr<AST> ast);
    void emit_def(std::shared_ptr<AST> ast);
    void emit_function(std::shared_ptr<AST> ast);
    void emit_expr(std::shared_ptr<AST> ast);

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
