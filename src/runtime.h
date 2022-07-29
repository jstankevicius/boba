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

    Runtime();

    std::shared_ptr<Value> eval_ast(std::unique_ptr<AST>& ast);
};
