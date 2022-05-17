#include <iostream>
#include <string.h>
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

Token* get_identifier_or_keyword(Lexer* lexer)  {

    Token* token = new Token;
    token->col_num = lexer->col_num;
    token->line_num = lexer->line_num;
    token->stream = &lexer->stream;
    std::string identifier;

    // Don't need to check for out of bounds since cur_char just returns -1 once
    // we've reached the end of the stream.
    while (is_alphanumeric(lexer->cur_char())) {
        identifier += lexer->cur_char();
        lexer->advance_char();
    }

    token->string_value = identifier;

    if (lexer->reserved_types.find(identifier) != lexer->reserved_types.end())
        token->type = lexer->reserved_types[identifier];
    else
        token->type = TOKEN_IDENT;

    return token;
}

Token* get_operator(Lexer* lexer) {

    Token* token = new Token;
    token->col_num = lexer->col_num;
    token->line_num = lexer->line_num;
    token->stream = &lexer->stream;
    std::string op;

    op += lexer->cur_char();
    lexer->advance_char();

    // Handle cases for ==, <=, >=
    if (lexer->cur_char() == '=') {
        op += lexer->cur_char();
        lexer->advance_char();
    }

    token->string_value = op;

    if (lexer->reserved_types.find(op) != lexer->reserved_types.end())
        token->type = lexer->reserved_types[op];
    else
        error(token, "unrecognized operator");

    return token;
}

Token* get_numeric_literal(Lexer* lexer) {

    Token* token = new Token;
    token->col_num = lexer->col_num;
    token->line_num = lexer->line_num;
    token->stream = &lexer->stream;
    std::string num_literal;
    bool is_float_literal = false;

    while (is_numeric(lexer->cur_char())) {
        num_literal += lexer->cur_char();
        lexer->advance_char();
    }

    // Next character could potentially be a '.', which would make this a float
    // literal.
    if (lexer->stream_idx - 1 < lexer->stream.length()) {

        if (lexer->cur_char() == '.' && is_numeric(lexer->lookahead_char(1))) {
            is_float_literal = true;
            num_literal += lexer->cur_char();
            lexer->advance_char();
        }

        // Case of a float without anything following the decimal. We should
        // probably keep this as an error since we're basically enforcing having
        // a leading 0 in front of a decimal already.
        else if (lexer->cur_char() == '.' && !is_numeric(lexer->lookahead_char(1))) {
            // TODO: fail here
            token->type = TOKEN_ERROR;
            return token;
        }
    }

    // Add the decimal part, if it exists.
    while (is_numeric(lexer->cur_char())) {
        num_literal += lexer->cur_char();
        lexer->advance_char();
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

Token* get_punctuation(Lexer* lexer) {
    
    Token* token = new Token;
    token->col_num = lexer->col_num;
    token->line_num = lexer->line_num;
    token->stream = &lexer->stream;

    switch (lexer->cur_char()) {
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
            if (lexer->lookahead_char(1) == ':') {
                token->type = TOKEN_PARAM_INDICATOR;
                lexer->advance_char();
                break;
            }
        default:
            error(token, "unrecognized character");
            break;
    }
    
    lexer->advance_char();
    return token;
}

Token* get_string_literal(Lexer* lexer) {

    Token* token = new Token;
    token->col_num = lexer->col_num;
    token->line_num = lexer->line_num;
    token->stream = &lexer->stream;
    std::string str_literal;

    str_literal += lexer->cur_char();
    lexer->advance_char();

    while (lexer->cur_char() != '"' && !lexer->done()) {
        str_literal += lexer->cur_char();
        lexer->advance_char();
    }

    // Add in closing quote, if it exists:
    if (lexer->cur_char() == '"') {
        str_literal += lexer->cur_char();
        lexer->advance_char();
    } else {
        // No matching quote
        error(token, "no matching quote");
    }

    token->type = TOKEN_STR_LITERAL;;
    token->string_value = str_literal;
    return token;
}

Token* get_end_of_line(Lexer* lexer) {
    // \n or \r\n?
    assert(lexer->cur_char() == '\n' || lexer->cur_char() == '\r');

    Token* token = new Token;
    token->col_num = lexer->col_num;
    token->line_num = lexer->line_num;
    token->stream = &lexer->stream;
    token->type = TOKEN_EOL;

    if (lexer->cur_char() == '\n')
        lexer->advance_char();
    else if (lexer->cur_char() == '\r' && lexer->lookahead_char(1) == '\n')
        lexer->advance_char();

    return token;
}

std::vector<Token*> get_tokens(Lexer* lexer)  {

    std::vector<Token*> tokens;

    while (!lexer->done()) {

        // Add the identifier token:
        if (is_alpha(lexer->cur_char()))
            tokens.push_back(get_identifier_or_keyword(lexer));
        else if (is_operator(lexer->cur_char()))
            tokens.push_back(get_operator(lexer));
        else if (is_numeric(lexer->cur_char()))
            tokens.push_back(get_numeric_literal(lexer));

        // Beginning of a string literal
        else if (lexer->cur_char() == '"')
            tokens.push_back(get_string_literal(lexer));
        else if (lexer->cur_char() == '\r' || lexer->cur_char() == '\n')
            tokens.push_back(get_end_of_line(lexer));

        // Everything else is assumed to be punctuation
        else
            tokens.push_back(get_punctuation(lexer));

        // Skip whitespace characters
        lexer->skip_whitespace();
    }

    Token* eof_token = new Token;
    eof_token->type = TOKEN_EOF;
    tokens.push_back(eof_token);

    
    return tokens;
}