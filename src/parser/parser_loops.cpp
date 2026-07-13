#include "parser.h"

std::unique_ptr<IfStmt> Parser::parseIfStmt() {
  auto tok = consume(); // if
  if (!match(TokenType::PARENTHESIS_OPEN)) {
    return nullptr;
  }

  std::unique_ptr<Expr> condition = parseExpr();
  if (!condition) {
    error("Expected a condition expression after 'if'.", tok);
    return nullptr;
  }

  if (!match(TokenType::PARENTHESIS_CLOSE)) {
    return nullptr;
  }
  auto then_body = parseStatementOrBlock();
  std::unique_ptr<BlockStmt> else_body;
  if (peek() && peek()->tokentype == TokenType::ELSE) {
    consume();
    if (peek() && peek()->tokentype == TokenType::IF) {
      auto nested_if = parseIfStmt();
      if (!nested_if)
        return nullptr;

      else_body = std::make_unique<BlockStmt>();
      else_body->statements.push_back(std::move(nested_if));
    } else {
      else_body = parseStatementOrBlock();
      if (!else_body)
        return nullptr;
    }
  }

  return std::make_unique<IfStmt>(std::move(condition), std::move(then_body), std::move(else_body));
}

std::unique_ptr<WhileStmt> Parser::parseWhileStmt() {
  auto tok = consume(); // while
  if (!match(TokenType::PARENTHESIS_OPEN)) {
    return nullptr;
  }
  auto condition = parseExpr();
  if (!condition) {
    error("Expected a condition expression after 'while'.", tok);

    return nullptr;
  }
  if (!match(TokenType::PARENTHESIS_CLOSE)) {
    return nullptr;
  }

  auto body = parseStatementOrBlock();

  if (!body) {
    return nullptr;
  }

  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<DoWhileStmt> Parser::parseDoWhileStmt() {
  consume(); // do
  auto body = parseStatementOrBlock();
  if (!body) {
    return nullptr;
  }
  if (!match(TokenType::WHILE)) {
    return nullptr;
  }
  if (!match(TokenType::PARENTHESIS_OPEN)) {
    return nullptr;
  }
  auto condition = parseExpr();
  if (!condition) {
    error("Expected a condition expression after 'while'.", peek());
    return nullptr;
  }
  if (!match(TokenType::PARENTHESIS_CLOSE)) {
    return nullptr;
  }
  if (!match(TokenType::SEMI_COLON)) {
    return nullptr;
  }
  return std::make_unique<DoWhileStmt>(std::move(body), std::move(condition));
}

std::unique_ptr<SwitchStmt> Parser::parseSwitchStmt() {
  auto tok = consume(); // switch
  if (!match(TokenType::PARENTHESIS_OPEN)) {
    return nullptr;
  }
  auto condition = parseExpr();
  if (!condition) {
    error("Expected a condition expression after 'switch'.", tok);
    return nullptr;
  }
  if (!match(TokenType::PARENTHESIS_CLOSE)) {
    return nullptr;
  }
  if (!match(TokenType::BRACES_OPEN)) {
    return nullptr;
  }
  std::vector<SwitchCase> cases;
  std::unique_ptr<BlockStmt> default_body;
  bool seen_default = false;
  while (peek() && peek()->tokentype != TokenType::BRACES_CLOSE) {
    if (peek()->tokentype == TokenType::CASE) {
      auto tok = consume(); // case
      auto case_value = parseExpr();
      if (!case_value) {
        error("Expected a case expression after 'case'.", tok);
        return nullptr;
      }
      if (!match(TokenType::COLON)) {
        return nullptr;
      }
      auto case_body = std::make_unique<BlockStmt>();
      while (peek() && peek()->tokentype != TokenType::CASE && peek()->tokentype != TokenType::DEFAULT && peek()->tokentype != TokenType::BRACES_CLOSE) {
        auto stmt = parse_stmt();

        if (!stmt) {
          continue;
        }

        case_body->statements.push_back(std::move(stmt));
      }
      cases.emplace_back(std::move(case_value), std::move(case_body));
    } else if (peek()->tokentype == TokenType::DEFAULT) {
      if (seen_default) {
        error("Multiple 'default' labels are not allowed.", peek());
        return nullptr;
      }
      seen_default = true;
      consume(); // default
      if (!match(TokenType::COLON)) {
        return nullptr;
      }
      default_body = std::make_unique<BlockStmt>();
      while (peek() && peek()->tokentype != TokenType::CASE && peek()->tokentype != TokenType::DEFAULT && peek()->tokentype != TokenType::BRACES_CLOSE) {
        auto stmt = parse_stmt();
        if (!stmt) {
          continue;
        }
        default_body->statements.push_back(std::move(stmt));
      }
    } else {
      error("Expected 'case' or 'default' label.", peek());
      return nullptr;
    }
  }

  if (!match(TokenType::BRACES_CLOSE)) {
    return nullptr;
  }

  return std::make_unique<SwitchStmt>(std::move(condition), std::move(cases), std::move(default_body));
}

std::unique_ptr<Stmt> Parser::parseForInitStmt() {
  if (!peek()) {
    error("Expected a for-loop initializer.", std::nullopt);
    return nullptr;
  }
  // Variable declaration
  if (isTypedefName() || isDatatype(peek()->tokentype)) {
    ParsedType type = parseDatatype();

    if (type.datatype == DataType::INVALID) {
      return nullptr;
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
      } else if (peek()->tokentype != TokenType::SQUARE_BRACKETS_CLOSE) {
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
      auto tok = consume();
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

    return std::make_unique<VariableDeclarationStmt>(std::move(type), std::move(var_name), std::move(expr_ptr), std::move(array_initializer));
  }

  auto expr = parseExpr();

  if (!expr) {
    return nullptr;
  }

  return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::parseForUpdateStmt() {
  if (!peek()) {
    error("Expected a for-loop update expression.", std::nullopt);
    return nullptr;
  }

  auto expr = parseExpr();
  if (!expr) {
    return nullptr;
  }

  return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<ForStmt> Parser::parseForStmt() {
  consume(); // for
  if (!match(TokenType::PARENTHESIS_OPEN)) {
    return nullptr;
  }
  std::unique_ptr<Stmt> init_stmt = nullptr;
  if (peek() && peek()->tokentype != TokenType::SEMI_COLON) {
    init_stmt = parseForInitStmt();
    recoverIfFailed(init_stmt, TokenType::SEMI_COLON);
  }
  if (!match(TokenType::SEMI_COLON)) {
    return nullptr;
  }
  std::unique_ptr<Expr> condition = nullptr;
  if (peek() && peek()->tokentype != TokenType::SEMI_COLON) {
    condition = parseExpr();
    recoverIfFailed(condition, TokenType::SEMI_COLON);
  }
  if (!match(TokenType::SEMI_COLON)) {
    return nullptr;
  }
  std::unique_ptr<Stmt> update_stmt = nullptr;
  if (peek() && peek()->tokentype != TokenType::PARENTHESIS_CLOSE) {
    update_stmt = parseForUpdateStmt();
    recoverIfFailed(update_stmt, TokenType::PARENTHESIS_CLOSE);
  }
  if (!match(TokenType::PARENTHESIS_CLOSE)) {
    return nullptr;
  }

  auto body = parseStatementOrBlock();

  if (!body) {
    return nullptr;
  }

  return std::make_unique<ForStmt>(std::move(init_stmt), std::move(condition), std::move(update_stmt), std::move(body));
}