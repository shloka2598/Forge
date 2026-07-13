#include "parser.h"

#include <memory>

std::unique_ptr<BlockStmt> Parser::parseBlock() {
  if (!match(TokenType::BRACES_OPEN)) {
    return nullptr;
  }

  auto block = std::make_unique<BlockStmt>();

  while (peek() && peek()->tokentype != TokenType::BRACES_CLOSE) {
    std::unique_ptr<Stmt> stmt = parse_stmt();
    if (!stmt) {
      if (has_error) {
        recoverStatement();
        continue;
      }
      return nullptr;
    }
    block->statements.push_back(std::move(stmt));
  }

  if (!match(TokenType::BRACES_CLOSE)) {
    return nullptr;
  }

  return block;
}

std::unique_ptr<BlockStmt> Parser::parseStatementOrBlock() {
  if (peek() && peek()->tokentype == TokenType::BRACES_OPEN) {
    return parseBlock();
  }

  auto stmt = parse_stmt();

  if (!stmt) {
    return nullptr;
  }

  auto result = std::make_unique<BlockStmt>();
  result->statements.push_back(std::move(stmt));

  return result;
}
