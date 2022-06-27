#include <iostream>
#include <string>
#include <fstream>
#include <deque>

#include "lexer.h"
#include "parser.h"
#include "bytecode.h"
#include "runtime.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cerr << "Error: no input file" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::ifstream file;

    file.open(argv[1]);

    if (!file.is_open()) 
    {
        perror("Error: open()");
        exit(EXIT_FAILURE);
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        (std::istreambuf_iterator<char>()   ));

    Lexer lexer;
    Parser parser;
    auto tokens = lexer.tokenize_stream(content);
    std::cout << "Tokens:" << std::endl;
    for (auto& token : tokens) {
        std::cout << token->string_value << " ";
    }
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "Parsing:" << std::endl;
    auto ast = parser.parse_tokens(tokens);
    auto instructions = gen_bytecode(ast);
    std::cout << std::endl;
    print_instructions(instructions);
    Runtime runtime;
    runtime.execute(instructions);
}
