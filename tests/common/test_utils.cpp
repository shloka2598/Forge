#include <catch2/catch_test_macros.hpp>

#include "parser/parser.h"
#include "test_utils.h"

LexerResult tokenize(std::string_view source) {
  LexerResult result;

  Lexer lexer{std::string{source}, result.diagnostics};

  result.tokens = lexer.tokenize();

  return result;
}

ParserResult parse(std::string_view source) {
  ParserResult result;

  Lexer lexer{std::string{source}, result.diagnostics};
  auto tokens = lexer.tokenize();

  Parser parser{tokens, result.program};
  parser.parse();

  return result;
}
