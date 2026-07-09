#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "common/test_utils.h"

TEST_CASE("Lexer recognizes keywords") {
  auto [keyword, expected] = GENERATE(
      std::pair{"return", TokenType::RETURN},
      std::pair{"if", TokenType::IF},
      std::pair{"else", TokenType::ELSE},
      std::pair{"while", TokenType::WHILE},
      std::pair{"for", TokenType::FOR},
      std::pair{"switch", TokenType::SWITCH},
      std::pair{"case", TokenType::CASE},
      std::pair{"default", TokenType::DEFAULT},
      std::pair{"break", TokenType::BREAK},
      std::pair{"do", TokenType::DO},
      std::pair{"continue", TokenType::CONTINUE},
      std::pair{"goto", TokenType::GOTO},
      std::pair{"sizeof", TokenType::SIZEOF},
      std::pair{"auto", TokenType::AUTO},
      std::pair{"extern", TokenType::EXTERN},
      std::pair{"static", TokenType::STATIC},
      std::pair{"register", TokenType::REGISTER},
      std::pair{"inline", TokenType::INLINE},
      std::pair{"struct", TokenType::STRUCT},
      std::pair{"union", TokenType::UNION},
      std::pair{"enum", TokenType::ENUM},
      std::pair{"typedef", TokenType::TYPEDEF},
      std::pair{"const", TokenType::CONST},
      std::pair{"volatile", TokenType::VOLATILE},
      std::pair{"restrict", TokenType::RESTRICT},
      std::pair{"void", TokenType::DATATYPE_VOID},
      std::pair{"char", TokenType::DATATYPE_CHAR},
      std::pair{"int", TokenType::DATATYPE_INT},
      std::pair{"float", TokenType::DATATYPE_FLOAT},
      std::pair{"double", TokenType::DATATYPE_DOUBLE},
      std::pair{"short", TokenType::SHORT},
      std::pair{"long", TokenType::LONG},
      std::pair{"signed", TokenType::SIGNED},
      std::pair{"unsigned", TokenType::UNSIGNED});

  DYNAMIC_SECTION("Keyword = " << keyword) {
    auto result = tokenize(keyword);
    auto &tokens = result.tokens;

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].tokentype == expected);
    REQUIRE_FALSE(tokens[0].value.has_value());
  }
}
