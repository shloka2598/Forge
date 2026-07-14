#pragma once

#include <catch2/catch_test_macros.hpp>

#include <string_view>
#include <vector>

#include "ast/program.h"
#include "diagnostics/DiagnosticEngine.h"
#include "lexer/lexer.h"
#include "semantics/semantics.h"

struct LexerResult {
  std::vector<Token> tokens;
  DiagnosticEngine diagnostics;
};

struct ParserResult {
  Program program;
  DiagnosticEngine diagnostics;
};

struct SemanticResult {
  Program program;
  DiagnosticEngine diagnostics;
  std::unique_ptr<Semantics> semantics;
  bool semantic_error = false;
};

ParserResult parse(std::string_view source);
LexerResult tokenize(std::string_view source);
SemanticResult analyze(std::string_view source);

FunctionDeclStmt *get_function_decl(ParserResult &);
FunctionDeclStmt *get_function_decl(SemanticResult &);
FunctionDeclStmt *get_function_decl(SemanticResult &, std::string_view);
FunctionDeclStmt *get_function_decl(ParserResult &, std::string_view);

ReturnStmt *get_return_stmt(ParserResult &);
ReturnStmt *get_return_stmt(SemanticResult &);

Expr *get_return_expr(SemanticResult &);

template <typename T>
T *get_stmt(FunctionDeclStmt *fn, size_t index) {
  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body != nullptr);
  REQUIRE(index < fn->declaration->body->statements.size());

  auto *stmt = dynamic_cast<T *>(fn->declaration->body->statements[index].get());

  REQUIRE(stmt != nullptr);

  return stmt;
}

template <typename T>
T *get_stmt(BlockStmt *block, size_t index) {
  REQUIRE(block != nullptr);
  REQUIRE(index < block->statements.size());

  auto *stmt = dynamic_cast<T *>(block->statements[index].get());

  REQUIRE(stmt != nullptr);

  return stmt;
}

template <typename T>
T *get_expr(ReturnStmt *ret) {
  REQUIRE(ret != nullptr);

  auto *expr = dynamic_cast<T *>(ret->expr_ptr.get());

  REQUIRE(expr != nullptr);

  return expr;
}

template <typename T>
T *get_expr(ExpressionStmt *stmt) {
  REQUIRE(stmt != nullptr);

  auto *expr = dynamic_cast<T *>(stmt->expr.get());

  REQUIRE(expr != nullptr);

  return expr;
}

template <typename T>
T *get_type(Expr *expr) {
  REQUIRE(expr != nullptr);
  REQUIRE(expr->type != nullptr);

  auto *type = dynamic_cast<T *>(expr->type);

  REQUIRE(type != nullptr);

  return type;
}

Parameter *get_parameter(FunctionDeclStmt *fn, size_t index);

template <typename T>
T *get_argument(FunctionCallExpr *call, size_t index) {
  REQUIRE(call != nullptr);
  REQUIRE(index < call->arguments.size());

  auto *arg = dynamic_cast<T *>(call->arguments[index].get());

  REQUIRE(arg != nullptr);

  return arg;
}