#include "parser.h"

std::optional<std::vector<Parameter>> Parser::parseParameters() {
  std::vector<Parameter> params;

  while (peek() && peek()->tokentype != TokenType::PARENTHESIS_CLOSE) {
    if (!isDatatype(peek()->tokentype) && !isTypedefName()) {
      error("Expected datatype");
      return std::nullopt;
    }

    ParsedType type = parseDatatype();

    if (type.datatype == DataType::INVALID) {
      return std::nullopt;
    }

    // void in parameter case
    if (params.empty() && type.datatype == DataType::VOID && type.pointer_depth == 0 && peek() && peek()->tokentype == TokenType::PARENTHESIS_CLOSE) {
      return params;
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

    std::optional<std::string> name;

    if (peek() && peek()->tokentype == TokenType::IDENTIFIER) {
      name = consume()->value.value();
    }

    while (peek() && peek()->tokentype == TokenType::SQUARE_BRACKETS_OPEN) {
      consume(); // [
      size_t size = 0;
      if (peek() && peek()->tokentype == TokenType::INT_LET) {
        size = std::stoull(consume()->value.value());
      }
      type.dimensions.push_back(size);
      if (!match(TokenType::SQUARE_BRACKETS_CLOSE)) {
        return std::nullopt;
      }
    }
    params.push_back({.type = std::move(type), .name = std::move(name)});
    if (peek() && peek()->tokentype == TokenType::COMMA) {
      consume();
    } else {
      break;
    }
  }
  return params;
}

std::unique_ptr<FunctionDecl> Parser::parseFunction() {
  if (!peek() || (!isDatatype(peek()->tokentype) && !isTypedefName())) {
    error("Expected datatype");
    return nullptr;
  }
  ParsedType return_type = parseDatatype();

  if (pending_struct) {
    program.statements.push_back(std::make_unique<StructDeclarationStmt>(std::move(pending_struct)));
  }

  if (pending_union) {
    program.statements.push_back(std::make_unique<UnionDeclarationStmt>(std::move(pending_union)));
  }

  if (pending_enum) {
    program.statements.push_back(std::make_unique<EnumDeclarationStmt>(std::move(pending_enum)));
  }

  if (return_type.datatype == DataType::INVALID) {
    return nullptr;
  }

  parsePointerSuffix(return_type);

  if (!peek() || peek()->tokentype != TokenType::IDENTIFIER) {
    error("Expected function name");
    return nullptr;
  }

  std::string function_name = consume()->value.value();
  if (!match(TokenType::PARENTHESIS_OPEN)) {
    return nullptr;
  }
  std::vector<Parameter> params;

  if (peek() && peek()->tokentype != TokenType::PARENTHESIS_CLOSE) {
    auto parsed_params = parseParameters();
    if (!parsed_params) {
      return nullptr;
    }
    params = std::move(parsed_params.value());
  }

  if (!match(TokenType::PARENTHESIS_CLOSE)) {
    return nullptr;
  }
  if (peek() && peek()->tokentype == TokenType::SEMI_COLON) {
    consume(); // ;
    return std::make_unique<FunctionDecl>(std::move(return_type), std::move(function_name), std::move(params));
  }

  auto body = parseBlock();

  if (!body) {
    return nullptr;
  }
  return std::make_unique<FunctionDecl>(std::move(return_type), std::move(function_name), std::move(params), std::move(body));
}