#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "common/test_utils.h"

TEST_CASE("Lexer ignores leading whitespace") {
  auto source = GENERATE(
      std::string_view{" return"},
      std::string_view{"\treturn"},
      std::string_view{"\nreturn"},
      std::string_view{"\rreturn"},
      std::string_view{"\r\nreturn"});

  DYNAMIC_SECTION("Leading whitespace in: " << source) {
    auto tokens = tokenize(source);

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == TokenType::RETURN);
    REQUIRE_FALSE(tokens[0].value.has_value());
  }
}

TEST_CASE("Lexer ignores trailing whitespace") {
  auto source = GENERATE(
      std::string_view{"return "},
      std::string_view{"return\t"},
      std::string_view{"return\n"},
      std::string_view{"return\r"},
      std::string_view{"return\r\n"});

  DYNAMIC_SECTION("Trailing whitespace in: " << source) {
    auto tokens = tokenize(source);

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == TokenType::RETURN);
    REQUIRE_FALSE(tokens[0].value.has_value());
  }
}

TEST_CASE("Lexer separates tokens with whitespace") {
  std::string_view source{"return 10 ;"};

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

TEST_CASE("Lexer ignores consecutive whitespace") {
  std::string_view source{"return      \t\t\n\n10        ;"};

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

TEST_CASE("Lexer ignores whitespace-only input") {
  auto source = GENERATE(
      std::string_view{" "},
      std::string_view{"\t"},
      std::string_view{"\n"},
      std::string_view{"\r"},
      std::string_view{"\r\n"},
      std::string_view{"   \t\n\r\n"});

  DYNAMIC_SECTION("Whitespace only: " << source) {
    auto tokens = tokenize(source);

    REQUIRE(tokens.empty());
  }
}

TEST_CASE("Lexer handles mixed whitespace") {
  std::string_view source{"\t return \n 10 \r\n ;"};

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