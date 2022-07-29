
#pragma once

#include <vector>
#include <memory>
#include "token.h"

enum class ASTType
{
    Root,

    Expr, // implicit grouping of parentheses

    Symbol,

    // Literals
    IntLiteral,
    FloatLiteral,
    StrLiteral,
    BoolLiteral,
};

struct AST
{
    ASTType type;
    std::vector<std::unique_ptr<AST>> children;
    std::shared_ptr<Token> token;

    AST(ASTType type) : type(type)
    {

    }

    AST(ASTType type, std::shared_ptr<Token> token) : type(type), token(token)
    {

    }
        

    inline void add_leaf_child(ASTType type, std::shared_ptr<Token> token)
    {
        children.push_back(std::make_unique<AST>(type, token));
    }
};
