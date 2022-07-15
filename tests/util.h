#include <iostream>
#include <string>
#include <fstream>
#include <deque>

#include "lexer.h"
#include "parser.h"
#include "runtime.h"

template<typename T>
T eval_program(std::string program) {
    auto parser = std::make_unique<Parser>();
    auto runtime = std::make_unique<Runtime>();

    parser->tokenize_string(program);
    while (!parser->eof()) {
        auto ast = parser->parse_sexpr();
        runtime->eval_ast(ast);
    }
    return runtime->get_stack_top<T>();
}
