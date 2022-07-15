#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <deque>

#include "lexer.h"
#include "parser.h"
#include "runtime.h"
#include "util.h"
#include <memory>

TEST_CASE("Addition: 2 + 3 == 5") {
    REQUIRE(eval_program<int>("(+ 2 3)") == 5);
}

TEST_CASE("Subtraction: 1 - 3 == -2") {
    REQUIRE(eval_program<int>("(- 1 3)") == -2);
}

TEST_CASE("Multiplication: 3 * 2 == 6") {
    REQUIRE(eval_program<int>("(* 3 2)") == 6);
}

TEST_CASE("Division: 4 / 2 == 2") {
    REQUIRE(eval_program<int>("(/ 4 2)") == 2);
}

TEST_CASE("Unary negation works as expected") {
    REQUIRE(eval_program<int>("(- 2)") == -2);
}

TEST_CASE("Multiple negations") {
    REQUIRE(eval_program<int>("(- (- (- (- 2))))") == 2);
}

TEST_CASE("Simple compound operations: 2 + 3 * 4 == 14") {
    REQUIRE(eval_program<int>("(+ 2 (* 3 4))") == 14);
}

TEST_CASE("Simple compound operations: (12 / 4) + (3 - 2) == 4") {
    REQUIRE(eval_program<int>("(+ (/ 12 4) (- 3 2))") == 4);
}
