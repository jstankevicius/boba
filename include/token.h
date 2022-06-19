#pragma once

#include <string>
#include <memory>

enum Token_Type {

    TOKEN_ERROR,

    // Anything that isn't a literal - can be a function name, identifier, or
    // a non-alphanumeric symbol like >=.

    // End of file
    TOKEN_EOF,

    TOKEN_PUNCTUATION,

    TOKEN_SYMBOL,

    // Literals
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STR_LITERAL,
    TOKEN_BOOL_LITERAL,
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
    std::string string_value;
};