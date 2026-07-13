#include "parser.h"

std::unique_ptr<ReturnStmt> Parser::parseReturnStmt() {
  if (peek() && peek()->tokentype == TokenType::SEMI_COLON) {
    consume();
    return std::make_unique<ReturnStmt>(nullptr);
  }

  std::unique_ptr<Expr> expr = parseExpr();

  if (!expr) {
    error("Expected an expression after 'return'.", peek());
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
    error("Expected a variable name.", peek());
    return nullptr;
  }

  std::string var_name = consume()->value.value();

  while (peek() && peek()->tokentype == TokenType::SQUARE_BRACKETS_OPEN) {
    consume();
    size_t size = 0;
    if (peek() && peek()->tokentype == TokenType::INT_LET) {
      size = std::stoull(consume()->value.value());
    } else if (!peek()) {
      error("Expected an integer array size before end of file.", std::nullopt);
      return nullptr;
    } else if (peek() && peek()->tokentype != TokenType::SQUARE_BRACKETS_CLOSE) {
      error("Expected an integer array size.", peek());
      return nullptr;
    }
    type.dimensions.push_back(size);
    if (!match(TokenType::SQUARE_BRACKETS_CLOSE)) {
      return nullptr;
    }
  }

  std::unique_ptr<Expr> expr_ptr;

  std::optional<ArrayInitializer> array_initializer;

  if (peek() && peek()->tokentype == TokenType::EQUALS) {
    auto tok = consume(); // =
    if (!type.dimensions.empty() && peek() && peek()->tokentype == TokenType::BRACES_OPEN) {
      array_initializer = parseArrayInitializer();
    } else {
      expr_ptr = parseExpr();
      if (!expr_ptr) {
        error("Expected an initializer expression after '='.", tok);
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
    return nullptr;
  }

  if (!match(TokenType::SEMI_COLON)) {
    return nullptr;
  }

  return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::parse_stmt() {
  std::unique_ptr<Stmt> stmt;

  if (!peek()) {
    return nullptr;
  }
  TokenType tok = peek()->tokentype;

  if (tok == TokenType::BRACES_OPEN) {
    stmt = parseBlock();
  } else if (tok == TokenType::RETURN) {
    consume();
    stmt = parseReturnStmt();
  } else if (tok == TokenType::UNION || tok == TokenType::STRUCT || tok == TokenType::ENUM) {
    stmt = parseVariableDeclarationStmt();
  } else if (tok == TokenType::TYPEDEF) {
    stmt = parseTypedefDeclarationStmt();
  } else if (isDatatype(tok) || isTypedefName()) {
    stmt = parseVariableDeclarationStmt();
  } else if (tok == TokenType::IDENTIFIER ||
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
    stmt = parseExpressionStmt();
  } else if (tok == TokenType::IF) {
    stmt = parseIfStmt();
  } else if (tok == TokenType::WHILE) {
    stmt = parseWhileStmt();
  } else if (tok == TokenType::DO) {
    stmt = parseDoWhileStmt();
  } else if (tok == TokenType::FOR) {
    stmt = parseForStmt();
  } else if (tok == TokenType::BREAK) {
    stmt = parseBreakStmt();
  } else if (tok == TokenType::CONTINUE) {
    stmt = parseContinueStmt();
  } else if (tok == TokenType::SWITCH) {
    stmt = parseSwitchStmt();
  } else if (tok == TokenType::SEMI_COLON) {
    consume();
    return std::make_unique<EmptyStmt>();
  } else {
    error("Expected a statement.", peek());
    recoverStatement();
    return nullptr;
  }

  if (!stmt && has_error) {
    recoverStatement();
    return std::make_unique<EmptyStmt>();
  }

  return stmt;
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
    error("Expected a typedef name.", peek());
    return nullptr;
  }

  std::string alias_name = consume()->value.value();

  while (peek() && peek()->tokentype == TokenType::SQUARE_BRACKETS_OPEN) {
    consume(); // [
    size_t size = 0;
    if (peek() && peek()->tokentype == TokenType::INT_LET) {
      size = std::stoull(consume()->value.value());
    } else if (!peek()) {
      error("Expected an integer array size before end of file.", std::nullopt);
      return nullptr;
    } else if (peek()->tokentype != TokenType::SQUARE_BRACKETS_CLOSE) {
      error("Expected an integer array size.", peek());
      return nullptr;
    }
    type.dimensions.push_back(size);
    if (!match(TokenType::SQUARE_BRACKETS_CLOSE)) {
      return nullptr;
    }
  }

  if (peek() && peek()->tokentype == TokenType::EQUALS) {
    error("A typedef declaration cannot have an initializer.", peek());
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