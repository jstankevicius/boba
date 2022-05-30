#pragma once

#include <string>

enum Token_Type {

    TOKEN_ERROR,

    // Any identifier (including types!)
    TOKEN_IDENT,

    // Keywords
    TOKEN_KEYWORD_LET,
    TOKEN_KEYWORD_IF,

    // Literals
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STR_LITERAL,
    
    // Assignment operator:
    TOKEN_OPERATOR_ASSIGN,

    // Arithmetic operators:
    TOKEN_OPERATOR_PLUS,
    TOKEN_OPERATOR_MINUS,
    TOKEN_OPERATOR_MULT,
    TOKEN_OPERATOR_DIV,
    TOKEN_OPERATOR_EXP,

    // Comparison operators:
    TOKEN_OPERATOR_EQUALS,
    TOKEN_OPERATOR_NOT_EQUALS,
    TOKEN_OPERATOR_LESS,
    TOKEN_OPERATOR_LESS_EQ,
    TOKEN_OPERATOR_GREATER,
    TOKEN_OPERATOR_GREATER_EQ,

    // Boolean operators
    TOKEN_OPERATOR_OR,
    TOKEN_OPERATOR_AND,
    TOKEN_OPERATOR_NOT,

    // Punctuation, i.e. characters and symbols that are not operators but have
    // some kind of meaning in the language.
    TOKEN_PAREN_OPEN,
    TOKEN_PAREN_CLOSE,
    TOKEN_CURLY_OPEN,
    TOKEN_CURLY_CLOSE,
    TOKEN_PARAM_INDICATOR,
    TOKEN_TYPE_SIG,
    TOKEN_LEFT_ARROW,

    // End of line
    TOKEN_EOL,

    // End of file
    TOKEN_EOF
};


struct Token {

    int line_num, col_num;

    // TODO: This is probably not good, but for the sake of error-handling there
    // needs to be some way of determining the stream the token originally came
    // from. We obviously don't want to make a copy of the stream for every 
    // token so we'll just take a pointer to the stream instead. Probably want
    // some other way of getting the parent stream in the future. Thankfully
    // fixing this should only be a matter of fixing lexer.cpp, error.cpp, and
    // this file.
    std::string* stream;

    Token_Type type;

    // TODO: more types eventually?
    union {
        std::string string_value = "";
        int int_value;
        float float_value;
    };

};