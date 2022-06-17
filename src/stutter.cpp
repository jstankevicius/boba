#include <iostream>
#include <string>
#include <fstream>
#include <deque>

#include "lexer.h"
#include "parser.h"

int main() 
{
    std::ifstream file;

    file.open("file.stt");

    if (!file.is_open()) 
    {
        perror("Error: open()");
        exit(EXIT_FAILURE);
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        (std::istreambuf_iterator<char>()   ));

    Lexer lexer;
    Parser parser;
    std::deque<Token*> tokens = lexer.tokenize_stream(content);
    std::cout << "Tokens:" << std::endl;
    for (auto token : tokens) {
        std::cout << token->string_value << " ";
    }
    std::cout << std::endl;

    std::cout << "Parsing:" << std::endl;
    parser.parse_tokens(tokens);
}
