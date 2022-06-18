#include <deque>
#include <string>
#include <iostream>
#include "token.h"
#include "error.h"
#include "parser.h"
#include "ast.h"
#include "lexer.h"

Parser::Parser() {}

void show_ast(AST* ast, int indent_level) {
    if (ast == NULL) return;

    for (int i = 0; i < indent_level; i++) std::cout << "   ";
    std::cout << "AST(" << ast->string_value << ")" << std::endl;

    for (AST* child : ast->children) {
        show_ast(child, indent_level + 1);
    }
}

void expect_token_string(std::string str, std::deque<Token*> &tokens) {
    Token* token = tokens.front();
    if (token->string_value != str)
        err_token(token, "syntax error: expected '" + str + "', but got '"
            + token->string_value + "' ");
    
    tokens.pop_front();
}

void expect_token_type(Token_Type type, std::deque<Token*> &tokens) {
    assert(tokens.size() > 0);
    Token* token = tokens.front();

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
AST* Parser::parse_tokens(std::deque<Token*> &tokens) {

    AST* root = new AST;
    root->type = AST_ROOT;

    while (tokens.front()->type != TOKEN_EOF) {
        root->children.push_back(parse_expression(tokens, root));
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

AST* Parser::parse_expression(std::deque<Token*> &tokens, AST* parent) {
    AST* ast = new AST;
    expect_token_string("(", tokens);
    ast->type = AST_EXPR;
    ast->string_value = tokens.front()->string_value;

    expect_token_type(TOKEN_SYMBOL, tokens);
    //check_valid_symbol(ast->string_value);

    // TODO: maybe this could be cleaner?
    while (tokens.front()->type != TOKEN_EOF && tokens.front()->string_value != ")") {
        Token* front = tokens.front();

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
            ast->children.push_back(parse_expression(tokens, ast));
        } else {
            err_token(front, "internal parser error: unhandled token type");
        }
    }
    expect_token_string(")", tokens);
    return ast;
}