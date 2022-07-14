#include "parser.h"

#include <deque>
#include <string>
#include <iostream>
#include <memory>

#include "token.h"
#include "error.h"
#include "ast.h"
#include "lexer.h"


// "Show" (display) the AST in std::cout. Used for debugging.
void show_ast(std::shared_ptr<AST> ast, int indent_level) {
    if (ast == NULL) return;

    for (int i = 0; i < indent_level; i++) std::cout << "   ";
    std::cout << "AST(" << ast->string_value << ")" << std::endl;

    for (auto& child : ast->children) {
        show_ast(child, indent_level + 1);
    }
}


// Expect the next token in the stream to have a particular string as
// its contents. If not, fail with an error on the token.
inline void expect_token_string(std::string str, std::deque<std::shared_ptr<Token>> &tokens) {
    auto& token = tokens.front();
    if (token->string_value != str)
        err_token(token, "syntax error: expected '" + str + "', but got '"
            + token->string_value + "' ");
    
    tokens.pop_front();
}


// Expect the next token in the stream to have a particular type. If
// not, fail with an error on the token.
inline void expect_token_type(TokenType type, std::deque<std::shared_ptr<Token>> &tokens) {
    assert(tokens.size() > 0);
    auto& token = tokens.front();

    if (tokens.front()->type != type) {
        switch (type) {
            case TokenType::SYMBOL:
                err_token(token, "expected a symbol");

		// TODO: handle all token types. For now we only care
		// about symbols.
            default:
                err_token(token, "internal parser error: unhandled token type!");
        }
    }
    tokens.pop_front();
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


void Parser::tokenize_string(std::string &str) {
    Lexer lexer;
    auto lexed_tokens = lexer.tokenize_stream(str);

    for (auto& token : lexed_tokens) {
	tokens.push_back(token);
    }
}


// Parse an s-expression from the token stream. An expression (for now)
// is anything that is enclosed by parentheses.
std::shared_ptr<AST> Parser::parse_sexpr() {
    
    expect_token_string("(", tokens);

    auto ast = std::make_shared<AST>(ASTType::EXPR, tokens.front()->string_value);
    expect_token_type(TokenType::SYMBOL, tokens);
    //check_valid_symbol(ast->string_value);

    while (tokens.front()->type != TokenType::_EOF && tokens.front()->string_value != ")") {
        auto& front = tokens.front();

        if (front->type == TokenType::SYMBOL) {
            ast->add_leaf_child(ASTType::SYMBOL, front->string_value);
            expect_token_type(TokenType::SYMBOL, tokens);
        } else if (front->type == TokenType::STR_LITERAL) {
            ast->add_leaf_child(ASTType::STR_LITERAL, front->string_value);
            expect_token_type(TokenType::STR_LITERAL, tokens);
        } else if (front->type == TokenType::INT_LITERAL) {
            ast->add_leaf_child(ASTType::INT_LITERAL, front->string_value);
            expect_token_type(TokenType::INT_LITERAL, tokens);
        } else if (front->type == TokenType::FLOAT_LITERAL) {
            ast->add_leaf_child(ASTType::FLOAT_LITERAL, front->string_value);
            expect_token_type(TokenType::FLOAT_LITERAL, tokens);
        } else if (front->type == TokenType::BOOL_LITERAL) {
            ast->add_leaf_child(ASTType::BOOL_LITERAL, front->string_value);
            expect_token_type(TokenType::BOOL_LITERAL, tokens);
        } 
        else if (front->string_value == "(") {
            ast->children.push_back(parse_sexpr());
        } else {
            err_token(front, "internal parser error: unhandled token type");
        }
    }
    expect_token_string(")", tokens);
    show_ast(ast, 0);
    return ast;
}
