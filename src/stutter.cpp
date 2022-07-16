#include <iostream>
#include <string>
#include <fstream>
#include <deque>

#include "lexer.h"
#include "parser.h"
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

    Parser parser;
    Runtime runtime;

    parser.tokenize_string(content);
    while (!parser.eof()) {
        auto ast = parser.parse_sexpr();
        runtime.eval_ast(ast);
    }
}
