#include "catch.hpp"
#include "lexer.h"

TEST_CASE("testy test 2 :)") {
    std::string program = "";
    Lexer lexer;
    REQUIRE(lexer.tokenize_stream(program).size() == 1);
}