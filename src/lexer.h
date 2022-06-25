#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <deque>
#include <memory>

#include "token.h"

// Macros:
#define is_alpha(c) (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
#define is_underscore(c) (c == '_')
#define is_numeric(c) ('0' <= c && c <= '9')
#define is_whitespace(c) (c == ' ' || c == '\t' || c == '\n' || c == '\r')
#define is_alphanumeric(c) (is_alpha(c) || is_numeric(c) || is_underscore(c))

#define is_arithmetic_operator(c) (c == '+' || c == '-' || c == '*' || c == '/' || c == '^')
#define is_comparator(c) (c == '=' || c == '<' || c == '>')
#define is_operator(c) (is_comparator(c) || is_arithmetic_operator(c))

class Lexer {

    private:
        int stream_idx = 0;
        int line_num = 1;
        int col_num = 1;

        std::string stream;

        char cur_char();
        void advance_char();
        char lookahead_char(int lookahead);
        char lookahead_char_at(int idx, int lookahead);
        void skip_whitespace();
        bool done();

        std::shared_ptr<Token> get_identifier_or_keyword();
        std::shared_ptr<Token> get_operator();
        std::shared_ptr<Token> get_numeric_literal();
        std::shared_ptr<Token> get_string_literal();
        std::shared_ptr<Token> get_punctuation();
        std::shared_ptr<Token> get_end_of_line();

    public:
        Lexer();
        std::deque<std::shared_ptr<Token>> tokenize_stream(std::string &stream);
};


