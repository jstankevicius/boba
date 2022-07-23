#include <iostream>
#include <string>
#include <fstream>

#include "lexer.h"
#include "parser.h"
#include "runtime.h"
#include <memory>


class TestRunner {

private:
    Parser parser;
    Runtime runtime;

public:

    void tokenize_string(std::string str) {
        parser.tokenize_string(str);
    }
    // Evaluates a single expression (if multiple expressions exist in
    // the input string, only the top one is evaluated
    std::shared_ptr<Value> eval_expr() {
        auto ast = parser.parse_sexpr();
        return runtime.eval_ast(ast);
    }
};


int main() {
    TestRunner t;

    std::ifstream test_file("tests/arithmetic.test");
    std::string content = "";
    
    std::vector<std::string> expected_outputs;
    std::vector<std::string> section_names;
    
    std::string line;

    // Prefix that precedes the expected output:
    const std::string prefix = ";;=>";
    const std::string name_prefix = ";;name=";
        
    // Get expected outputs from file:
    while (std::getline(test_file, line)) {
        content += line;
        content += '\n';
        if (line.substr(0, prefix.size()) == prefix)
            expected_outputs.push_back(line.substr(prefix.size()));

        if (line.substr(0, name_prefix.size()) == name_prefix)
            section_names.push_back(line.substr(name_prefix.size()));
    }

    assert(expected_outputs.size() == section_names.size());
    t.tokenize_string(content);
    
    int successes = 0;
    int failures = 0;
    
    for (int i = 0; i < expected_outputs.size(); i++) {
        std::cout << "Running " << section_names[i] << "... ";
        auto result = t.eval_expr()->to_string();
        if (result == expected_outputs[i]) {
            std::cout << "OK\n";
            successes++;
        }
        else {
            std::cout << "failed (expected '" << expected_outputs[i]
                      << "' but got '" << result << "')\n";
            failures++;
        }
    }

    printf("===================================================\n");
    printf("Test run complete: Successes: %d, failures: %d, "
           "total: %d\n", successes, failures, successes + failures);

    if (failures > 0)
        exit(-1);
}
