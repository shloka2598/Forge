#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "common/test_utils.h"

TEST_CASE("Lexer recognizes decimal integer literals") {
  auto num = GENERATE(std::string_view{"0"}, std::string_view{"1"}, std::string_view{"42"}, std::string_view{"12345"}, std::string_view{"999999"});

  DYNAMIC_SECTION("Decimal Integer Literal = " << num) {
    auto tokens = tokenize(num);

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == TokenType::INT_LET);
    REQUIRE(tokens[0].value.has_value());
    REQUIRE(*tokens[0].value == num);
  }
}

TEST_CASE("Lexer recognizes hexadecimal integer literals") {
  auto num = GENERATE(
      std::string_view{"0x0"},
      std::string_view{"0x1"},
      std::string_view{"0xA"},
      std::string_view{"0xFF"},
      std::string_view{"0x123ABC"},
      std::string_view{"0X10"},
      std::string_view{"0xabcdef"},
      std::string_view{"0XABCDEF"});

  DYNAMIC_SECTION("Hexadecimal Integer Literal = " << num) {
    auto tokens = tokenize(num);

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == TokenType::INT_LET);
    REQUIRE(tokens[0].value.has_value());
    REQUIRE(*tokens[0].value == num);
  }
}

TEST_CASE("Lexer recognizes octal integer literals") {
  auto num = GENERATE(
      std::string_view{"00"},
      std::string_view{"01"},
      std::string_view{"077"},
      std::string_view{"01234567"});

  DYNAMIC_SECTION("Octal Integer Literal = " << num) {
    auto tokens = tokenize(num);

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == TokenType::INT_LET);
    REQUIRE(tokens[0].value.has_value());
    REQUIRE(*tokens[0].value == num);
  }
}

TEST_CASE("Lexer recognizes double literals") {
  auto num = GENERATE(
      std::string_view{"0.1"},
      std::string_view{"3.14"},
      std::string_view{"123.456"},
      std::string_view{"999.999"});

  DYNAMIC_SECTION("Double Literal = " << num) {
    auto tokens = tokenize(num);

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == TokenType::DOUBLE_LET);
    REQUIRE(tokens[0].value.has_value());
    REQUIRE(*tokens[0].value == num);
  }
}

TEST_CASE("Lexer recognizes float literals") {
  auto [num, expected] = GENERATE(
      std::pair{std::string_view{"1.0f"}, std::string_view{"1.0"}},
      std::pair{std::string_view{"0.0f"}, std::string_view{"0.0"}},
      std::pair{std::string_view{"0.0F"}, std::string_view{"0.0"}},
      std::pair{std::string_view{"3.14f"}, std::string_view{"3.14"}},
      std::pair{std::string_view{"2.5F"}, std::string_view{"2.5"}});

  DYNAMIC_SECTION("Float Literal = " << num) {
    auto tokens = tokenize(num);

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == TokenType::FLOAT_LET);
    REQUIRE(tokens[0].value.has_value());
    REQUIRE(*tokens[0].value == expected);
  }
}