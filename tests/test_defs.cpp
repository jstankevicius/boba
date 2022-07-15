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

TEST_CASE("Simple def") {
    REQUIRE(eval_program<int>("(def a 2) (def b 3) (+ a b)") == 5);
}
