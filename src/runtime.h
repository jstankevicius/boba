#pragma once

#include <any>
#include <cstring>
#include <deque>
#include <iostream>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

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
    void emit_push_ref(std::shared_ptr<Value> value);
    void emit_push(std::shared_ptr<Value> value);
    void emit_if(std::shared_ptr<Value> value);
    void emit_cond(std::shared_ptr<Value> value);
    void emit_def(std::shared_ptr<Value> value);
    void emit_fn(std::shared_ptr<Value> value);
    void emit_call(std::shared_ptr<Value> value);
    void emit_expr(std::shared_ptr<Value> value);

public:

    Runtime();

    std::shared_ptr<Value> eval(std::shared_ptr<Value> value);
};
