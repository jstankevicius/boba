#pragma once

#include <vector>
#include "token.h"

enum AST_Type {
    // TODO: better names for this stuff
    AST_ROOT,

    AST_EXPR, // implicit grouping of parentheses

    AST_SYMBOL,

    // Literals
    AST_INT_LITERAL,
    AST_FLOAT_LITERAL,
    AST_STR_LITERAL,
    AST_BOOL_LITERAL,
};

struct AST {
    AST_Type type;

    // TODO: Maybe vector is too heavy-handed?
    std::vector<AST*> children;

    // We need something that lets us just send errors to the AST. One way to do
    // this is to attach an associated token to the AST. The problem is that
    // some AST constructs (such as the type chain) don't really have a "token"
    // that belongs to them. It may be better to attach an entire expression to
    // the AST.
    std::string string_value;

    void add_leaf_child(AST_Type type, std::string string_value) {
        AST* child = new AST;
        child->type = type;
        child->string_value = string_value;
        children.push_back(child);
    }
};

