#include "parser.h"

std::unique_ptr<ReturnStmt> Parser::parseReturnStmt() {
  if (peek() && peek()->tokentype == TokenType::SEMI_COLON) {
    consume();
    return std::make_unique<ReturnStmt>(nullptr);
  }

  std::unique_ptr<Expr> expr = parseExpr();

  if (!expr) {
    error("Expected expression after return");
    return nullptr;
  }

  if (!match(TokenType::SEMI_COLON)) {
    return nullptr;
  }

  return std::make_unique<ReturnStmt>(std::move(expr));
}

std::unique_ptr<VariableDeclarationStmt> Parser::parseVariableDeclarationStmt() {

  ParsedType type = parseDatatype();

  if (type.datatype == DataType::INVALID) {
    return nullptr;
  }

  if (pending_struct) {
    program.statements.push_back(std::make_unique<StructDeclarationStmt>(std::move(pending_struct)));
  }

  if (pending_union) {
    program.statements.push_back(std::make_unique<UnionDeclarationStmt>(std::move(pending_union)));
  }

  if (pending_enum) {
    program.statements.push_back(std::make_unique<EnumDeclarationStmt>(std::move(pending_enum)));
  }

  parsePointerSuffix(type);

  if (!peek() || peek()->tokentype != TokenType::IDENTIFIER) {
    error("Expected identifier");
    return nullptr;
  }

  std::string var_name = consume()->value.value();

  while (peek() && peek()->tokentype == TokenType::SQUARE_BRACKETS_OPEN) {
    consume();
    size_t size = 0;
    if (peek() && peek()->tokentype == TokenType::INT_LET) {
      size = std::stoull(consume()->value.value());
    }
    type.dimensions.push_back(size);
    if (!match(TokenType::SQUARE_BRACKETS_CLOSE)) {
      return nullptr;
    }
  }

  std::unique_ptr<Expr> expr_ptr;

  std::optional<ArrayInitializer> array_initializer;

  if (peek() && peek()->tokentype == TokenType::EQUALS) {
    consume(); // =
    if (!type.dimensions.empty() && peek() && peek()->tokentype == TokenType::BRACES_OPEN) {
      array_initializer = parseArrayInitializer();
    } else {
      expr_ptr = parseExpr();
      if (!expr_ptr) {
        error("Expected initializer expression");
        return nullptr;
      }
    }
  }

  if (!match(TokenType::SEMI_COLON)) {
    return nullptr;
  }

  return std::make_unique<VariableDeclarationStmt>(std::move(type), std::move(var_name), std::move(expr_ptr), std::move(array_initializer));
}

std::unique_ptr<ExpressionStmt> Parser::parseExpressionStmt() {
  std::unique_ptr<Expr> expr = parseExpr();

  if (!expr) {
    error("Expected expression");
    return nullptr;
  }

  if (!match(TokenType::SEMI_COLON)) {
    return nullptr;
  }

  return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::parse_stmt() {
  if (!peek()) {
    return nullptr;
  }
  TokenType tok = peek()->tokentype;

  if (tok == TokenType::BRACES_OPEN) {
    return parseBlock();
  }

  if (tok == TokenType::RETURN) {
    consume();
    return parseReturnStmt();
  }

  if (tok == TokenType::UNION || tok == TokenType::STRUCT || tok == TokenType::ENUM) {
    return parseVariableDeclarationStmt();
  }

  if (tok == TokenType::TYPEDEF) {
    return parseTypedefDeclarationStmt();
  }

  if (isDatatype(tok) || isTypedefName()) {
    return parseVariableDeclarationStmt();
  }

  if (tok == TokenType::IDENTIFIER ||
      tok == TokenType::INT_LET ||
      tok == TokenType::FLOAT_LET ||
      tok == TokenType::DOUBLE_LET ||
      tok == TokenType::CHAR_LET ||
      tok == TokenType::STRING_LET ||
      tok == TokenType::PLUS_PLUS ||
      tok == TokenType::MINUS_MINUS ||
      tok == TokenType::PARENTHESIS_OPEN ||
      tok == TokenType::PLUS ||
      tok == TokenType::MINUS ||
      tok == TokenType::EXCLAMATION ||
      tok == TokenType::TILDE ||
      tok == TokenType::MULTIPLY ||
      tok == TokenType::AMPERSAND || tok == TokenType::SIZEOF) {

    return parseExpressionStmt();
  }

  if (tok == TokenType::IF) {
    return parseIfStmt();
  }

  if (tok == TokenType::WHILE) {
    return parseWhileStmt();
  }

  if (tok == TokenType::DO) {
    return parseDoWhileStmt();
  }

  if (tok == TokenType::FOR) {
    return parseForStmt();
  }

  if (tok == TokenType::BREAK) {
    return parseBreakStmt();
  }

  if (tok == TokenType::CONTINUE) {
    return parseContinueStmt();
  }

  if (tok == TokenType::SWITCH) {
    return parseSwitchStmt();
  }

  if (tok == TokenType::SEMI_COLON) {
    consume();
    return std::make_unique<EmptyStmt>();
  }

  error("Unknown statement");
  return nullptr;
}

std::unique_ptr<Stmt> Parser::parseBreakStmt() {
  consume(); // break
  if (!match(TokenType::SEMI_COLON)) {
    return nullptr;
  }
  return std::make_unique<BreakStmt>();
}

std::unique_ptr<Stmt> Parser::parseContinueStmt() {
  consume(); // continue
  if (!match(TokenType::SEMI_COLON)) {
    return nullptr;
  }
  return std::make_unique<ContinueStmt>();
}

std::unique_ptr<StructDeclarationStmt> Parser::parseStructDeclarationStmt() {
  consume(); // struct

  std::string name;

  if (peek() && peek()->tokentype == TokenType::IDENTIFIER) {
    name = consume()->value.value();
  }

  auto decl = parseStructDeclaration(name);
  if (!decl) {
    return nullptr;
  }

  return std::make_unique<StructDeclarationStmt>(std::move(decl));
}

std::unique_ptr<UnionDeclarationStmt> Parser::parseUnionDeclarationStmt() {
  consume(); // union
  std::string name;
  if (peek() && peek()->tokentype == TokenType::IDENTIFIER) {
    name = consume()->value.value();
  }

  auto decl = parseUnionDeclaration(name);

  if (!decl) {
    return nullptr;
  }

  return std::make_unique<UnionDeclarationStmt>(std::move(decl));
}

std::unique_ptr<EnumDeclarationStmt> Parser::parseEnumDeclarationStmt() {
  consume(); // enum
  std::string name;
  if (peek() && peek()->tokentype == TokenType::IDENTIFIER) {
    name = consume()->value.value();
  }
  auto decl = parseEnumDeclaration(name);

  if (!decl) {
    return nullptr;
  }

  return std::make_unique<EnumDeclarationStmt>(std::move(decl));
}

std::unique_ptr<Stmt> Parser::parseTypedefDeclarationStmt() {
  consume(); // typedef

  ParsedType type = parseDatatype();

  if (type.datatype == DataType::INVALID) {
    return nullptr;
  }

  parsePointerSuffix(type);

  if (!peek() || peek()->tokentype != TokenType::IDENTIFIER) {
    error("Expected typedef name");
    return nullptr;
  }

  std::string alias_name = consume()->value.value();

  while (peek() && peek()->tokentype == TokenType::SQUARE_BRACKETS_OPEN) {
    consume(); // [
    size_t size = 0;
    if (peek() && peek()->tokentype == TokenType::INT_LET) {
      size = std::stoull(consume()->value.value());
    }
    type.dimensions.push_back(size);
    if (!match(TokenType::SQUARE_BRACKETS_CLOSE)) {
      return nullptr;
    }
  }

  if (peek() && peek()->tokentype == TokenType::EQUALS) {
    error("typedef cannot have an initializer");
    return nullptr;
  }

  if (!match(TokenType::SEMI_COLON)) {
    return nullptr;
  }

  typedef_names.insert(alias_name);

  auto stmt = std::make_unique<TypedefDeclarationStmt>(std::move(type), std::move(alias_name));

  if (pending_struct) {
    stmt->anonymous_struct = std::move(pending_struct);
  }

  if (pending_union) {
    stmt->anonymous_union = std::move(pending_union);
  }

  if (pending_enum) {
    stmt->anonymous_enum = std::move(pending_enum);
  }

  return stmt;
}