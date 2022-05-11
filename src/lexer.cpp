#include <iostream>
#include <string.h>
#include <unordered_map>
#include <vector>

#include "lexer.h"

Lexer::Lexer(std::string stream) {
    this->stream = stream;
}

bool Lexer::done() {
    return this->stream_idx >= this->stream.length();
}

void Lexer::advance_char() {

    char cur = this->cur_char();
    if (!this->done()) {
        this->stream_idx++;
        this->col_num++;

        // Did we just go over to the next line?
        if (cur == '\n') {
            this->col_num = 1;
            this->line_num++;
        }
    }
}

char Lexer::cur_char() {

    if (!this->done())
        return this->stream[this->stream_idx];
    
    return -1;
}

char Lexer::lookahead_char(int lookahead) {
    if (this->stream_idx + lookahead < this->stream.length())
        return this->stream[this->stream_idx + lookahead];

    return -1;
}

void Lexer::skip_whitespace() {
    while (is_whitespace(this->cur_char())) {
        this->advance_char();
    }
}

Token* maybe_get_identifier(Lexer* lexer)  {

    Token* token = new Token;
    std::string identifier;

    // Don't need to check for out of bounds since cur_char just returns -1 once
    // we've reached the end of the stream.
    while (is_alphanumeric(lexer->cur_char())) {
        identifier += lexer->cur_char();
        lexer->advance_char();
    }

    token->string_value = identifier;
    token->type = IDENTIFIER;

    if (identifier == "if" || identifier == "fn" || identifier == "let")
        token->type = KEYWORD;

    return token;
}

Token* maybe_get_operator(Lexer* lexer) {

    Token* token = new Token;
    std::string op;

    op += lexer->cur_char();
    lexer->advance_char();

    // Handle cases for ==, <=, >=, +=...
    if (lexer->cur_char() == '=') {
        op += lexer->cur_char();
        lexer->advance_char();
    }

    token->type = OPERATOR;
    token->string_value = op;

    return token;
}

Token* maybe_get_numeric_literal(Lexer* lexer) {

    Token* token = new Token;
    std::string num_literal;
    bool is_float_literal = false;

    while (is_numeric(lexer->cur_char())) {
        num_literal += lexer->cur_char();
        lexer->advance_char();
    }

    // Next character could potentially be a '.', which would make this a float
    // literal.
    if (lexer->stream_idx - 1 < lexer->stream.length()) {

        // TODO: potential bug if the number is something like "2." with no
        // number after the decimal?
        if (lexer->cur_char() == '.' && is_numeric(lexer->lookahead_char(1))) {
            is_float_literal = true;
            num_literal += lexer->cur_char();
            lexer->advance_char();
        }
        else if (lexer->cur_char() == '.' && !is_numeric(lexer->lookahead_char(1))) {
            token->type = ERROR;
            return token;
        }
    }

    // Add the decimal part, if it exists.
    while (is_numeric(lexer->cur_char())) {
        num_literal += lexer->cur_char();
        lexer->advance_char();
    }

    if (is_float_literal) {
        token->type = FLOAT_LITERAL;
        token->float_value = std::stof(num_literal);
    }
    else {
        token->type = INT_LITERAL;
        token->int_value = std::stoi(num_literal);
    }
    
    return token;
}

Token* maybe_get_string_literal(Lexer* lexer) {

    Token* token = new Token;
    std::string str_literal;

    str_literal += lexer->cur_char();
    lexer->advance_char();

    while (lexer->cur_char() != '"') {
        str_literal += lexer->cur_char();
        lexer->advance_char();
    }

    // Add in closing quote, if it exists:
    if (lexer->cur_char() == '"') {
        str_literal += lexer->cur_char();
        lexer->advance_char();
    } else {
        token->type = ERROR;
        return token;
    }

    token->type = STR_LITERAL;
    token->string_value = str_literal;
    return token;
}

std::vector<Token*> get_tokens(Lexer* lexer)  {

    std::vector<Token*> tokens;

    while (!lexer->done()) {

        // Add the identifier token:
        if (is_alpha(lexer->cur_char()))
            tokens.push_back(maybe_get_identifier(lexer));
        else if (is_operator(lexer->cur_char()))
            tokens.push_back(maybe_get_operator(lexer));
        else if (is_numeric(lexer->cur_char()))
            tokens.push_back(maybe_get_numeric_literal(lexer));

        // Beginning of a string literal
        else if (lexer->cur_char() == '"')
            tokens.push_back(maybe_get_string_literal(lexer));

        // Unknown
        else {
            Token* token = new Token;
            token->type = OTHER;
            token->string_value = lexer->cur_char();
            tokens.push_back(token);
            lexer->advance_char();
        }

        // Skip whitespace characters
        lexer->skip_whitespace();
    }

    std::cout << "Tokens: [";

    for (auto token : tokens) {
        if (token->string_value.length() > 0)
            std::cout << token->string_value << ", ";
        else
        std::cout << token->int_value << ", ";
    }
    
    std::cout << "]" << std::endl;
    return tokens;
}