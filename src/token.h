#pragma once

#include <string>
#include <memory>

enum class TokenType {

    // Used if the lexer ever somehow fails to categorize a token.
    Error,

    // End of file
    Eof,

    Punctuation,
    
    // Anything that isn't a literal - can be a function name,
    // identifier, or a non-alphanumeric symbol like >=.
    Symbol,

    // Literals
    IntLiteral,
    FloatLiteral,
    StrLiteral,
    BoolLiteral,
};


struct Token {

    int line_num, col_num;

    // TODO: This is probably not good, but for the sake of
    // error-handling there needs to be some way of determining the
    // stream the token originally came from. We obviously don't want
    // to make a copy of the stream for every token so we'll just take
    // a pointer to the stream instead. Probably want some other way
    // of getting the parent stream in the future. Thankfully fixing
    // this should only be a matter of fixing lexer.cpp, error.cpp,
    // and this file.
    std::string* stream;

    TokenType type;

    // TODO: more types eventually?
    std::string string_value;
};
