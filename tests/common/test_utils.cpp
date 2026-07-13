#include <catch2/catch_test_macros.hpp>

#include "parser/parser.h"
#include "semantics/semantics.h"
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

  Parser parser{tokens, result.program, result.diagnostics};
  parser.parse();

  return result;
}

FunctionDeclStmt *get_function_decl(ParserResult &result) {
  for (auto &stmt : result.program.statements) {
    if (stmt->stmt_type() == StmtType::FUNCTION_DECL_STMT) {
      return static_cast<FunctionDeclStmt *>(stmt.get());
    }
  }

  return nullptr;
}

FunctionDeclStmt *get_function_decl(SemanticResult &result) {
  for (auto &stmt : result.program.statements) {
    if (stmt->stmt_type() == StmtType::FUNCTION_DECL_STMT) {
      return static_cast<FunctionDeclStmt *>(stmt.get());
    }
  }

  return nullptr;
}

ReturnStmt *get_return_stmt(ParserResult &result) {
  FunctionDeclStmt *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 1);

  auto *ret = dynamic_cast<ReturnStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(ret != nullptr);

  return ret;
}

ReturnStmt *get_return_stmt(SemanticResult &result) {
  FunctionDeclStmt *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 1);

  auto *ret = dynamic_cast<ReturnStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(ret != nullptr);

  return ret;
}

SemanticResult analyze(std::string_view source) {
  SemanticResult result;

  Lexer lexer{std::string{source}, result.diagnostics};
  auto tokens = lexer.tokenize();

  Parser parser{tokens, result.program, result.diagnostics};
  parser.parse();

  result.semantics = std::make_unique<Semantics>(result.program);
  result.semantics->analyze();

  result.semantic_error = result.semantics->error_occured();

  return result;
}

Expr *get_return_expr(SemanticResult &result) {
  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr != nullptr);

  return ret->expr_ptr.get();
}