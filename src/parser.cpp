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
void show_ast(std::unique_ptr<AST>& ast, int indent_level) {
    if (ast == NULL) return;

    for (int i = 0; i < indent_level; i++) std::cout << "   ";
    std::cout << "AST(" << ast->token->string_value << ") " << (int)ast->type << std::endl;

    for (auto& child : ast->children) {
        show_ast(child, indent_level + 1);
    }
}


// Expect the next token in the stream to have a particular string as
// its contents. If not, fail with an error on the token.
void expect_token_string(std::string str,
                         std::deque<std::shared_ptr<Token>> &tokens) {

    if (tokens.size() == 0) {
        printf("Unexpected EOF at end of file (you might have"
               " forgotten to close a parenthesis)");
        exit(-1);
    }
    
    auto& token = tokens.front();
    if (token->string_value != str)
        err_token(token, "syntax error: expected '" + str + "', but got '"
            + token->string_value + "' ");

    tokens.pop_front();
}


// Expect the next token in the stream to have a particular type. If
// not, fail with an error on the token.
void expect_token_type(TokenType type,
                       std::deque<std::shared_ptr<Token>> &tokens) {

    if (tokens.size() == 0) {
        printf("Unexpected EOF at end of file\n");
        exit(-1);
    }

    auto& token = tokens.front();

    if (tokens.front()->type != type) {
        switch (type) {
        case TokenType::Symbol:
            err_token(token, "expected a symbol");
            break;
        case TokenType::Punctuation:
            err_token(token, "expected '(', ')', '[', ']', '{', or '{'");
            break;
        case TokenType::IntLiteral:
            err_token(token, "expected an integer literal");
            break;
        case TokenType::FloatLiteral:
            err_token(token, "expected a float literal");
            break;
        case TokenType::StrLiteral:
            err_token(token, "expected a string literal");
            break;
        case TokenType::BoolLiteral:
            err_token(token, "expected a boolean literal");
            break;
        }
    }
    tokens.pop_front();
}


bool Parser::eof() {
    return tokens.size() == 0;
}


void Parser::tokenize_string(std::string &str) {
    Lexer lexer;
    auto lexed_tokens = lexer.tokenize_stream(str);

    for (auto& token : lexed_tokens) {
	tokens.push_back(token);
    }
}


// Parse an s-expression from the token stream. An expression (for
// now) is anything that is enclosed by parentheses.
std::unique_ptr<AST> Parser::parse_sexpr() {

    auto ast = std::make_unique<AST>(ASTType::Expr, tokens.front());
    expect_token_string("(", tokens);

    while (tokens.size() > 0 && tokens.front()->string_value != ")") {

        auto front = tokens.front();

        switch (front->type) {
        case (TokenType::Symbol):
            ast->add_leaf_child(ASTType::Symbol, front);
            expect_token_type(TokenType::Symbol, tokens);
            break;
        case (TokenType::StrLiteral):
            ast->add_leaf_child(ASTType::StrLiteral, front);
            expect_token_type(TokenType::StrLiteral, tokens);
            break;
        case (TokenType::IntLiteral):
            ast->add_leaf_child(ASTType::IntLiteral, front);
            expect_token_type(TokenType::IntLiteral, tokens);
            break;
        case (TokenType::FloatLiteral):
            ast->add_leaf_child(ASTType::FloatLiteral, front);
            expect_token_type(TokenType::FloatLiteral, tokens);
            break;
        case (TokenType::BoolLiteral):
            ast->add_leaf_child(ASTType::BoolLiteral, front);
            expect_token_type(TokenType::BoolLiteral, tokens);
            break;
        default:
            if (front->string_value == "(")
                ast->children.push_back(parse_sexpr());                
            else
                err_token(front, "internal parser error: unhandled"
                          " token type");
        }
    }
    
    expect_token_string(")", tokens);
    return ast;
}
