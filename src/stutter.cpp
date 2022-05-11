#include <iostream>
#include <string.h>
#include <fstream>

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

    std::cout << "File contents:" << std::endl;
    std::cout << content << std::endl;

    Lexer* lexer = new Lexer(content);
    get_tokens(lexer);
}