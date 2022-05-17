#include <iostream>
#include <string.h>
#include "error.h"

void error(Token* token, std::string message) {

    std::string stream = *token->stream; // probably bad practice
    std::cout << "ERROR: line " << token->line_num << ", column " << token->col_num << std::endl;

    int i = 0;
    for (int line = 1; line < token->line_num; line++) {
        while (stream[i] != '\n' && stream[i] != '\r') i++;

        // Handle \r\n
        if (stream[i] == 'r' && (i + 1 < stream.length() ? stream[i + 1] : -1) == '\n') i++;
        
        // We've ended on top of a \n, so move by one more char.
        i++;
    }

    // Print this entire current line:
    std::string line;

    while (stream[i] != '\n' && stream[i] != '\r') {
        line += stream[i];
        i++;
    }
    std::cout << line << std::endl;

    // Print the message under the line:
    for (int i = 1; i < token->col_num; i++) std::cout << " ";
    std::cout << "^ " << message << std::endl;
    
    exit(-1);
}