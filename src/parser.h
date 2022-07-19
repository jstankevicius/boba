#pragma once

#include <deque>
#include <unordered_map>
#include <vector>
#include "ast.h"
#include "token.h"

// The parser combines the roles of the traditional lexer and
// parser. Instead of the lexer and parser being used in two separate
// stages, the parser can both tokenize (lex) input strings
// (traditionally the job of the lexer) as well as produce ASTs
// (traditionally the job of the parser). This design has the
// advantage of allowing us to:
//
// 1. tokenize an entire file at once and then produce the AST, or
// 2. tokenize an expression and then produce the AST only for that
// expression
//
// This is clearly advantageous when we want to support both a REPL
// environment and a file-based interpreter environment. The idea here
// is to try and blur the line between the REPL and the file-based
// environment, which is a core feature of most Lisp languages.
//
// To combine the roles of lexer and parser into one, this parser
// utilizes an internal token queue. Calling tokenize_string() on a
// string will tokenize it and add the tokens to the queue; calling
// parse_expression() will consume tokens from the queue and return an
// AST.

class Parser {

private:
    std::deque<std::shared_ptr<Token>> tokens;
    
public:
    void tokenize_string(std::string &str);
    bool eof();
    std::unique_ptr<AST> parse_sexpr();
};
