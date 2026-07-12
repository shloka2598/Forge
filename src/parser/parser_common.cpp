#include "parser.h"

std::optional<Token> Parser::peek(int ahead) const {
  if (_index + ahead < _tokens.size()) {
    return _tokens[_index + ahead];
  }
  return std::nullopt;
}

std::optional<Token> Parser::consume() {
  if (_index >= _tokens.size()) {
    return std::nullopt;
  }

  return _tokens[_index++];
}

bool Parser::isAssignable(const Expr *expr) const {
  if (dynamic_cast<const IdentifierExpr *>(expr))
    return true;

  if (dynamic_cast<const ArrayAccessExpr *>(expr))
    return true;

  if (dynamic_cast<const MemberAccessExpr *>(expr))
    return true;

  if (auto unary = dynamic_cast<const UnaryExpr *>(expr))
    return unary->op == TokenType::MULTIPLY;

  return false;
}

bool Parser::match(TokenType expected) {
  if (!peek().has_value()) {
    error("Unexpected end of file while parsing", std::nullopt);
    return false;
  }

  if (peek()->tokentype != expected) {
    error("Expected " + std::string(token_name(expected)), peek());
    return false;
  }

  consume();
  return true;
}

void Parser::error(const std::string &msg, std::optional<Token> token) {
  has_error = true;
  if (token.has_value()) {
    diagnostics.error(msg, token->line, token->column, token->length);
  } else {
    diagnostics.error(msg, 0, 0, 0);
  }
}

bool Parser::isDatatype(TokenType token) const {
  switch (token) {
  case TokenType::SIGNED:
  case TokenType::UNSIGNED:
  case TokenType::DATATYPE_CHAR:
  case TokenType::DATATYPE_INT:
  case TokenType::DATATYPE_VOID:
  case TokenType::DATATYPE_FLOAT:
  case TokenType::DATATYPE_DOUBLE:
  case TokenType::STRUCT:
  case TokenType::UNION:
  case TokenType::SHORT:
  case TokenType::LONG:
  case TokenType::CONST:
  case TokenType::VOLATILE:
  case TokenType::RESTRICT:
  case TokenType::INLINE:
  case TokenType::STATIC:
  case TokenType::EXTERN:
  case TokenType::REGISTER:
  case TokenType::AUTO:
  case TokenType::TYPEDEF:
  case TokenType::ENUM:
    return true;

  default:
    return false;
  }
}

ParsedType Parser::parseDatatype() {
  ParsedType type{};

  bool is_signed = false;
  bool is_unsigned = false;
  bool is_short = false;
  int long_count = 0;

  bool found_base_type = false;

  while (peek()) {
    switch (peek()->tokentype) {
      // type qualifiers
    case TokenType::CONST: {
      constexpr auto flag = static_cast<uint8_t>(TypeQualifier::CONST);

      if (type.qualifiers & flag) {
        error("Duplicate 'const' qualifier.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }

      type.qualifiers |= flag;
      consume();
      break;
    }

    case TokenType::VOLATILE: {
      constexpr auto flag = static_cast<uint8_t>(TypeQualifier::VOLATILE);

      if (type.qualifiers & flag) {
        error("Duplicate 'volatile' qualifier.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }

      type.qualifiers |= flag;
      consume();
      break;
    }

    case TokenType::RESTRICT: {
      constexpr auto flag = static_cast<uint8_t>(TypeQualifier::RESTRICT);

      if (type.qualifiers & flag) {
        error("Duplicate 'restrict' qualifier.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }

      type.qualifiers |= flag;
      consume();
      break;
    }

      // storage classes
    case TokenType::STATIC: {
      if (type.storage != StorageClass::NONE) {
        error("Multiple storage class specifiers are not allowed.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      type.storage = StorageClass::STATIC;
      consume();
      break;
    }

    case TokenType::EXTERN: {
      if (type.storage != StorageClass::NONE) {
        error("Multiple storage class specifiers are not allowed.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      type.storage = StorageClass::EXTERN;
      consume();
      break;
    }

    case TokenType::REGISTER: {
      if (type.storage != StorageClass::NONE) {
        error("Multiple storage class specifiers are not allowed.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      type.storage = StorageClass::REGISTER;
      consume();
      break;
    }

    case TokenType::AUTO: {
      if (type.storage != StorageClass::NONE) {
        error("Multiple storage class specifiers are not allowed.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      type.storage = StorageClass::AUTO;
      consume();
      break;
    }

    case TokenType::TYPEDEF: {
      if (type.storage != StorageClass::NONE) {
        error("Multiple storage class specifiers are not allowed.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      type.storage = StorageClass::TYPEDEF;
      consume();
      break;
    }

      // function specifier
    case TokenType::INLINE: {
      if (type.function_specifier != FunctionSpecifier::NONE) {
        error("Duplicate 'inline' specifier.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }

      type.function_specifier = FunctionSpecifier::INLINE;
      consume();
      break;
    }

      // int mods
    case TokenType::SIGNED: {
      if (is_signed) {
        error("Duplicate 'signed' specifier.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      is_signed = true;
      consume();
      break;
    }

    case TokenType::UNSIGNED: {
      if (is_unsigned) {
        error("Duplicate 'unsigned' specifier.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }

      is_unsigned = true;
      consume();
      break;
    }

    case TokenType::SHORT: {
      if (is_short) {
        error("Duplicate 'short' specifier.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      is_short = true;
      consume();
      break;
    }

    case TokenType::LONG: {
      long_count++;
      if (long_count > 2) {
        error("Too many 'long' specifiers.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      consume();
      break;
    }

      // primitive types
    case TokenType::DATATYPE_VOID: {
      if (found_base_type) {
        error("Multiple base types specified.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      type.datatype = DataType::VOID;
      found_base_type = true;
      consume();
      break;
    }

    case TokenType::DATATYPE_CHAR: {
      if (found_base_type) {
        error("Multiple base types specified.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      type.datatype = DataType::CHAR;
      found_base_type = true;
      consume();
      break;
    }

    case TokenType::DATATYPE_INT: {
      if (found_base_type) {
        error("Multiple base types specified.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      type.datatype = DataType::INT;
      found_base_type = true;
      consume();
      break;
    }

    case TokenType::DATATYPE_FLOAT: {
      if (found_base_type) {
        error("Multiple base types specified.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      type.datatype = DataType::FLOAT;
      found_base_type = true;
      consume();
      break;
    }

    case TokenType::DATATYPE_DOUBLE: {
      if (found_base_type) {
        error("Multiple base types specified.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      type.datatype = DataType::DOUBLE;
      found_base_type = true;
      consume();
      break;
    }

      // user defined types
    case TokenType::STRUCT: {
      consume();
      if (found_base_type) {
        error("Multiple base types specified.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      type.datatype = DataType::STRUCT;

      if (peek() && peek()->tokentype == TokenType::IDENTIFIER) {
        type.custom_name = consume()->value.value();
      }

      if (peek() && (peek()->tokentype == TokenType::BRACES_OPEN || peek()->tokentype == TokenType::SEMI_COLON)) {
        pending_struct = parseStructDeclaration(type.custom_name, false);

        if (!pending_struct) {
          type.datatype = DataType::INVALID;
          return type;
        }
      }

      found_base_type = true;
      break;
    }

    case TokenType::UNION: {
      consume();
      if (found_base_type) {
        error("Multiple base types specified.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      type.datatype = DataType::UNION;

      if (peek() && peek()->tokentype == TokenType::IDENTIFIER) {
        type.custom_name = consume()->value.value();
      }

      if (peek() && (peek()->tokentype == TokenType::BRACES_OPEN || peek()->tokentype == TokenType::SEMI_COLON)) {
        pending_union = parseUnionDeclaration(type.custom_name, false);
        if (!pending_union) {
          type.datatype = DataType::INVALID;
          return type;
        }
      }

      found_base_type = true;
      break;
    }

    case TokenType::ENUM: {
      consume();
      if (found_base_type) {
        error("Multiple base types specified.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      type.datatype = DataType::ENUM;

      if (peek() && peek()->tokentype == TokenType::IDENTIFIER) {
        type.custom_name = consume()->value.value();
      }

      if (peek() && (peek()->tokentype == TokenType::BRACES_OPEN || peek()->tokentype == TokenType::SEMI_COLON)) {
        pending_enum = parseEnumDeclaration(type.custom_name, false);
        if (!pending_enum) {
          type.datatype = DataType::INVALID;
          return type;
        }
      }

      found_base_type = true;
      break;
    }

    case TokenType::IDENTIFIER: {
      if (!typedef_names.contains(peek()->value.value())) {
        goto done;
      }

      if (found_base_type) {
        error("Multiple base types specified.", peek());
        type.datatype = DataType::INVALID;
        return type;
      }
      type.datatype = DataType::TYPEDEF_NAME;
      type.custom_name = consume()->value.value();
      found_base_type = true;
      break;
    }

    default:
      goto done;
    }
  }

done:

  if (!found_base_type && (is_short || long_count > 0 || is_signed || is_unsigned)) {
    type.datatype = DataType::INT;
    found_base_type = true;
  }

  if (!found_base_type) {
    error("Expected a datatype.", peek());
    type.datatype = DataType::INVALID;
    return type;
  }

  // validating int mods
  if (is_signed && is_unsigned) {
    error("Type cannot be both 'signed' and 'unsigned'.", peek());
    type.datatype = DataType::INVALID;
    return type;
  }

  if (is_short && long_count > 0) {
    error("Type cannot be both 'short' and 'long'.", peek());
    type.datatype = DataType::INVALID;
    return type;
  }

  // Apply modifiers
  switch (type.datatype) {
  case DataType::INT: {
    if (is_short) {
      type.datatype = is_unsigned ? DataType::USHORT : DataType::SHORT;
    } else if (long_count == 1) {
      type.datatype = is_unsigned ? DataType::ULONG : DataType::LONG;
    } else if (long_count == 2) {
      type.datatype = is_unsigned ? DataType::ULONGLONG : DataType::LONGLONG;
    } else {
      type.datatype = is_unsigned ? DataType::UINT : DataType::INT;
    }
    break;
  }
  case DataType::CHAR: {
    if (is_short || long_count) {
      error("Illegal type modifiers for 'char'.", peek());
      type.datatype = DataType::INVALID;
      return type;
    }
    if (is_signed) {
      error("'signed char' is not supported yet.", peek());
      type.datatype = DataType::INVALID;
      return type;
    }

    if (is_unsigned) {
      type.datatype = DataType::UCHAR;
    }

    break;
  }
  case DataType::FLOAT: {
    if (is_signed || is_unsigned || is_short || long_count) {
      error("Illegal modifiers for 'float'.", peek());
      type.datatype = DataType::INVALID;
      return type;
    }
    break;
  }
  case DataType::DOUBLE: {
    if (is_signed || is_unsigned || is_short) {
      error("Illegal modifiers for 'double'.", peek());
      type.datatype = DataType::INVALID;
      return type;
    }
    if (long_count == 1) {
      type.datatype = DataType::LONGDOUBLE;
    } else if (long_count > 1) {
      error("'long long double' is not a valid type specifier.", peek());
      type.datatype = DataType::INVALID;
      return type;
    }
    break;
  }
  case DataType::VOID: {
    if (is_signed || is_unsigned || is_short || long_count) {
      error("Illegal modifiers for 'void'.", peek());
      type.datatype = DataType::INVALID;
      return type;
    }
    break;
  }
  case DataType::STRUCT: {
    if (is_signed || is_unsigned || is_short || long_count) {
      error("Illegal modifiers for 'struct'.", peek());
      type.datatype = DataType::INVALID;
      return type;
    }
    break;
  }
  case DataType::UNION: {
    if (is_signed || is_unsigned || is_short || long_count) {
      error("Illegal modifiers for 'union'.", peek());
      type.datatype = DataType::INVALID;
      return type;
    }
    break;
  }

  case DataType::ENUM: {
    if (is_signed || is_unsigned || is_short || long_count) {
      error("Illegal modifiers for 'enum'.", peek());
      type.datatype = DataType::INVALID;
      return type;
    }
    break;
  }

  default:
    break;
  }

  return type;
}

bool Parser::isCastExpression() const {
  if (!peek() || peek()->tokentype != TokenType::PARENTHESIS_OPEN) {
    return false;
  }
  int i = 1;
  while (peek(i) && (isDatatype(peek(i)->tokentype) || (peek(i)->tokentype == TokenType::IDENTIFIER && typedef_names.contains(peek(i)->value.value())))) {
    if (peek(i)->tokentype == TokenType::STRUCT || peek(i)->tokentype == TokenType::UNION || peek(i)->tokentype == TokenType::ENUM) {
      i++;
      if (peek(i) && peek(i)->tokentype == TokenType::IDENTIFIER) {
        i++;
      }
      if (peek(i) && peek(i)->tokentype == TokenType::BRACES_OPEN) {
        int depth = 1;
        i++;
        while (peek(i) && depth > 0) {
          if (peek(i)->tokentype == TokenType::BRACES_OPEN) {
            depth++;
          } else if (peek(i)->tokentype == TokenType::BRACES_CLOSE) {
            depth--;
          }
          i++;
        }
      }

      continue;
    }
    i++;
  }

  // Skip pointer declarators and their qualifiers
  while (peek(i) && peek(i)->tokentype == TokenType::MULTIPLY) {
    i++;

    while (peek(i) && (peek(i)->tokentype == TokenType::CONST || peek(i)->tokentype == TokenType::VOLATILE || peek(i)->tokentype == TokenType::RESTRICT)) {
      i++;
    }
  }

  if (i == 1) {
    return false;
  }

  return (peek(i) && peek(i)->tokentype == TokenType::PARENTHESIS_CLOSE);
}

TokenType Parser::compound_to_binary(TokenType compound_op) {
  switch (compound_op) {
  case TokenType::PLUS_EQUALS: {
    return TokenType::PLUS;
  }
  case TokenType::MINUS_EQUALS: {
    return TokenType::MINUS;
  }
  case TokenType::MULTIPLY_EQUALS: {
    return TokenType::MULTIPLY;
  }
  case TokenType::DIVIDE_EQUALS: {
    return TokenType::DIVIDE;
  }
  case TokenType::MOD_EQUALS: {
    return TokenType::MODULO;
  }
  case TokenType::LEFT_SHIFT_EQUALS: {
    return TokenType::LEFT_SHIFT;
  }
  case TokenType::RIGHT_SHIFT_EQUALS: {
    return TokenType::RIGHT_SHIFT;
  }
  case TokenType::AMPERSAND_EQUALS: {
    return TokenType::AMPERSAND;
  }
  case TokenType::PIPE_EQUALS: {
    return TokenType::PIPE;
  }
  case TokenType::CARET_EQUALS: {
    return TokenType::CARET;
  }
  default: {
    return TokenType::INVALID;
  }
  }
}

bool Parser::looksLikeFunction() const {
  int i = 0;

  while (peek(i) && (isDatatype(peek(i)->tokentype) || (peek(i)->tokentype == TokenType::IDENTIFIER && typedef_names.contains(peek(i)->value.value())))) {
    if (peek(i)->tokentype == TokenType::STRUCT || peek(i)->tokentype == TokenType::UNION || peek(i)->tokentype == TokenType::ENUM) {
      i++;
      if (peek(i) && peek(i)->tokentype == TokenType::IDENTIFIER) {
        i++;
      }
      if (peek(i) && peek(i)->tokentype == TokenType::BRACES_OPEN) {
        int depth = 1;
        i++;
        while (peek(i) && depth > 0) {
          if (peek(i)->tokentype == TokenType::BRACES_OPEN) {
            depth++;
          } else if (peek(i)->tokentype == TokenType::BRACES_CLOSE) {
            depth--;
          }
          i++;
        }
      }
      continue;
    }
    i++;
  }

  while (peek(i) && peek(i)->tokentype == TokenType::MULTIPLY) {
    i++;
  }

  if (!peek(i) || peek(i)->tokentype != TokenType::IDENTIFIER) {
    return false;
  }

  i++;

  return peek(i) && peek(i)->tokentype == TokenType::PARENTHESIS_OPEN;
}

void Parser::parsePointerSuffix(ParsedType &type) {
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
}