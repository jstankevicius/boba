#include <deque>
#include <string>
#include <iostream>
#include <memory>
#include "token.h"
#include "error.h"
#include "parser.h"
#include "ast.h"
#include "lexer.h"


void show_ast(std::shared_ptr<AST> ast, int indent_level) {
    if (ast == NULL) return;

    for (int i = 0; i < indent_level; i++) std::cout << "   ";
    std::cout << "AST(" << ast->string_value << ")" << std::endl;

    for (auto& child : ast->children) {
        show_ast(child, indent_level + 1);
    }
}


inline void expect_token_string(std::string str, std::deque<std::shared_ptr<Token>> &tokens) {
    auto& token = tokens.front();
    if (token->string_value != str)
        err_token(token, "syntax error: expected '" + str + "', but got '"
            + token->string_value + "' ");
    
    tokens.pop_front();
}


inline void expect_token_type(Token_Type type, std::deque<std::shared_ptr<Token>> &tokens) {
    assert(tokens.size() > 0);
    auto& token = tokens.front();

    if (tokens.front()->type != type) {
        switch (type) {
            case TOKEN_SYMBOL:
                err_token(token, "expected a symbol");
            default:
                err_token(token, "internal parser error: unhandled token type!");
        }
    }
    tokens.pop_front();
}


// parse_tokens
// Parse a stream of tokens into an AST.
std::shared_ptr<AST> Parser::parse_tokens(std::deque<std::shared_ptr<Token>> &tokens) {

    auto root = std::make_shared<AST>(AST_ROOT, "");

    while (tokens.front()->type != TOKEN_EOF) {
        root->children.push_back(parse_expression(tokens));
    }

    show_ast(root, 0);
    return root;
}

/*
void check_valid_symbol(std::string symbol) {
    if (is_alpha(symbol.front() || is_underscore(symbol.front()))) {
        // Parse as variable:
    } else if (is_operator(symbol.front())) {
        // Parse as operator:
    }
}
*/

std::shared_ptr<AST> Parser::parse_expression(std::deque<std::shared_ptr<Token>> &tokens) {
    
    expect_token_string("(", tokens);

    auto ast = std::make_shared<AST>(AST_EXPR, tokens.front()->string_value);
    expect_token_type(TOKEN_SYMBOL, tokens);
    //check_valid_symbol(ast->string_value);

    while (tokens.front()->type != TOKEN_EOF && tokens.front()->string_value != ")") {
        auto& front = tokens.front();

        if (front->type == TOKEN_SYMBOL) {
            ast->add_leaf_child(AST_SYMBOL, front->string_value);
            expect_token_type(TOKEN_SYMBOL, tokens);
        } else if (front->type == TOKEN_STR_LITERAL) {
            ast->add_leaf_child(AST_STR_LITERAL, front->string_value);
            expect_token_type(TOKEN_STR_LITERAL, tokens);
        } else if (front->type == TOKEN_INT_LITERAL) {
            ast->add_leaf_child(AST_INT_LITERAL, front->string_value);
            expect_token_type(TOKEN_INT_LITERAL, tokens);
        } else if (front->type == TOKEN_FLOAT_LITERAL) {
            ast->add_leaf_child(AST_FLOAT_LITERAL, front->string_value);
            expect_token_type(TOKEN_FLOAT_LITERAL, tokens);
        } else if (front->type == TOKEN_BOOL_LITERAL) {
            ast->add_leaf_child(AST_BOOL_LITERAL, front->string_value);
            expect_token_type(TOKEN_BOOL_LITERAL, tokens);
        } 
        else if (front->string_value == "(") {
            ast->children.push_back(parse_expression(tokens));
        } else {
            err_token(front, "internal parser error: unhandled token type");
        }
    }
    expect_token_string(")", tokens);
    return ast;
}
