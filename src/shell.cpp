#include "iostream"
#include "string.h"

int main() {
    std::cout << "This is the Stutter shell" << std::endl;
    std::string line;
    
    std::cout << "> ";
    while (getline(std::cin, line)) {
        std::cout << line << std::endl;
        std::cout << "> ";
    }
}