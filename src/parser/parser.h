#pragma once

#include <memory>
#include <optional>
#include <unordered_set>
#include <vector>

#include "../ast/exprs.h"
#include "../ast/program.h"
#include "../ast/stmts.h"
#include "../lexer/lexer.h"
#include "diagnostics/DiagnosticEngine.h"

class Parser {
private:
  // Core Parser State
  size_t _index = 0;
  const std::vector<Token> _tokens;

  DiagnosticEngine &diagnostics;
  bool has_error = false;

  Program &program;

  std::unordered_set<std::string> typedef_names;

  std::unique_ptr<StructDecl> pending_struct;
  std::unique_ptr<UnionDecl> pending_union;
  std::unique_ptr<EnumDecl> pending_enum;

  inline bool isTypedefName() const {
    return (peek() && peek()->tokentype == TokenType::IDENTIFIER && typedef_names.contains(peek()->value.value()));
  }

  // Tokens
  [[nodiscard]] std::optional<Token> peek(int ahead = 0) const;
  std::optional<Token> consume();
  bool match(TokenType expected);

  // Error Handling
  void error(const std::string &msg, std::optional<Token>);

  // Error recovery
  void recoverStatement();
  void recoverTopLevel();
  void recoverUntil(TokenType);
  void recoverParameter();
  void recoverExpression();

  template <typename T>
  bool recoverIfFailed(const std::unique_ptr<T> &node, TokenType sync) {
    if (node || !has_error) {
      return false;
    }

    recoverUntil(sync);
    return true;
  }

  // Type Utilities
  bool isDatatype(TokenType) const;
  bool isCastExpression() const;
  bool isAssignable(const Expr *expr) const;

  ParsedType parseDatatype();
  void parsePointerSuffix(ParsedType &type);

  TokenType compound_to_binary(TokenType);
  bool looksLikeFunction() const;

  // Top-Level Parsing
  void parseTopLevelDeclaration();

  std::unique_ptr<FunctionDecl> parseFunction();
  std::optional<std::vector<Parameter>> parseParameters();
  std::unique_ptr<GlobalVariableDecl> parseGlobalVariable();

  std::unique_ptr<StructDecl> parseStructDeclaration(std::string name, bool expect_semicolon = true);
  std::unique_ptr<UnionDecl> parseUnionDeclaration(std::string name, bool expect_semicolon = true);
  std::unique_ptr<EnumDecl> parseEnumDeclaration(std::string name, bool expect_semicolon = true);

  // Array Initializers
  ArrayInitializer parseArrayInitializer();

  // Statement Parsing
  std::unique_ptr<Stmt> parse_stmt();

  std::unique_ptr<ReturnStmt> parseReturnStmt();
  std::unique_ptr<VariableDeclarationStmt> parseVariableDeclarationStmt();
  std::unique_ptr<ExpressionStmt> parseExpressionStmt();

  std::unique_ptr<IfStmt> parseIfStmt();
  std::unique_ptr<WhileStmt> parseWhileStmt();
  std::unique_ptr<DoWhileStmt> parseDoWhileStmt();
  std::unique_ptr<ForStmt> parseForStmt();
  std::unique_ptr<SwitchStmt> parseSwitchStmt();

  std::unique_ptr<StructDeclarationStmt> parseStructDeclarationStmt();
  std::unique_ptr<UnionDeclarationStmt> parseUnionDeclarationStmt();
  std::unique_ptr<EnumDeclarationStmt> parseEnumDeclarationStmt();

  std::unique_ptr<Stmt> parseTypedefDeclarationStmt();

  std::unique_ptr<Stmt> parseForInitStmt();
  std::unique_ptr<Stmt> parseForUpdateStmt();

  std::unique_ptr<Stmt> parseBreakStmt();
  std::unique_ptr<Stmt> parseContinueStmt();

  std::unique_ptr<BlockStmt> parseBlock();
  std::unique_ptr<BlockStmt> parseStatementOrBlock();

  // Expression Parsing
  std::unique_ptr<Expr> parseExpr();
  std::unique_ptr<Expr> parseComma();
  std::unique_ptr<Expr> parseAssignment();
  std::unique_ptr<Expr> parseConditional();
  std::unique_ptr<Expr> parseLogicalOr();
  std::unique_ptr<Expr> parseLogicalAnd();
  std::unique_ptr<Expr> parseComparison();
  std::unique_ptr<Expr> parseShift();

  std::unique_ptr<Expr> parseBitwiseOr();
  std::unique_ptr<Expr> parseBitwiseXor();
  std::unique_ptr<Expr> parseBitwiseAnd();
  std::unique_ptr<Expr> parseEquality();
  std::unique_ptr<Expr> parseRelational();

  std::unique_ptr<Expr> parseAdditionAndSubtraction();
  std::unique_ptr<Expr> parseMultiplicationAndDivision();

  std::unique_ptr<Expr> parseUnaryAndCasting();
  std::unique_ptr<Expr> parsePostfix();
  std::unique_ptr<Expr> parsePrimary();

  // Function Call Helpers
  std::optional<std::vector<std::unique_ptr<Expr>>> parseArguments();

public:
  explicit Parser(const std::vector<Token> &tokens, Program &_prog, DiagnosticEngine &engine)
      : _tokens{tokens}, diagnostics{engine}, program(_prog) {
  }

  void parse();
};