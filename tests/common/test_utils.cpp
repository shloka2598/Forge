#include <catch2/catch_test_macros.hpp>

#include "parser/parser.h"
#include "semantics/semantics.h"
#include "test_utils.h"

LexerResult tokenize(std::string_view source) {
  LexerResult result;
  result.diagnostics = DiagnosticEngine(std::string{source});

  Lexer lexer{std::string{source}, result.diagnostics};

  result.tokens = lexer.tokenize();

  return result;
}

ParserResult parse(std::string_view source) {
  ParserResult result;
  result.diagnostics = DiagnosticEngine(std::string{source});

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

FunctionDeclStmt *get_function_decl(SemanticResult &result, std::string_view name) {
  for (auto &stmt : result.program.statements) {
    if (stmt->stmt_type() != StmtType::FUNCTION_DECL_STMT)
      continue;

    auto *fn = static_cast<FunctionDeclStmt *>(stmt.get());

    if (fn->declaration->name == name)
      return fn;
  }

  return nullptr;
}

FunctionDeclStmt *get_function_decl(ParserResult &result, std::string_view name) {
  for (auto &stmt : result.program.statements) {
    if (stmt->stmt_type() != StmtType::FUNCTION_DECL_STMT)
      continue;

    auto *fn = static_cast<FunctionDeclStmt *>(stmt.get());

    if (fn->declaration->name == name)
      return fn;
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
  result.diagnostics = DiagnosticEngine(std::string{source});

  Lexer lexer{std::string{source}, result.diagnostics};
  auto tokens = lexer.tokenize();

  Parser parser{tokens, result.program, result.diagnostics};
  parser.parse();

  result.semantics = std::make_unique<Semantics>(result.program, result.diagnostics);
  result.semantics->analyze();

  return result;
}

Expr *get_return_expr(SemanticResult &result) {
  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr != nullptr);

  return ret->expr_ptr.get();
}

Parameter *get_parameter(FunctionDeclStmt *fn, size_t index) {
  REQUIRE(fn != nullptr);
  REQUIRE(index < fn->declaration->parameters.size());

  return &fn->declaration->parameters[index];
}

GlobalVariableDeclStmt *get_global_variable_decl(SemanticResult &result, size_t index) {
  REQUIRE(index < result.program.statements.size());

  auto *decl = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[index].get());

  REQUIRE(decl != nullptr);

  return decl;
}