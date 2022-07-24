#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <deque>
#include <memory>
#include <cassert>

#include "token.h"

class Lexer {

    private:
        unsigned int stream_idx = 0;
        unsigned int line_num = 1;
        unsigned int col_num = 1;

        std::string stream;

        char cur_char();
        void advance_char();
        char lookahead_char(unsigned int lookahead);
        char lookahead_char_at(unsigned int idx,
                               unsigned int lookahead);
        void skip_whitespace();
        bool done();

        std::shared_ptr<Token> get_symbol();
        std::shared_ptr<Token> get_numeric_literal();
        std::shared_ptr<Token> get_string_literal();
        std::shared_ptr<Token> get_punctuation();
        std::shared_ptr<Token> get_end_of_line();

    public:
        Lexer();
        std::deque<std::shared_ptr<Token>> tokenize_stream(std::string &stream);
};


