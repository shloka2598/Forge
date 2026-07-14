#include "parser.h"

std::optional<std::vector<Parameter>> Parser::parseParameters() {
  std::vector<Parameter> params;

  while (peek() && peek()->tokentype != TokenType::PARENTHESIS_CLOSE) {
    if (!isDatatype(peek()->tokentype) && !isTypedefName()) {
      error("Expected a parameter datatype.", peek());
      recoverParameter();

      if (peek() && peek()->tokentype == TokenType::COMMA) {
        consume();
        continue;
      }

      break;
    }

    ParsedType type = parseDatatype();

    if (type.datatype == DataType::INVALID) {
      recoverParameter();
      if (peek() && peek()->tokentype == TokenType::COMMA) {
        consume();
        continue;
      }
      break;
    }

    // void in parameter case
    if (params.empty() && type.datatype == DataType::VOID && type.pointer_depth == 0 && peek() && peek()->tokentype == TokenType::PARENTHESIS_CLOSE) {
      return params;
    }

    if (pending_struct) {
      program.statements.push_back(std::make_unique<StructDeclarationStmt>(pending_struct->token, std::move(pending_struct)));
    }

    if (pending_union) {
      program.statements.push_back(std::make_unique<UnionDeclarationStmt>(pending_union->token, std::move(pending_union)));
    }

    if (pending_enum) {
      program.statements.push_back(std::make_unique<EnumDeclarationStmt>(pending_enum->token, std::move(pending_enum)));
    }

    parsePointerSuffix(type);

    auto type_token = peek(-1);
    std::optional<std::string> name;

    if (peek() && peek()->tokentype == TokenType::IDENTIFIER) {
      auto tok = consume();
      name = tok->value.value();
      type_token = *tok;
    }

    bool parameter_failed = false;
    while (peek() && peek()->tokentype == TokenType::SQUARE_BRACKETS_OPEN) {
      consume(); // [
      size_t size = 0;
      if (peek() && peek()->tokentype == TokenType::INT_LET) {
        size = std::stoull(consume()->value.value());
      } else if (!peek()) {
        error("Expected an integer array size before end of file.", std::nullopt);
        return std::nullopt;
      } else if (peek()->tokentype != TokenType::SQUARE_BRACKETS_CLOSE) {
        error("Expected an integer array size.", peek());
        recoverParameter();
        parameter_failed = true;
        if (peek() && peek()->tokentype == TokenType::COMMA) {
          consume();
        }

        break;
      }
      type.dimensions.push_back(size);
      if (!match(TokenType::SQUARE_BRACKETS_CLOSE)) {
        recoverParameter();

        parameter_failed = true;

        if (peek() && peek()->tokentype == TokenType::COMMA) {
          consume();
        }
        break;
      }
    }
    if (parameter_failed) {
      continue;
    }
    params.push_back({.token = std::move(*type_token), .type = std::move(type), .name = std::move(name)});
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
    error("Expected a function return type.", peek());
    return nullptr;
  }
  ParsedType return_type = parseDatatype();

  if (return_type.datatype == DataType::INVALID) {
    return nullptr;
  }

  if (pending_struct) {
    program.statements.push_back(std::make_unique<StructDeclarationStmt>(pending_struct->token, std::move(pending_struct)));
  }

  if (pending_union) {
    program.statements.push_back(std::make_unique<UnionDeclarationStmt>(pending_union->token, std::move(pending_union)));
  }

  if (pending_enum) {
    program.statements.push_back(std::make_unique<EnumDeclarationStmt>(pending_enum->token, std::move(pending_enum)));
  }

  parsePointerSuffix(return_type);

  if (!peek() || peek()->tokentype != TokenType::IDENTIFIER) {
    error("Expected a function name.", peek());
    return nullptr;
  }

  auto name_tok = consume();
  std::string function_name = name_tok->value.value();

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
    return std::make_unique<FunctionDecl>(*name_tok, std::move(return_type), std::move(function_name), std::move(params));
  }

  auto body = parseBlock();

  if (!body) {
    return nullptr;
  }
  return std::make_unique<FunctionDecl>(*name_tok, std::move(return_type), std::move(function_name), std::move(params), std::move(body));
}