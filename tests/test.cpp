#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "lexer.h"

TEST_CASE("testy test :)") {
    std::string program = "";
    Lexer lexer;
    REQUIRE(lexer.tokenize_stream(program).size() == 1);
}