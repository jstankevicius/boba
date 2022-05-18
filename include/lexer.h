#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <deque>

#include "token.h"

// Macros:
#define is_alpha(c) (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || c == '_')
#define is_numeric(c) ('0' <= c && c <= '9')
#define is_whitespace(c) (c == ' ' || c == '\t')
#define is_alphanumeric(c) (is_alpha(c) || is_numeric(c))

#define is_arithmetic_operator(c) (c == '+' || c == '-' || c == '*' || c == '/' || c == '^')
#define is_comparator(c) (c == '=' || c == '<' || c == '>')
#define is_operator(c) (is_comparator(c) || is_arithmetic_operator(c))

class Lexer {

    private:
        int stream_idx = 0;
        int line_num = 1;
        int col_num = 1;

        std::string stream;
        std::unordered_map<std::string, Token_Type> reserved_types;

        char cur_char();
        void advance_char();
        char lookahead_char(int lookahead);
        char lookahead_char_at(int idx, int lookahead);
        void skip_whitespace();
        bool done();

        Token* get_identifier_or_keyword();
        Token* get_operator();
        Token* get_numeric_literal();
        Token* get_string_literal();
        Token* get_punctuation();
        Token* get_end_of_line();

    public:
        Lexer(std::string& stream);
        std::deque<Token*> tokenize_stream();
};


