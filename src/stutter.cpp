#include <iostream>
#include <string>
#include <fstream>
#include <deque>

#include "lexer.h"

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

    Lexer lexer(content);
    std::deque<Token*> tokens = lexer.tokenize_stream();

    std::cout << "Tokens: [";

    for (auto token : tokens) {
        if (token->string_value.length() > 0)
            std::cout << token->string_value << ", ";
        else if (token->type == TOKEN_INT_LITERAL || token->type == TOKEN_FLOAT_LITERAL)
            std::cout << token->int_value << ", ";
        else
            std::cout << "TOKEN_TYPE_" << token->type << ", ";
    }

    std::cout << "]" << std::endl;
}