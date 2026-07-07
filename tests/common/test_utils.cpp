#include <catch2/catch_test_macros.hpp>

#include "test_utils.h"

std::vector<Token> tokenize(std::string_view source) {
  Lexer lexer{std::string{source}};

  return lexer.tokenize();
}
