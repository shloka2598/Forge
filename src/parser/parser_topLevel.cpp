#include "parser.h"

std::unique_ptr<StructDecl> Parser::parseStructDeclaration(std::string struct_name, bool expect_semicolon) {
  if (!peek()) {
    return nullptr;
  }

  if (peek()->tokentype == TokenType::SEMI_COLON) {
    auto decl = std::make_unique<StructDecl>(struct_name, std::vector<StructField>{});
    decl->is_definition = false;

    if (expect_semicolon)
      consume();

    return decl;
  }

  if (!match(TokenType::BRACES_OPEN)) {
    return nullptr;
  }

  std::vector<StructField> fields;
  while (peek() && peek()->tokentype != TokenType::BRACES_CLOSE) {
    ParsedType field_type = parseDatatype();
    if (field_type.datatype == DataType::INVALID) {
      return nullptr;
    }

    parsePointerSuffix(field_type);

    if (!peek() || peek().value().tokentype != TokenType::IDENTIFIER) {
      error("Expected a struct field name.", peek());
      return nullptr;
    }
    std::string field_name = consume().value().value.value();

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
      field_type.dimensions.push_back(size);
      if (!match(TokenType::SQUARE_BRACKETS_CLOSE)) {
        return nullptr;
      }
    }
    if (!match(TokenType::SEMI_COLON)) {
      return nullptr;
    }
    fields.push_back({.type = std::move(field_type), .name = std::move(field_name)});
  }
  if (!match(TokenType::BRACES_CLOSE)) {
    return nullptr;
  }
  if (expect_semicolon) {
    if (!match(TokenType::SEMI_COLON)) {
      return nullptr;
    }
  }

  auto decl = std::make_unique<StructDecl>(std::move(struct_name), std::move(fields));

  decl->is_definition = true;

  return decl;
}

std::unique_ptr<UnionDecl> Parser::parseUnionDeclaration(std::string union_name, bool expect_semicolon) {
  if (!peek()) {
    return nullptr;
  }

  if (peek()->tokentype == TokenType::SEMI_COLON) {
    auto decl = std::make_unique<UnionDecl>(union_name, std::vector<UnionField>{});
    decl->is_definition = false;

    if (expect_semicolon) {
      consume();
    }

    return decl;
  }

  if (!match(TokenType::BRACES_OPEN)) {
    return nullptr;
  }

  std::vector<UnionField> fields;
  while (peek() && peek()->tokentype != TokenType::BRACES_CLOSE) {
    ParsedType field_type = parseDatatype();
    if (field_type.datatype == DataType::INVALID) {
      return nullptr;
    }

    parsePointerSuffix(field_type);

    if (!peek() || peek().value().tokentype != TokenType::IDENTIFIER) {
      error("Expected a union field name.", peek());
      return nullptr;
    }
    std::string field_name = consume().value().value.value();

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
      field_type.dimensions.push_back(size);
      if (!match(TokenType::SQUARE_BRACKETS_CLOSE)) {
        return nullptr;
      }
    }
    if (!match(TokenType::SEMI_COLON)) {
      return nullptr;
    }
    fields.push_back({.type = std::move(field_type), .name = std::move(field_name)});
  }

  if (!match(TokenType::BRACES_CLOSE)) {
    return nullptr;
  }
  if (expect_semicolon) {
    if (!match(TokenType::SEMI_COLON)) {
      return nullptr;
    }
  }

  auto decl = std::make_unique<UnionDecl>(std::move(union_name), std::move(fields));

  decl->is_definition = true;

  return decl;
}

std::unique_ptr<EnumDecl> Parser::parseEnumDeclaration(std::string enum_name, bool expect_semicolon) {
  if (!peek()) {
    return nullptr;
  }

  if (peek()->tokentype == TokenType::SEMI_COLON) {
    auto decl = std::make_unique<EnumDecl>(enum_name, std::vector<EnumMember>{});
    decl->is_definition = false;

    if (expect_semicolon) {
      consume();
    }

    return decl;
  }

  if (!match(TokenType::BRACES_OPEN)) {
    return nullptr;
  }

  std::vector<EnumMember> members;

  while (peek() && peek()->tokentype != TokenType::BRACES_CLOSE) {
    if (!peek() || peek()->tokentype != TokenType::IDENTIFIER) {
      error("Expected an enum member name.", peek());
      return nullptr;
    }

    EnumMember member;
    member.name = consume()->value.value();

    if (peek() && peek()->tokentype == TokenType::EQUALS) {
      auto tok = consume(); // =
      member.value = parseAssignment();
      if (!member.value) {
        error("Expected an enumerator initializer after '='.", tok);
        return nullptr;
      }
    }

    members.push_back(std::move(member));

    if (peek() && peek()->tokentype == TokenType::COMMA) {
      consume();
    } else {
      break;
    }
  }

  if (!match(TokenType::BRACES_CLOSE)) {
    return nullptr;
  }

  if (expect_semicolon) {
    if (!match(TokenType::SEMI_COLON)) {
      return nullptr;
    }
  }

  auto decl = std::make_unique<EnumDecl>(std::move(enum_name), std::move(members));
  decl->is_definition = true;
  return decl;
}

ArrayInitializer Parser::parseArrayInitializer() {
  ArrayInitializer init;

  if (!match(TokenType::BRACES_OPEN)) {
    return init;
  }

  while (peek() && peek()->tokentype != TokenType::BRACES_CLOSE) {
    if (peek()->tokentype == TokenType::BRACES_OPEN) {
      init.children.push_back(parseArrayInitializer());
    } else {
      auto expr = parseAssignment();
      if (!expr) {
        error("Expected an initializer expression.", peek());
        return init;
      }
      ArrayInitializer leaf;
      leaf.is_leaf = true;
      leaf.expr = std::move(expr);
      init.children.push_back(std::move(leaf));
    }
    if (peek() && peek()->tokentype == TokenType::COMMA) {
      consume();
    } else {
      break;
    }
  }
  if (!match(TokenType::BRACES_CLOSE)) {
    return init;
  }
  return init;
}

std::unique_ptr<GlobalVariableDecl> Parser::parseGlobalVariable() {
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

  if (peek() && peek()->tokentype == TokenType::SEMI_COLON) {
    consume();
    return nullptr;
  }

  parsePointerSuffix(type);

  if (!peek() || peek()->tokentype != TokenType::IDENTIFIER) {
    error("Expected a global variable name.", peek());
    return nullptr;
  }

  std::string name = consume()->value.value();

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
  std::unique_ptr<Expr> initializer;
  auto global = std::make_unique<GlobalVariableDecl>();
  if (peek() && peek()->tokentype == TokenType::EQUALS) {
    auto tok = consume(); // =
    if (!type.dimensions.empty()) {
      if (peek() && peek()->tokentype == TokenType::BRACES_OPEN) {
        global->array_initializer = parseArrayInitializer();
      } else {
        error("Expected an array initializer after '='.", tok);
        return nullptr;
      }
    } else {
      initializer = parseExpr();
      if (!initializer) {
        error("Expected an initializer expression after '='.", tok);
        return nullptr;
      }
    }
  }
  if (!match(TokenType::SEMI_COLON)) {
    return nullptr;
  }

  global->type = std::move(type);
  global->name = std::move(name);
  global->initializer = std::move(initializer);

  return global;
}

void Parser::parseTopLevelDeclaration() {
  if (!peek()) {
    return;
  }

  if (!(isDatatype(peek()->tokentype) || isTypedefName())) {
    error("Expected a function, global variable, or typedef declaration.", peek());
    recoverTopLevel();
    return;
  }

  if (peek()->tokentype == TokenType::TYPEDEF) {
    auto td = parseTypedefDeclarationStmt();

    if (!td) {
      if (has_error) {
        recoverTopLevel();
      }
      return;
    }

    program.statements.push_back(std::move(td));
    return;
  }

  bool is_function = looksLikeFunction();

  if (is_function) {
    auto fn = parseFunction();
    if (!fn) {
      if (has_error) {
        recoverTopLevel();
      }
      return;
    }
    program.statements.push_back(std::make_unique<FunctionDeclStmt>(std::move(fn)));
  } else {
    auto global = parseGlobalVariable();
    if (!global) {
      if (has_error) {
        recoverTopLevel();
      }
      return;
    }
    program.statements.push_back(std::make_unique<GlobalVariableDeclStmt>(std::move(global)));
  }
}

void Parser::parse() {
  while (peek()) {
    parseTopLevelDeclaration();
  }
}