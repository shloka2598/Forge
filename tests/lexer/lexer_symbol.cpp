#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "common/test_utils.h"

TEST_CASE("Lexer recognizes punctuation") {
  auto [pun, expected] = GENERATE(
      std::pair<std::string_view, TokenType>{",", TokenType::COMMA},
      std::pair<std::string_view, TokenType>{";", TokenType::SEMI_COLON},
      std::pair<std::string_view, TokenType>{"...", TokenType::ELLIPSIS});

  DYNAMIC_SECTION("Punctuation = '" << pun << "'") {
    auto result = tokenize(pun);
    auto &tokens = result.tokens;

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == expected);
    REQUIRE_FALSE(tokens[0].value.has_value());
  }
}

TEST_CASE("Lexer recognizes delimiters") {
  auto [del, expected] = GENERATE(
      std::pair<std::string_view, TokenType>{"(", TokenType::PARENTHESIS_OPEN},
      std::pair<std::string_view, TokenType>{")", TokenType::PARENTHESIS_CLOSE},
      std::pair<std::string_view, TokenType>{"[", TokenType::SQUARE_BRACKETS_OPEN},
      std::pair<std::string_view, TokenType>{"]", TokenType::SQUARE_BRACKETS_CLOSE},
      std::pair<std::string_view, TokenType>{"{", TokenType::BRACES_OPEN},
      std::pair<std::string_view, TokenType>{"}", TokenType::BRACES_CLOSE});

  DYNAMIC_SECTION("Delimiter = '" << del << "'") {
    auto result = tokenize(del);
    auto &tokens = result.tokens;

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == expected);
    REQUIRE_FALSE(tokens[0].value.has_value());
  }
}