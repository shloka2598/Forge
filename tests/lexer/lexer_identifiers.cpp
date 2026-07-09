#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "common/test_utils.h"

TEST_CASE("Lexer recognizes identifiers") {
  auto identifier = GENERATE(std::string_view{"main"}, std::string_view{"hello"}, std::string_view{"abc"}, std::string_view{"variable"}, std::string_view{"_variable"}, std::string_view{"__GNUC__"}, std::string_view{"a123"}, std::string_view{"abc123"}, std::string_view{"_"}, std::string_view{"__"}, std::string_view{"a"}, std::string_view{"_1"}, std::string_view{"a_b_c"}, std::string_view{"this_is_a_very_long_identifier_name"});

  DYNAMIC_SECTION("Identifier = " << identifier) {
    auto result = tokenize(identifier);
    auto &tokens = result.tokens;

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == TokenType::IDENTIFIER);
    REQUIRE(tokens[0].value.has_value());
    REQUIRE(*tokens[0].value == identifier);
  }
}

TEST_CASE("Lexer distinguishes keywords from identifiers") {
  auto identifier = GENERATE(std::string_view{"return1"}, std::string_view{"ifElse"}, std::string_view{"whileLoop"}, std::string_view{"forEach"}, std::string_view{"structData"}, std::string_view{"int32"}, std::string_view{"doubleValue"});

  DYNAMIC_SECTION("Identifier = " << identifier) {
    auto result = tokenize(identifier);
    auto &tokens = result.tokens;

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == TokenType::IDENTIFIER);
    REQUIRE(tokens[0].value.has_value());
    REQUIRE(*tokens[0].value == identifier);
  }
}