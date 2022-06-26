#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include <string>
#include "lexer.h"

TEST_CASE("empty program") {
    Lexer lexer;
    REQUIRE(true);
}