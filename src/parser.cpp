#include "parser.h"

#include <deque>
#include <iostream>
#include <memory>
#include <string>

#include "ast.h"
#include "environment.h"
#include "error.h"
#include "lexer.h"
#include "token.h"

// Expect the next token in the stream to have a particular string as its
// contents. If not, fail with an error on the token.
void expect_token_string(std::string str,
                         std::deque<std::shared_ptr<Token>> &tokens)
{

    if (tokens.size() == 0)
    {
        printf("Unexpected EOF at end of file\n");
        exit(-1);
    }
    
    auto& token = tokens.front();
    if (token->string_value != str)
    {
        err_token(token,
                  "syntax error: expected '"
                  + str
                  + "', but got '"
                  + token->string_value
                  + "' ");
    }

    tokens.pop_front();
}

// Expect the next token in the stream to have a particular type. If not, fail
// with an error on the token.
void expect_token_type(TokenType type,
                       std::deque<std::shared_ptr<Token>> &tokens)
{

    if (tokens.size() == 0)
    {
        printf("Unexpected EOF at end of file\n");
        exit(-1);
    }

    auto token = tokens.front();

    if (tokens.front()->type != type)
    {
        switch (type)
        {
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

// Parse an s-expression from the token stream. An expression (for
// now) is anything that is enclosed by parentheses.
std::shared_ptr<ConsCell> parse_expr(std::deque<std::shared_ptr<Token>>& tokens)
{

    auto cons_val = std::make_shared<Value>();
    auto first = cons_val;
    
    expect_token_string("(", tokens);

    while (tokens.size() > 0 && tokens.front()->string_value != ")")
    {
        cons_val->type = ValueType::ConsCell;
        auto front = tokens.front();
        auto cons = std::make_shared<ConsCell>();
        cons->cdr = std::make_shared<Value>();

        switch (front->type)
        {
        case (TokenType::Symbol):
        {
            auto sym_val = std::make_shared<Value>();
            sym_val->type = ValueType::Symbol;
            sym_val->value = std::make_shared<Symbol>(front->string_value);
            cons->car = sym_val;
            
            expect_token_type(TokenType::Symbol, tokens);
            break;
        }
        case (TokenType::IntLiteral):
        {
            auto int_val = std::make_shared<Value>();
            int_val->type = ValueType::Int;
            int_val->value = std::stoi(front->string_value);
            cons->car = int_val;
            
            expect_token_type(TokenType::IntLiteral, tokens);
            break;
        }
        default:
        {
            if (front->string_value == "(")
            {
                if (tokens.at(1)->string_value == ")")
                {
                    auto nil_val = std::make_shared<Value>();
                    nil_val->type = ValueType::EmptyList;
                    nil_val->value = nullptr;
                    cons->car = nil_val;
                }

                else
                {
                    auto expr = parse_expr(tokens);
                    auto expr_val = std::make_shared<Value>();
                    expr_val->type = ValueType::ConsCell;
                    expr_val->value = expr;
                }
            }
            else
            {
                err_token(front, "internal parser error: unhandled token type");
            }
        }
        }
        cons_val->value = cons;
        cons_val = cons_val->as_ptr<ConsCell>()->cdr;
    }

    printf("%zu\n", first->as_ptr<ConsCell>()->size());
    
    expect_token_string(")", tokens);
    return first->as_ptr<ConsCell>();
}
