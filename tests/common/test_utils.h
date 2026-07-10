#pragma once

#include <string_view>
#include <vector>

#include "ast/program.h"
#include "diagnostics/DiagnosticEngine.h"
#include "lexer/lexer.h"

struct LexerResult {
  std::vector<Token> tokens;
  DiagnosticEngine diagnostics;
};

struct ParserResult {
  Program program;
  DiagnosticEngine diagnostics;
};

ParserResult parse(std::string_view source);
LexerResult tokenize(std::string_view source);

FunctionDeclStmt *get_function_decl(ParserResult &);
ReturnStmt *get_return_stmt(ParserResult &);