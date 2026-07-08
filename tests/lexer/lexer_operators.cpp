#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "common/test_utils.h"

TEST_CASE("Lexer recognizes arithmetic operators") {
  auto [op, expected] = GENERATE(
      std::pair<std::string_view, TokenType>{"+", TokenType::PLUS},
      std::pair<std::string_view, TokenType>{"-", TokenType::MINUS},
      std::pair<std::string_view, TokenType>{"*", TokenType::MULTIPLY},
      std::pair<std::string_view, TokenType>{"/", TokenType::DIVIDE},
      std::pair<std::string_view, TokenType>{"%", TokenType::MODULO},
      std::pair<std::string_view, TokenType>{"++", TokenType::PLUS_PLUS},
      std::pair<std::string_view, TokenType>{"--", TokenType::MINUS_MINUS});

  DYNAMIC_SECTION("Arithmetic Operator = '" << op << "'") {
    auto tokens = tokenize(op);

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == expected);
    REQUIRE_FALSE(tokens[0].value.has_value());
  }
}

TEST_CASE("Lexer recognizes assignment operators") {
  auto [op, expected] = GENERATE(
      std::pair<std::string_view, TokenType>{"=", TokenType::EQUALS},
      std::pair<std::string_view, TokenType>{"+=", TokenType::PLUS_EQUALS},
      std::pair<std::string_view, TokenType>{"-=", TokenType::MINUS_EQUALS},
      std::pair<std::string_view, TokenType>{"*=", TokenType::MULTIPLY_EQUALS},
      std::pair<std::string_view, TokenType>{"/=", TokenType::DIVIDE_EQUALS},
      std::pair<std::string_view, TokenType>{"%=", TokenType::MOD_EQUALS},
      std::pair<std::string_view, TokenType>{"<<=", TokenType::LEFT_SHIFT_EQUALS},
      std::pair<std::string_view, TokenType>{">>=", TokenType::RIGHT_SHIFT_EQUALS},
      std::pair<std::string_view, TokenType>{"&=", TokenType::AMPERSAND_EQUALS},
      std::pair<std::string_view, TokenType>{"|=", TokenType::PIPE_EQUALS},
      std::pair<std::string_view, TokenType>{"^=", TokenType::CARET_EQUALS});

  DYNAMIC_SECTION("Assignment Operator = '" << op << "'") {
    auto tokens = tokenize(op);

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == expected);
    REQUIRE_FALSE(tokens[0].value.has_value());
  }
}

TEST_CASE("Lexer recognizes comparison operators") {
  auto [op, expected] = GENERATE(
      std::pair<std::string_view, TokenType>{">", TokenType::GREATER_THAN},
      std::pair<std::string_view, TokenType>{"<", TokenType::SMALLER_THAN},
      std::pair<std::string_view, TokenType>{">=", TokenType::GREATER_THAN_EQUAL_THAN},
      std::pair<std::string_view, TokenType>{"<=", TokenType::SMALLER_THAN_EQUAL_THAN},
      std::pair<std::string_view, TokenType>{"==", TokenType::DOUBLE_EQUALS},
      std::pair<std::string_view, TokenType>{"!=", TokenType::NOT_EQUALS});

  DYNAMIC_SECTION("Comparison Operator = '" << op << "'") {
    auto tokens = tokenize(op);

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == expected);
    REQUIRE_FALSE(tokens[0].value.has_value());
  }
}

TEST_CASE("Lexer recognizes logical operators") {
  auto [op, expected] = GENERATE(
      std::pair<std::string_view, TokenType>{"!", TokenType::EXCLAMATION},
      std::pair<std::string_view, TokenType>{"&&", TokenType::DOUBLE_AMPERSAND},
      std::pair<std::string_view, TokenType>{"||", TokenType::DOUBLE_PIPE});

  DYNAMIC_SECTION("Logical Operator = '" << op << "'") {
    auto tokens = tokenize(op);

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == expected);
    REQUIRE_FALSE(tokens[0].value.has_value());
  }
}

TEST_CASE("Lexer recognizes bitwise operators") {
  auto [op, expected] = GENERATE(
      std::pair<std::string_view, TokenType>{"&", TokenType::AMPERSAND},
      std::pair<std::string_view, TokenType>{"|", TokenType::PIPE},
      std::pair<std::string_view, TokenType>{"^", TokenType::CARET},
      std::pair<std::string_view, TokenType>{"~", TokenType::TILDE},
      std::pair<std::string_view, TokenType>{"<<", TokenType::LEFT_SHIFT},
      std::pair<std::string_view, TokenType>{">>", TokenType::RIGHT_SHIFT});

  DYNAMIC_SECTION("Bitwise Operator = '" << op << "'") {
    auto tokens = tokenize(op);

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == expected);
    REQUIRE_FALSE(tokens[0].value.has_value());
  }
}

TEST_CASE("Lexer recognizes miscellaneous operators") {
  auto [op, expected] = GENERATE(
      std::pair<std::string_view, TokenType>{"?", TokenType::QUESTION_MARK},
      std::pair<std::string_view, TokenType>{":", TokenType::COLON},
      std::pair<std::string_view, TokenType>{".", TokenType::DOT},
      std::pair<std::string_view, TokenType>{"->", TokenType::ARROW});

  DYNAMIC_SECTION("Miscellaneous Operator = '" << op << "'") {
    auto tokens = tokenize(op);

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == expected);
    REQUIRE_FALSE(tokens[0].value.has_value());
  }
}