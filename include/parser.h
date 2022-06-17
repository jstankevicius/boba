#pragma once

#include <deque>
#include <unordered_map>
#include <vector>
#include "ast.h"

#define INDENT_WIDTH 4

class Parser {
    private:
        int indent_level;

        // Parsing functions
        AST* parse_type_annotation(std::deque<Token*> &tokens, AST* parent);
        AST* parse_function_decl(std::deque<Token*> &tokens, AST* parent);
        AST* parse_let_stmt(std::deque<Token*> &tokens, AST* parent);
        AST* parse_declaration(std::deque<Token*> &tokens, AST* parent);
        AST* parse_expression(std::deque<Token*> &tokens, AST* parent);

    public:
        Parser();
        AST* parse_tokens(std::deque<Token*> &tokens);
};
