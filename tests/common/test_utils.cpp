#include <catch2/catch_test_macros.hpp>

#include "test_utils.h"

LexerResult tokenize(std::string_view source) {
  LexerResult result;

  Lexer lexer{std::string{source}, result.diagnostics};

  result.tokens = lexer.tokenize();

  return result;
}
