#pragma once

#include <vector>
#include <memory>
#include "token.h"

enum class ASTType {
    Root,

    Expr, // implicit grouping of parentheses

    Symbol,

    // Literals
    IntLiteral,
    FloatLiteral,
    StrLiteral,
    BoolLiteral,
};

struct AST {
    ASTType type;

    // TODO: Maybe vector is too heavy-handed?
    std::vector<std::shared_ptr<AST>> children;
    
    std::string string_value;

    AST(ASTType type) {
        this->type = type;
    }

    AST(ASTType type, std::string string_value) {
        this->type = type;
        this->string_value = string_value;
    }

    inline void
    add_leaf_child(ASTType type, std::string string_value) {
        children.push_back(std::make_shared<AST>(type, string_value));
    }
};
