#include <vector>
#include <string.h>

// Macros:
#define is_alpha(c) (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || c == '_')
#define is_numeric(c) ('0' <= c && c <= '9')
#define is_whitespace(c) (c == ' ' || c == '\t' || c == '\n' || c == '\r')
#define is_alphanumeric(c) (is_alpha(c) || is_numeric(c))
#define is_operator(c) (c == '=' || c == '<' || c == '>' || c == '+' || c == '-' || c == '*' || c == '/')

enum Token_Type {
    IDENTIFIER,
    KEYWORD,

    INT_LITERAL,
    FLOAT_LITERAL,
    STR_LITERAL,
    
    OPERATOR,
    OTHER,
    ERROR
};


struct Token {

    int line_num, col_num;

    Token_Type type = OTHER;

    // TODO: more types eventually?
    union {
        std::string string_value = "";
        int int_value;
        float float_value;
    };

};

struct Lexer {
    int stream_idx = 0;

    int line_num = 1;
    int col_num = 1;

    std::string stream;
    std::vector<Token*> tokens;

    Lexer(std::string stream);
    char cur_char();
    void advance_char();
    char lookahead_char(int lookahead);
    void skip_whitespace();
    bool done();
};

std::vector<Token *> get_tokens(Lexer* lexer);

