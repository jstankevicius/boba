#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <deque>

#include "lexer.h"
#include "parser.h"
#include "bytecode.h"
#include "runtime.h"

int get_result_of_program(std::string program) {
    Lexer lexer;
    Parser parser;
    Runtime runtime;
    auto tokens = lexer.tokenize_stream(program);
    auto ast = parser.parse_tokens(tokens);
    auto instructions = gen_bytecode(ast);
    runtime.execute(instructions);
    return runtime.get_stack().back().get_int();
}

TEST_CASE("Addition: 2 + 3 == 5") {
    REQUIRE(get_result_of_program("(+ 2 3)") == 5);
}

TEST_CASE("Subtraction: 1 - 3 == -2") {
    REQUIRE(get_result_of_program("(- 1 3)") == -2);
}

TEST_CASE("Multiplication: 3 * 2 == 6") {
    REQUIRE(get_result_of_program("(* 3 2)") == 6);
}

TEST_CASE("Division: 4 / 2 == 2") {
    REQUIRE(get_result_of_program("(/ 4 2)") == 2);
}

TEST_CASE("Unary negation works as expected") {
    REQUIRE(get_result_of_program("(- 2)") == -2);
}

TEST_CASE("Multiple negations") {
    REQUIRE(get_result_of_program("(- (- (- (- 2))))") == 2);
}

TEST_CASE("Multiple additions") {
    REQUIRE(get_result_of_program("(+ 1 2 3 4)") == 10);
}

TEST_CASE("Multiple multiplications") {
    REQUIRE(get_result_of_program("(* 1 2 3 4)") == 24);
}

TEST_CASE("Simple compound operations: 2 + 3 * 4 == 14") {
    REQUIRE(get_result_of_program("(+ 2 (* 3 4))") == 14);
}

TEST_CASE("Simple compound operations: (12 / 4) + (3 - 2) == 4") {
    REQUIRE(get_result_of_program("(+ (/ 12 4) (- 3 2))") == 4);
}