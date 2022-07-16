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

TEST_CASE("'=' test: inputs equal") {
    REQUIRE(eval_program<int>("(if (= 2 2) 1 0)") == 1);
}

TEST_CASE("'=' test: inputs unequal") {
    REQUIRE(eval_program<int>("(if (= 2 3) 1 0)") == 0);
}

TEST_CASE("'>' test: inputs equal") {
    REQUIRE(eval_program<int>("(if (> 2 2) 1 0)") == 0);
}

TEST_CASE("'>' test: left > right") {
    REQUIRE(eval_program<int>("(if (>= 3 2) 1 0)") == 1);
}

TEST_CASE("'>' test: left < right") {
    REQUIRE(eval_program<int>("(if (>= 2 3) 1 0)") == 0);
}

TEST_CASE("'>=' test: inputs equal") {
    REQUIRE(eval_program<int>("(if (>= 2 2) 1 0)") == 1);
}

TEST_CASE("'>=' test: left > right") {
    REQUIRE(eval_program<int>("(if (>= 3 2) 1 0)") == 1);
}

TEST_CASE("'>=' test: left < right") {
    REQUIRE(eval_program<int>("(if (>= 2 3) 1 0)") == 0);
}

