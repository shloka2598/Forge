#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

TEST_CASE("Lexer ignores single-line comments") {
  std::string_view source{"// a comment\n return"};
  auto tokens = tokenize(source);

  REQUIRE(tokens.size() == 1);
  REQUIRE(tokens[0].tokentype == TokenType::RETURN);
  REQUIRE_FALSE(tokens[0].value.has_value());
}

TEST_CASE("Lexer ignores multi-line comments") {
  std::string_view source{"/* a comment*/ return"};
  auto tokens = tokenize(source);

  REQUIRE(tokens.size() == 1);
  REQUIRE(tokens[0].tokentype == TokenType::RETURN);
  REQUIRE_FALSE(tokens[0].value.has_value());
}

TEST_CASE("Lexer ignores comments between tokens") {
  std::string_view source{"return /* a comment*/ 10;"};
  auto tokens = tokenize(source);

  REQUIRE(tokens.size() == 3);

  REQUIRE(tokens[0].tokentype == TokenType::RETURN);
  REQUIRE_FALSE(tokens[0].value.has_value());

  REQUIRE(tokens[1].tokentype == TokenType::INT_LET);
  REQUIRE(tokens[1].value.has_value());
  REQUIRE(*tokens[1].value == std::string_view{"10"});

  REQUIRE(tokens[2].tokentype == TokenType::SEMI_COLON);
  REQUIRE_FALSE(tokens[2].value.has_value());
}

TEST_CASE("Lexer ignores multiple comments") {
  std::string_view source{"// single line comment \n /* multi line comment */ 1008;"};
  auto tokens = tokenize(source);

  REQUIRE(tokens.size() == 2);

  REQUIRE(tokens[0].tokentype == TokenType::INT_LET);
  REQUIRE(tokens[0].value.has_value());
  REQUIRE(*tokens[0].value == std::string_view{"1008"});

  REQUIRE(tokens[1].tokentype == TokenType::SEMI_COLON);
  REQUIRE_FALSE(tokens[1].value.has_value());
}

TEST_CASE("Lexer ignores empty comments") {
  std::string_view source{"//  \n /* */ return"};
  auto tokens = tokenize(source);

  REQUIRE(tokens.size() == 1);

  REQUIRE(tokens[0].tokentype == TokenType::RETURN);
  REQUIRE_FALSE(tokens[0].value.has_value());
}

TEST_CASE("Lexer ignores comment-only input") {
  std::string_view source{"//  \n /* */"};
  auto tokens = tokenize(source);

  REQUIRE(tokens.empty());
}

TEST_CASE("Lexer ignores single-line comments after valid input") {
  std::string_view source{"return 10; // sinlge line comment"};
  auto tokens = tokenize(source);

  REQUIRE(tokens.size() == 3);

  REQUIRE(tokens[0].tokentype == TokenType::RETURN);
  REQUIRE_FALSE(tokens[0].value.has_value());

  REQUIRE(tokens[1].tokentype == TokenType::INT_LET);
  REQUIRE(tokens[1].value.has_value());
  REQUIRE(*tokens[1].value == std::string_view{"10"});

  REQUIRE(tokens[2].tokentype == TokenType::SEMI_COLON);
  REQUIRE_FALSE(tokens[2].value.has_value());
}

TEST_CASE("Lexer ignores multi-line comments after valid input") {
  std::string_view source{"return 10; /* multi-line comment */"};
  auto tokens = tokenize(source);

  REQUIRE(tokens.size() == 3);

  REQUIRE(tokens[0].tokentype == TokenType::RETURN);
  REQUIRE_FALSE(tokens[0].value.has_value());

  REQUIRE(tokens[1].tokentype == TokenType::INT_LET);
  REQUIRE(tokens[1].value.has_value());
  REQUIRE(*tokens[1].value == std::string_view{"10"});

  REQUIRE(tokens[2].tokentype == TokenType::SEMI_COLON);
  REQUIRE_FALSE(tokens[2].value.has_value());
}

TEST_CASE("Lexer recognizes divide operator after comments") {
  std::string_view source{"/* comment *//2"};
  auto tokens = tokenize(source);

  REQUIRE(tokens.size() == 2);

  REQUIRE(tokens[0].tokentype == TokenType::DIVIDE);
  REQUIRE_FALSE(tokens[0].value.has_value());

  REQUIRE(tokens[1].tokentype == TokenType::INT_LET);
  REQUIRE(tokens[1].value.has_value());
  REQUIRE(*tokens[1].value == std::string_view{"2"});
}

TEST_CASE("Lexer ignores adjacent multi-line comments") {
  std::string_view source{"/**//*comment*/return"};
  auto tokens = tokenize(source);

  REQUIRE(tokens.size() == 1);
  REQUIRE(tokens[0].tokentype == TokenType::RETURN);
  REQUIRE_FALSE(tokens[0].value.has_value());
}

TEST_CASE("Lexer resumes after single-line comments followed by blank lines") {
  std::string_view source{"// comment\n\n\nreturn"};
  auto tokens = tokenize(source);

  REQUIRE(tokens.size() == 1);
  REQUIRE(tokens[0].tokentype == TokenType::RETURN);
  REQUIRE_FALSE(tokens[0].value.has_value());
}