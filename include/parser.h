#pragma once

#include <deque>
#include <unordered_map>
#include <vector>
#include "ast.h"
#include "token.h"

#define INDENT_WIDTH 4

class Parser {
    private:
        int indent_level;

        // Parsing functions
        std::shared_ptr<AST> parse_type_annotation(std::deque<std::shared_ptr<Token>> &tokens, std::shared_ptr<AST> parent);
        std::shared_ptr<AST> parse_function_decl(std::deque<std::shared_ptr<Token>> &tokens, std::shared_ptr<AST> parent);
        std::shared_ptr<AST> parse_let_stmt(std::deque<std::shared_ptr<Token>> &tokens, std::shared_ptr<AST> parent);
        std::shared_ptr<AST> parse_declaration(std::deque<std::shared_ptr<Token>> &tokens, std::shared_ptr<AST> parent);
        std::shared_ptr<AST> parse_expression(std::deque<std::shared_ptr<Token>> &tokens, std::shared_ptr<AST> parent);

    public:
        Parser();
        std::shared_ptr<AST> parse_tokens(std::deque<std::shared_ptr<Token>> &tokens);
};
