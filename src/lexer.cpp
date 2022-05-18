#include <iostream>
#include <string>
#include <vector>

#include "error.h"
#include "lexer.h"

Lexer::Lexer(std::string& stream) {
    this->stream = stream;

    this->reserved_types["let"]  = TOKEN_KEYWORD_LET;
    this->reserved_types["if"]   = TOKEN_KEYWORD_IF;

    // Skip literals...
    this->reserved_types["="]    = TOKEN_OPERATOR_ASSIGN;

    // Arithmetic operators:
    this->reserved_types["+"]    = TOKEN_OPERATOR_PLUS;
    this->reserved_types["-"]    = TOKEN_OPERATOR_MINUS;
    this->reserved_types["*"]    = TOKEN_OPERATOR_MULT;
    this->reserved_types["/"]    = TOKEN_OPERATOR_DIV;
    this->reserved_types["^"]    = TOKEN_OPERATOR_EXP;

    // Comparison operators:
    this->reserved_types["=="]   = TOKEN_OPERATOR_EQUALS;
    this->reserved_types["!="]   = TOKEN_OPERATOR_NOT_EQUALS;
    this->reserved_types["<"]    = TOKEN_OPERATOR_LESS;
    this->reserved_types["<="]   = TOKEN_OPERATOR_LESS_EQ;
    this->reserved_types[">"]    = TOKEN_OPERATOR_GREATER;
    this->reserved_types[">="]   = TOKEN_OPERATOR_GREATER_EQ;

    // Logical operators:
    this->reserved_types["||"]   = TOKEN_OPERATOR_OR;
    this->reserved_types["&&"]   = TOKEN_OPERATOR_AND;
    this->reserved_types["!"]    = TOKEN_OPERATOR_NOT;

    // Punctuation:
    this->reserved_types["("]    = TOKEN_PAREN_OPEN;
    this->reserved_types[")"]    = TOKEN_PAREN_CLOSE;
    this->reserved_types["{"]    = TOKEN_CURLY_OPEN;
    this->reserved_types["}"]    = TOKEN_CURLY_CLOSE;
    this->reserved_types["::"]   = TOKEN_PARAM_INDICATOR;
}

bool Lexer::done() {
    return this->stream_idx >= this->stream.length();
}

void Lexer::advance_char() {

    char cur = this->cur_char();
    if (!this->done()) {
        this->stream_idx++;
        this->col_num++;

        if (cur == '\n' || (cur == '\r' && this->lookahead_char(1) == '\n')) {
            // If we went over to the next line, reset col_num to 1 and
            // increment line_num.
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

char Lexer::lookahead_char_at(int idx, int lookahead) {
    if (idx + lookahead < this->stream.length())
        return this->stream[idx + lookahead];

    return -1;
}

void Lexer::skip_whitespace() {
    while (is_whitespace(this->cur_char())) {
        this->advance_char();
    }
}

Token* Lexer::get_identifier_or_keyword()  {

    Token* token = new Token;
    token->col_num = col_num;
    token->line_num = line_num;
    token->stream = &stream;
    std::string identifier;

    // Don't need to check for out of bounds since cur_char just returns -1 once
    // we've reached the end of the stream.
    while (is_alphanumeric(cur_char())) {
        identifier += cur_char();
        advance_char();
    }

    token->string_value = identifier;

    if (reserved_types.find(identifier) != reserved_types.end())
        token->type = reserved_types[identifier];
    else
        token->type = TOKEN_IDENT;

    return token;
}

Token* Lexer::get_operator() {

    Token* token = new Token;
    token->col_num = col_num;
    token->line_num = line_num;
    token->stream = &stream;
    std::string op;

    op += cur_char();
    advance_char();

    // Handle cases for ==, <=, >=
    if (cur_char() == '=') {
        op += cur_char();
        advance_char();
    }

    token->string_value = op;

    if (reserved_types.find(op) != reserved_types.end())
        token->type = reserved_types[op];
    else
        error(token, "unrecognized operator");

    return token;
}

Token* Lexer::get_numeric_literal() {

    Token* token = new Token;
    token->col_num = col_num;
    token->line_num = line_num;
    token->stream = &stream;
    std::string num_literal;
    bool is_float_literal = false;

    while (is_numeric(cur_char())) {
        num_literal += cur_char();
        advance_char();
    }

    // Next character could potentially be a '.', which would make this a float
    // literal.
    if (stream_idx - 1 < stream.length()) {

        if (cur_char() == '.' && is_numeric(lookahead_char(1))) {
            is_float_literal = true;
            num_literal += cur_char();
            advance_char();
        }

        // Case of a float without anything following the decimal. We should
        // probably keep this as an error since we're basically enforcing having
        // a leading 0 in front of a decimal already.
        else if (cur_char() == '.' && !is_numeric(lookahead_char(1))) {
            // TODO: fail here
            token->type = TOKEN_ERROR;
            return token;
        }
    }

    // Add the decimal part, if it exists.
    while (is_numeric(cur_char())) {
        num_literal += cur_char();
        advance_char();
    }

    if (is_float_literal) {
        token->type = TOKEN_FLOAT_LITERAL;
        token->float_value = std::stof(num_literal);
    }
    else {
        token->type = TOKEN_INT_LITERAL;
        token->int_value = std::stoi(num_literal);
    }
    
    return token;
}

Token* Lexer::get_punctuation() {
    
    Token* token = new Token;
    token->col_num = col_num;
    token->line_num = line_num;
    token->stream = &stream;

    switch (cur_char()) {
        case '(':
            token->type = TOKEN_PAREN_OPEN;
            break;
        case ')':
            token->type = TOKEN_PAREN_CLOSE;
            break;
        case '{':
            token->type = TOKEN_CURLY_OPEN;
            break;
        case '}':
            token->type = TOKEN_CURLY_CLOSE;
            break;
        case ':':
            if (lookahead_char(1) == ':') {
                token->type = TOKEN_PARAM_INDICATOR;
                advance_char();
                break;
            }
        default:
            error(token, "unrecognized character");
            break;
    }
    
    advance_char();
    return token;
}

Token* Lexer::get_string_literal() {

    Token* token = new Token;
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
        error(token, "no matching quote");
    }

    token->type = TOKEN_STR_LITERAL;;
    token->string_value = str_literal;
    return token;
}

Token* Lexer::get_end_of_line() {
    // \n or \r\n?
    assert(cur_char() == '\n' || cur_char() == '\r');

    Token* token = new Token;
    token->col_num = col_num;
    token->line_num = line_num;
    token->stream = &stream;
    token->type = TOKEN_EOL;

    if (cur_char() == '\n')
        advance_char();
    else if (cur_char() == '\r' && lookahead_char(1) == '\n')
        advance_char();

    return token;
}

std::deque<Token*> Lexer::tokenize_stream()  {

    std::deque<Token*> tokens;

    while (!done()) {

        // Add the identifier token:
        if (is_alpha(cur_char()))
            tokens.push_back(get_identifier_or_keyword());
        else if (is_operator(cur_char()))
            tokens.push_back(get_operator());
        else if (is_numeric(cur_char()))
            tokens.push_back(get_numeric_literal());

        // Beginning of a string literal
        else if (cur_char() == '"')
            tokens.push_back(get_string_literal());
        else if (cur_char() == '\r' || cur_char() == '\n')
            tokens.push_back(get_end_of_line());

        // Everything else is assumed to be punctuation
        else
            tokens.push_back(get_punctuation());

        // Skip whitespace characters
        skip_whitespace();
    }

    Token* eof_token = new Token;
    eof_token->type = TOKEN_EOF;
    tokens.push_back(eof_token);

    return tokens;
}