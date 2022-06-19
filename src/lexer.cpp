#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "error.h"
#include "lexer.h"

Lexer::Lexer() {}

bool Lexer::done() {
    return stream_idx >= stream.length();
}

void Lexer::advance_char() {

    char cur = cur_char();
    if (!done()) {
        stream_idx++;
        col_num++;

        if (cur == '\n' || (cur == '\r' && lookahead_char(1) == '\n')) {
            // If we went over to the next line, reset col_num to 1 and
            // increment line_num.
            col_num = 1;
            line_num++;
        }
    }
}

char Lexer::cur_char() {

    if (!done())
        return stream[stream_idx];
    
    return -1;
}

char Lexer::lookahead_char(int lookahead) {
    if (stream_idx + lookahead < stream.length())
        return stream[stream_idx + lookahead];

    return -1;
}

char Lexer::lookahead_char_at(int idx, int lookahead) {
    if (idx + lookahead < stream.length())
        return stream[idx + lookahead];

    return -1;
}

void Lexer::skip_whitespace() {
    while (is_whitespace(cur_char())) {
        advance_char();
    }
}

std::shared_ptr<Token> Lexer::get_identifier_or_keyword()  {

    auto token = std::make_shared<Token>();
    token->col_num = col_num;
    token->line_num = line_num;
    token->stream = &stream;
    std::string str;

    // Don't need to check for out of bounds since cur_char just returns -1 once
    // we've reached the end of the stream.
    // TODO: Although the lexer isn't enforcing any sort of naming scheme, we
    // can enforce variable naming rules in the parser.
    while (is_alphanumeric(cur_char())) {
        str += cur_char();
        advance_char();
    }

    token->string_value = str;

    // TODO: formatting?
    token->type = (str == "true" || str == "false") ? 
                  TOKEN_BOOL_LITERAL : TOKEN_SYMBOL;

    return token;
}

std::shared_ptr<Token> Lexer::get_operator() {

    auto token = std::make_shared<Token>();
    token->col_num = col_num;
    token->line_num = line_num;
    token->stream = &stream;
    std::string op;

    while (is_operator(cur_char())) {
        op += cur_char();
        advance_char();
    }

    token->string_value = op;
    token->type = TOKEN_SYMBOL;

    return token;
}

std::shared_ptr<Token> Lexer::get_numeric_literal() {

    auto token = std::make_shared<Token>();
    token->col_num = col_num;
    token->line_num = line_num;
    token->stream = &stream;
    std::string num_literal;
    bool is_float_literal = false;

    if (cur_char() == '-') {
        num_literal += cur_char();
        advance_char();
    }

    while (is_numeric(cur_char())) {
        num_literal += cur_char();
        advance_char();
    }

    // Next character could potentially be a '.', which would make this a float
    // literal.
    if (cur_char() == '.' && is_numeric(lookahead_char(1))) {
        is_float_literal = true;
        num_literal += cur_char();
        advance_char();
    }

    else if (cur_char() == '.' && !is_numeric(lookahead_char(1))) {
        err_token(token, "decimals in the form of 'x.' are not allowed");
    }

    // Add the decimal part, if it exists.
    while (is_numeric(cur_char())) {
        num_literal += cur_char();
        advance_char();
    }
    token->string_value = num_literal;
    token->type = is_float_literal ? TOKEN_FLOAT_LITERAL : TOKEN_INT_LITERAL;

    return token;
}

std::shared_ptr<Token> Lexer::get_punctuation() {
    
    auto token = std::make_shared<Token>();
    token->col_num = col_num;
    token->line_num = line_num;
    token->stream = &stream;
    token->string_value += cur_char();
    token->type = TOKEN_PUNCTUATION;

    switch (cur_char()) {
        case '(':
        case ')':
        case '{':
        case '}':
        case ':':
        case '[':
        case ']':
            break;
        default:
            err_token(token, "unrecognized character");
            break;
    }
    advance_char();
    return token;
}

std::shared_ptr<Token> Lexer::get_string_literal() {

    auto token = std::make_shared<Token>();
    token->col_num = col_num;
    token->line_num = line_num;
    token->stream = &stream;
    std::string str_literal;

    str_literal += cur_char();
    advance_char();

    while (cur_char() != '"' && !done()) {
        str_literal += cur_char();
        advance_char();
    }

    // Add in closing quote, if it exists:
    if (cur_char() == '"') {
        str_literal += cur_char();
        advance_char();
    } else {
        // No matching quote
        err_token(token, "no matching quote");
    }

    token->type = TOKEN_STR_LITERAL;;
    token->string_value = str_literal;
    return token;
}


std::deque<std::shared_ptr<Token>> Lexer::tokenize_stream(std::string &stream)  {
    this->stream = stream;
    std::deque<std::shared_ptr<Token>> tokens;

    while (!done()) {

        // Identifiers can start with letters or underscores:
        if (is_alpha(cur_char()) || is_underscore(cur_char()))
            tokens.push_back(get_identifier_or_keyword());

        // Case for negative numbers:
        else if ((cur_char() == '-') 
            && (lookahead_char(1) == '.' || is_numeric(lookahead_char(1))))
                tokens.push_back(get_numeric_literal());

        else if (is_operator(cur_char()))
            tokens.push_back(get_operator());
        
        else if (is_numeric(cur_char())
            || (cur_char() == '.' && is_numeric(lookahead_char(1))))
            tokens.push_back(get_numeric_literal());

        // Beginning of a string literal
        else if (cur_char() == '"')
            tokens.push_back(get_string_literal());
        
        // Comments. We'll just skip the rest of the line here.
        else if (cur_char() == ';') {
            advance_char(); // skip over #
            while (cur_char() != '\r' && cur_char() != '\n') advance_char();
            if (cur_char() == '\n')
                advance_char();
            else if (cur_char() == '\r' && lookahead_char(1) == '\n') {
                advance_char();
                advance_char();
            }
        }

        // Everything else is assumed to be punctuation
        else
            tokens.push_back(get_punctuation());

        // Skip whitespace characters
        skip_whitespace();
    }

    auto eof_token = std::make_shared<Token>();
    eof_token->type = TOKEN_EOF;
    eof_token->string_value = "EOF";
    eof_token->stream = &stream;
    eof_token->col_num = col_num;
    eof_token->line_num = line_num;

    tokens.push_back(eof_token);

    return tokens;
}
