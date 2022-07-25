// The Boba lexer. The lexer's only job is to take a "stream" (a fancy
// term for a string) and turn it into a list of tokens that is then
// passed back to the parser.

#include "lexer.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "error.h"

inline bool is_alpha(char c) {
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

inline bool is_numeric(char c) {
    return '0' <= c && c <= '9';
}

inline bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool is_alphanumeric(char c) {
    return is_numeric(c) || is_alphanumeric(c);
}

inline bool is_punctuation(char c) {
    return c == '(' || c == ')' || c == '['
        || c == ']' || c == '{' || c == '}';
}

// Returns whether the lexer has processed the entire stream. This
// happens when idx is pushed beyond the length of the actual stream.
inline bool TextHandle::done() {
    return idx >= stream.length();
}

// If the stream is not done, returns the character at
// stream_idx. Otherwise, returns -1 (an invalid character).
char TextHandle::cur_char() {
    if (!done())
        return stream[idx];

    return -1;
}

// Peeks at the character directly after current character of the
// TextHandle. If reading this character would cause an out-of-bounds
// violation (i.e. there are no more characters left), returns -1.
char TextHandle::peek() {
    if (idx + 1 < stream.length())
        return stream[idx + 1];

    return -1;
}

// If the lexer has not processed the entire stream yet, advances the
// TextHandle's idx to the next character, incrementing the current
// column number. If idx ends on a newline sequence (i.e. the
// character \n or the sequence \r\n), advance_char() skips over the
// newline and adjusts the line and column numbers accordingly.
void TextHandle::advance_char() {
    char cur = cur_char();
    
    if (!done()) {
        idx++;
        col_num++;

        if (cur == '\n' || (cur == '\r' && peek() == '\n')) {
            // If we went over to the next line, reset col_num to 1
	    // and increment line_num.
            col_num = 1;
            line_num++;
        }
    }
}

// Skips over whitespace characters until a non-whitespace character
// is encountered.
void TextHandle::skip_whitespace() {
    while (is_whitespace(cur_char())) {
        advance_char();
    }
}

// Returns a token that is not a literal. These can be tokens like
// "+", ">=", "variable-name", etc. The lexer does not validate the
// names of the tokens, as that is done later.
std::shared_ptr<Token> get_symbol(TextHandle& t)  {

    auto token = std::make_shared<Token>();
    token->col_num = t.col_num;
    token->line_num = t.line_num;
    token->stream = &t.stream;
    
    std::string str;

    // Don't need to check for out of bounds since cur_char just
    // returns -1 once we've reached the end of the stream.
    while (!is_whitespace(t.cur_char()) && !t.done()
           && !is_punctuation(t.cur_char())) {
        str += t.cur_char();
        t.advance_char();
    }

    token->string_value = str;

    // TODO: formatting?
    token->type = (str == "true" || str == "false")
        ? TokenType::BoolLiteral
        : TokenType::Symbol;

    return token;
}

// Returns a token for a numeric literal (like 123, 3.14, or their
// negative counterparts).
std::shared_ptr<Token> get_numeric_literal(TextHandle& t) {

    auto token = std::make_shared<Token>();
    token->col_num = t.col_num;
    token->line_num = t.line_num;
    token->stream = &t.stream;
    std::string num_literal;
    bool is_float_literal = false;

    if (t.cur_char() == '-') {
        num_literal += t.cur_char();
        t.advance_char();
    }

    while (is_numeric(t.cur_char())) {
        num_literal += t.cur_char();
        t.advance_char();
    }

    // Next character could potentially be a '.', which would make
    // this a float literal.
    if (t.cur_char() == '.' && is_numeric(t.peek())) {
        is_float_literal = true;
        num_literal += t.cur_char();
        t.advance_char();
    }

    else if (t.cur_char() == '.' && !is_numeric(t.peek())) {
        err_token(token, "decimals in the form of 'x.' are not allowed");
    }

    // Add the decimal part, if it exists.
    while (is_numeric(t.cur_char())) {
        num_literal += t.cur_char();
        t.advance_char();
    }
    
    token->string_value = num_literal;
    token->type = is_float_literal
        ? TokenType::FloatLiteral
        : TokenType::IntLiteral;

    return token;
}

// Returns a token for "punctuation". This is a catch-all term for
// tokens that are not symbols or literals.
std::shared_ptr<Token> get_punctuation(TextHandle& t) {

    auto token = std::make_shared<Token>();
    token->col_num = t.col_num;
    token->line_num = t.line_num;
    token->stream = &t.stream;
    token->string_value += t.cur_char();
    token->type = TokenType::Punctuation;

    switch (t.cur_char()) {

	// All supported "punctuation" characters can be seen here:
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
    
    t.advance_char();
    return token;
}

// Returns a token for a string literal, like "Hello".
std::shared_ptr<Token> get_string_literal(TextHandle& t) {

    auto token = std::make_shared<Token>();
    token->col_num = t.col_num;
    token->line_num = t.line_num;
    token->stream = &t.stream;
    std::string str_literal;

    str_literal += t.cur_char();
    t.advance_char();

    while (t.cur_char() != '"' && !t.done()) {
        str_literal += t.cur_char();
        t.advance_char();
    }

    // Add in closing quote, if it exists:
    if (t.cur_char() == '"') {
        str_literal += t.cur_char();
        t.advance_char();
    } else {
        // No matching quote
        err_token(token, "no matching quote");
    }

    token->type = TokenType::StrLiteral;;
    token->string_value = str_literal;
    return token;
}

// Tokenizes the string in a TextHandle into a token list.
std::deque<std::shared_ptr<Token>> tokenize(TextHandle& t) {
    
    std::deque<std::shared_ptr<Token>> tokens;

    while (!t.done()) {

        // Case for negative numbers:
        if (t.cur_char() == '-'
            && (t.peek() == '.' || is_numeric(t.peek())))
            tokens.push_back(get_numeric_literal(t));

        else if (is_numeric(t.cur_char())
                 || (t.cur_char() == '.' && is_numeric(t.peek())))
            tokens.push_back(get_numeric_literal(t));

        // Beginning of a string literal
        else if (t.cur_char() == '"')
            tokens.push_back(get_string_literal(t));

        // Comments. We'll just skip the rest of the line here.
        else if (t.cur_char() == ';') {
            t.advance_char(); // skip over ;
            while (t.cur_char() != '\r' && t.cur_char() != '\n'
                   && !t.done()) t.advance_char();
            
            if (t.cur_char() == '\n')
                t.advance_char();
            
            else if (t.cur_char() == '\r' && t.peek() == '\n') {
                t.advance_char();
                t.advance_char();
            }
        }

        // Everything else is assumed to be punctuation
        else if (is_punctuation(t.cur_char()))
            tokens.push_back(get_punctuation(t));

        else
            tokens.push_back(get_symbol(t));

        // Skip whitespace characters
        t.skip_whitespace();
    }

    return tokens;
}
