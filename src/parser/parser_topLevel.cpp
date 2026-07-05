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
      error("Expected a datatype");
      return nullptr;
    }

    while (peek() && peek()->tokentype == TokenType::MULTIPLY) {
      consume();
      field_type.pointer_depth++;
    }

    if (!peek() || peek().value().tokentype != TokenType::IDENTIFIER) {
      error("Expected an identifier");
      return nullptr;
    }
    std::string field_name = consume().value().value.value();

    while (peek() && peek()->tokentype == TokenType::SQUARE_BRACKETS_OPEN) {
      consume(); // [
      size_t size = 0;
      if (peek() && peek()->tokentype == TokenType::INT_LET) {
        size = std::stoull(consume()->value.value());
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
      error("Expected a datatype");
      return nullptr;
    }

    while (peek() && peek()->tokentype == TokenType::MULTIPLY) {
      consume();
      field_type.pointer_depth++;
      field_type.pointer_qualifiers.push_back(0);
      while (peek()) {
        if (peek()->tokentype == TokenType::CONST) {
          field_type.pointer_qualifiers.back() |= static_cast<uint8_t>(TypeQualifier::CONST);
          consume();
        } else if (peek()->tokentype == TokenType::VOLATILE) {
          field_type.pointer_qualifiers.back() |= static_cast<uint8_t>(TypeQualifier::VOLATILE);
          consume();
        } else if (peek()->tokentype == TokenType::RESTRICT) {
          field_type.pointer_qualifiers.back() |= static_cast<uint8_t>(TypeQualifier::RESTRICT);
          consume();
        } else {
          break;
        }
      }
    }
    if (!peek() || peek().value().tokentype != TokenType::IDENTIFIER) {
      error("Expected an identifier");
      return nullptr;
    }
    std::string field_name = consume().value().value.value();

    while (peek() && peek()->tokentype == TokenType::SQUARE_BRACKETS_OPEN) {
      consume(); // [
      size_t size = 0;
      if (peek() && peek()->tokentype == TokenType::INT_LET) {
        size = std::stoull(consume()->value.value());
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
      error("Expected enum member");
      return nullptr;
    }

    EnumMember member;
    member.name = consume()->value.value();

    if (peek() && peek()->tokentype == TokenType::EQUALS) {
      consume(); // =
      member.value = parseAssignment();
      if (!member.value) {
        error("Expected enum initializer");
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
        error("Expected initializer expression");
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

  while (peek() && peek()->tokentype == TokenType::MULTIPLY) {
    consume();
    type.pointer_depth++;
    type.pointer_qualifiers.push_back(0);
    while (peek()) {
      if (peek()->tokentype == TokenType::CONST) {
        type.pointer_qualifiers.back() |= static_cast<uint8_t>(TypeQualifier::CONST);
        consume();
      } else if (peek()->tokentype == TokenType::VOLATILE) {
        type.pointer_qualifiers.back() |= static_cast<uint8_t>(TypeQualifier::VOLATILE);
        consume();
      } else if (peek()->tokentype == TokenType::RESTRICT) {
        type.pointer_qualifiers.back() |= static_cast<uint8_t>(TypeQualifier::RESTRICT);
        consume();
      } else {
        break;
      }
    }
  }

  if (!peek() || peek()->tokentype != TokenType::IDENTIFIER) {
    error("Expected global variable name");
    return nullptr;
  }

  std::string name = consume()->value.value();

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
  std::unique_ptr<Expr> initializer;
  auto global = std::make_unique<GlobalVariableDecl>();
  if (peek() && peek()->tokentype == TokenType::EQUALS) {
    consume(); // =
    if (!type.dimensions.empty()) {
      if (peek() && peek()->tokentype == TokenType::BRACES_OPEN) {
        global->array_initializer = parseArrayInitializer();
      } else {
        error("Expected array initializer");
        return nullptr;
      }
    } else {
      initializer = parseExpr();
      if (!initializer) {
        error("Expected initializer expression");
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
    error("Expected top-level declaration");
    return;
  }

  if (peek()->tokentype == TokenType::TYPEDEF) {
    auto td = parseTypedefDeclarationStmt();

    if (!td) {
      return;
    }

    program.statements.push_back(std::move(td));
    return;
  }

  bool is_function = looksLikeFunction();

  if (is_function) {
    auto fn = parseFunction();
    if (!fn) {
      return;
    }
    program.statements.push_back(std::make_unique<FunctionDeclStmt>(std::move(fn)));
  } else {
    auto global = parseGlobalVariable();
    if (!global) {
      return;
    }
    program.statements.push_back(std::make_unique<GlobalVariableDeclStmt>(std::move(global)));
  }
}

void Parser::parse() {
  while (peek()) {
    parseTopLevelDeclaration();
    if (has_error) {
      return;
    }
  }
}