#pragma once

#include <string_view>
#include <vector>

#include "diagnostics/DiagnosticEngine.h"
#include "lexer/lexer.h"

struct LexerResult {
  std::vector<Token> tokens;
  DiagnosticEngine diagnostics;
};

LexerResult tokenize(std::string_view source);
