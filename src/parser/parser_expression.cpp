#include "parser.h"

std::unique_ptr<Expr> Parser::parsePrimary() {
  if (!peek()) {
    error("Expected primary expression");
    return nullptr;
  }

  TokenType tok = peek()->tokentype;

  if (tok == TokenType::INT_LET) {
    auto token = consume();
    if (!token) {
      error("Expected integer literal");
      return nullptr;
    }
    return std::make_unique<IntLetExpr>(std::stoull(token->value.value(), nullptr, 0));
  }
  if (tok == TokenType::DOUBLE_LET) {
    auto token = consume();
    if (!token) {
      error("Expected double literal");
      return nullptr;
    }
    return std::make_unique<DoubleLetExpr>(std::stod(token->value.value()));
  }
  if (tok == TokenType::FLOAT_LET) {
    auto token = consume();
    if (!token) {
      error("Expected float literal");
      return nullptr;
    }
    return std::make_unique<FloatLetExpr>(std::stof(token->value.value()));
  }
  if (tok == TokenType::CHAR_LET) {
    auto token = consume();
    if (!token) {
      error("Expected char literal");
      return nullptr;
    }
    return std::make_unique<CharLetExpr>(token->value.value()[0]);
  }
  if (tok == TokenType::STRING_LET) {
    auto token = consume();
    if (!token) {
      error("Expected string literal");
      return nullptr;
    }
    return std::make_unique<StringLiteralExpr>(token->value.value());
  }
  if (tok == TokenType::IDENTIFIER) {
    auto token = consume();
    if (!token) {
      error("Expected identifier");
      return nullptr;
    }
    return std::make_unique<IdentifierExpr>(token->value.value());
  }
  if (tok == TokenType::PARENTHESIS_OPEN) {
    consume(); // (
    auto expr = parseExpr();
    if (!expr) {
      return nullptr;
    }
    if (!match(TokenType::PARENTHESIS_CLOSE)) {
      return nullptr;
    }
    return expr;
  }

  error("Expected primary expression");
  return nullptr;
}

std::unique_ptr<Expr> Parser::parseUnaryAndCasting() {
  if (!peek()) {
    error("Expected expression");
    return nullptr;
  }
  TokenType tok = peek()->tokentype;

  if (tok == TokenType::PLUS_PLUS) {
    consume();
    auto expr = parseUnaryAndCasting();
    if (!expr) {
      error("Expected expression after ++");
      return nullptr;
    }
    if (!isAssignable(expr.get())) {
      error("Operand of ++ must be assignable");
      return nullptr;
    }
    return std::make_unique<IncrementExpr>(true, true, std::move(expr));
  }

  if (tok == TokenType::MINUS_MINUS) {
    consume();
    auto expr = parseUnaryAndCasting();
    if (!expr) {
      error("Expected expression after --");
      return nullptr;
    }
    if (!isAssignable(expr.get())) {
      error("Operand of -- must be assignable");
      return nullptr;
    }
    return std::make_unique<IncrementExpr>(true, false, std::move(expr));
  }

  // Unary +, -, !, *(dereference), &(address-of), ~
  if (tok == TokenType::PLUS || tok == TokenType::MINUS || tok == TokenType::EXCLAMATION || tok == TokenType::AMPERSAND || tok == TokenType::MULTIPLY || tok == TokenType::TILDE) {
    consume(); // op
    auto right_expr = parseUnaryAndCasting();
    if (!right_expr) {
      return nullptr;
    }
    return std::make_unique<UnaryExpr>(tok, std::move(right_expr));
  }

  // sizeof
  if (tok == TokenType::SIZEOF) {
    consume();
    // sizeof(type)
    if (isCastExpression()) {
      consume(); // (
      ParsedType type = parseDatatype();
      if (type.datatype == DataType::INVALID) {
        return nullptr;
      }

      parsePointerSuffix(type);

      if (!match(TokenType::PARENTHESIS_CLOSE)) {
        return nullptr;
      }

      return std::make_unique<SizeofExpr>(std::move(type));
    }

    auto expr = parseUnaryAndCasting();

    if (!expr) {
      error("Expected expression after sizeof");
      return nullptr;
    }

    return std::make_unique<SizeofExpr>(std::move(expr));
  }

  // Cast expression
  if (isCastExpression()) {
    consume(); // (
    ParsedType type = parseDatatype();
    if (type.datatype == DataType::INVALID) {
      return nullptr;
    }
    parsePointerSuffix(type);
    if (!match(TokenType::PARENTHESIS_CLOSE)) {
      return nullptr;
    }
    auto expr = parseUnaryAndCasting();
    if (!expr) {
      return nullptr;
    }
    return std::make_unique<CastExpr>(std::move(type), std::move(expr));
  }

  return parsePostfix();
}

std::unique_ptr<Expr> Parser::parsePostfix() {
  auto expr = parsePrimary();
  if (!expr) {
    return nullptr;
  }
  while (peek()) {
    // Function Call
    if (peek()->tokentype == TokenType::PARENTHESIS_OPEN) {
      consume(); // (
      std::vector<std::unique_ptr<Expr>> args;
      if (peek() && peek()->tokentype != TokenType::PARENTHESIS_CLOSE) {
        auto parsed_args = parseArguments();
        if (!parsed_args) {
          return nullptr;
        }
        args = std::move(parsed_args.value());
      }
      if (!match(TokenType::PARENTHESIS_CLOSE)) {
        return nullptr;
      }
      // only identifiers can be called
      auto *id = dynamic_cast<IdentifierExpr *>(expr.get());
      if (!id) {
        error("Expected function name");
        return nullptr;
      }
      expr = std::make_unique<FunctionCallExpr>(id->identifier_name, std::move(args));
      continue;
    }

    // Array Access
    if (peek()->tokentype == TokenType::SQUARE_BRACKETS_OPEN) {
      consume(); // [
      auto index_expr = parseExpr();
      if (!index_expr) {
        error("Expected index expression");
        return nullptr;
      }
      if (!match(TokenType::SQUARE_BRACKETS_CLOSE)) {
        return nullptr;
      }
      expr = std::make_unique<ArrayAccessExpr>(std::move(expr), std::move(index_expr));
      continue;
    }

    // Member Access (structs and unions)
    if (peek()->tokentype == TokenType::DOT) {
      consume(); // .
      if (!peek() || peek()->tokentype != TokenType::IDENTIFIER) {
        error("Expected member name after '.'");
        return nullptr;
      }

      std::string member = consume()->value.value();
      expr = std::make_unique<MemberAccessExpr>(std::move(expr), member, TokenType::DOT);
      continue;
    }

    // Pointer Member Access (structs and unions)
    if (peek()->tokentype == TokenType::ARROW) {
      consume(); // ->
      if (!peek() || peek()->tokentype != TokenType::IDENTIFIER) {
        error("Expected member name after '->'");
        return nullptr;
      }

      std::string member = consume()->value.value();
      expr = std::make_unique<MemberAccessExpr>(std::move(expr), member, TokenType::ARROW);

      continue;
    }

    // Post ++
    if (peek()->tokentype == TokenType::PLUS_PLUS) {
      consume();

      if (!isAssignable(expr.get())) {
        error("Operand of ++ must be assignable");
        return nullptr;
      }

      expr = std::make_unique<IncrementExpr>(false, true, std::move(expr));
      continue;
    }

    // Post --
    if (peek()->tokentype == TokenType::MINUS_MINUS) {
      consume();
      if (!isAssignable(expr.get())) {
        error("Operand of -- must be assignable");
        return nullptr;
      }
      expr = std::make_unique<IncrementExpr>(false, false, std::move(expr));
      continue;
    }

    break;
  }

  return expr;
}

std::unique_ptr<Expr> Parser::parseMultiplicationAndDivision() {
  std::unique_ptr<Expr> left_expr = parseUnaryAndCasting();

  if (!left_expr) {
    return nullptr;
  }

  while (peek() && (peek()->tokentype == TokenType::MULTIPLY || peek()->tokentype == TokenType::DIVIDE || peek()->tokentype == TokenType::MODULO)) {

    TokenType op = consume()->tokentype;

    std::unique_ptr<Expr> right_expr = parseUnaryAndCasting();

    if (!right_expr) {
      error("Expected expression after operator");
      return nullptr;
    }

    left_expr = std::make_unique<BinaryExpr>(std::move(left_expr), op, std::move(right_expr));
  }

  return left_expr;
}

std::unique_ptr<Expr> Parser::parseAdditionAndSubtraction() {
  std::unique_ptr<Expr> left_expr = parseMultiplicationAndDivision();

  if (!left_expr) {
    return nullptr;
  }

  while (peek() && (peek()->tokentype == TokenType::PLUS || peek()->tokentype == TokenType::MINUS)) {
    TokenType op = consume()->tokentype;

    std::unique_ptr<Expr> right_expr = parseMultiplicationAndDivision();

    if (!right_expr) {
      error("Expected expression after operator");
      return nullptr;
    }

    left_expr = std::make_unique<BinaryExpr>(std::move(left_expr), op, std::move(right_expr));
  }

  return left_expr;
}

std::unique_ptr<Expr> Parser::parseShift() {
  auto left = parseAdditionAndSubtraction();
  if (!left) {
    return nullptr;
  }

  while (peek().has_value() && (peek()->tokentype == TokenType::LEFT_SHIFT || peek()->tokentype == TokenType::RIGHT_SHIFT)) {
    TokenType op = consume()->tokentype;
    auto right = parseAdditionAndSubtraction();

    if (!right) {
      error("Expected expression after shift operator");
      return nullptr;
    }

    left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseRelational() {
  std::unique_ptr<Expr> left_expr = parseShift();

  if (!left_expr) {
    return nullptr;
  }

  while (peek() && (peek()->tokentype == TokenType::GREATER_THAN || peek()->tokentype == TokenType::SMALLER_THAN || peek()->tokentype == TokenType::GREATER_THAN_EQUAL_THAN || peek()->tokentype == TokenType::SMALLER_THAN_EQUAL_THAN)) {

    TokenType op = consume()->tokentype;

    std::unique_ptr<Expr> right_expr = parseShift();

    if (!right_expr) {
      error("Expected expression after comparison operator");
      return nullptr;
    }

    left_expr = std::make_unique<BinaryExpr>(std::move(left_expr), op, std::move(right_expr));
  }

  return left_expr;
}

std::unique_ptr<Expr> Parser::parseEquality() {
  auto left = parseRelational();
  if (!left) {
    return nullptr;
  }

  while (peek() && (peek()->tokentype == TokenType::DOUBLE_EQUALS || peek()->tokentype == TokenType::NOT_EQUALS)) {
    TokenType op = consume()->tokentype;
    auto right = parseRelational();
    if (!right) {
      error("Expected expression after equality operator");
      return nullptr;
    }
    left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseBitwiseAnd() {
  auto left = parseEquality();
  if (!left) {
    return nullptr;
  }

  while (peek() && peek()->tokentype == TokenType::AMPERSAND) {
    TokenType op = consume()->tokentype;
    auto right = parseEquality();
    if (!right) {
      error("Expected expression");
      return nullptr;
    }
    left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseBitwiseXor() {
  auto left = parseBitwiseAnd();
  if (!left) {
    return nullptr;
  }

  while (peek() && peek()->tokentype == TokenType::CARET) {
    TokenType op = consume()->tokentype;
    auto right = parseBitwiseAnd();
    if (!right) {
      error("Expected expression");
      return nullptr;
    }
    left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseBitwiseOr() {
  auto left = parseBitwiseXor();
  if (!left) {
    return nullptr;
  }

  while (peek() && peek()->tokentype == TokenType::PIPE) {
    TokenType op = consume()->tokentype;
    auto right = parseBitwiseXor();
    if (!right) {
      error("Expected expression");
      return nullptr;
    }
    left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseLogicalAnd() {
  std::unique_ptr<Expr> left_expr = parseBitwiseOr();

  if (!left_expr) {
    return nullptr;
  }

  while (peek() && peek()->tokentype == TokenType::DOUBLE_AMPERSAND) {
    TokenType op = consume()->tokentype;

    std::unique_ptr<Expr> right_expr = parseBitwiseOr();

    if (!right_expr) {
      error("Expected expression after &&");
      return nullptr;
    }

    left_expr = std::make_unique<BinaryExpr>(std::move(left_expr), op, std::move(right_expr));
  }

  return left_expr;
}

std::unique_ptr<Expr> Parser::parseLogicalOr() {
  std::unique_ptr<Expr> left_expr = parseLogicalAnd();

  if (!left_expr) {
    return nullptr;
  }

  while (peek() && peek()->tokentype == TokenType::DOUBLE_PIPE) {
    TokenType op = consume()->tokentype;

    std::unique_ptr<Expr> right_expr = parseLogicalAnd();

    if (!right_expr) {
      error("Expected expression after ||");
      return nullptr;
    }

    left_expr = std::make_unique<BinaryExpr>(std::move(left_expr), op, std::move(right_expr));
  }

  return left_expr;
}

std::unique_ptr<Expr> Parser::parseConditional() {
  auto condition = parseLogicalOr();
  if (!condition) {
    return nullptr;
  }

  if (peek().has_value() && peek()->tokentype == TokenType::QUESTION_MARK) {
    consume(); // ?
    auto true_expr = parseAssignment();

    if (!true_expr) {
      error("Expected expression after ?");
      return nullptr;
    }

    if (!match(TokenType::COLON)) {
      return nullptr;
    }

    auto false_expr = parseAssignment();
    if (!false_expr) {
      error("Expected expression after :");
      return nullptr;
    }

    return std::make_unique<ConditionalExpr>(std::move(condition), std::move(true_expr), std::move(false_expr));
  } else {
    return condition;
  }
}

std::unique_ptr<Expr> Parser::parseAssignment() {
  auto lhs = parseConditional();

  if (!lhs) {
    return nullptr;
  }
  if (peek() && peek()->tokentype == TokenType::EQUALS) {
    if (!isAssignable(lhs.get())) {
      error("Invalid assignment target");
      return nullptr;
    }
    consume(); // =
    auto rhs = parseAssignment();
    if (!rhs) {
      error("Expected expression after =");
      return nullptr;
    }
    return std::make_unique<AssignmentExpr>(std::move(lhs), std::move(rhs));
  } else if (peek() && ((peek()->tokentype == TokenType::PLUS_EQUALS) || (peek()->tokentype == TokenType::MINUS_EQUALS) || (peek()->tokentype == TokenType::MULTIPLY_EQUALS) || (peek()->tokentype == TokenType::DIVIDE_EQUALS) || (peek()->tokentype == TokenType::MOD_EQUALS) || (peek()->tokentype == TokenType::LEFT_SHIFT_EQUALS) || (peek()->tokentype == TokenType::RIGHT_SHIFT_EQUALS) || (peek()->tokentype == TokenType::AMPERSAND_EQUALS) || (peek()->tokentype == TokenType::PIPE_EQUALS) || (peek()->tokentype == TokenType::CARET_EQUALS))) {

    TokenType binary_op = compound_to_binary(consume()->tokentype); // also consumes compound assignment operator

    if (binary_op == TokenType::INVALID) {
      error("Received Invalid tokentype in compound assignment");
      return nullptr;
    }

    if (!isAssignable(lhs.get())) {
      error("Invalid assignment target");
      return nullptr;
    }

    auto lhs_copy = lhs->clone();
    auto rhs = parseAssignment();
    if (!rhs) {
      error("Expected expression after compound assignment");
      return nullptr;
    }

    auto binary = std::make_unique<BinaryExpr>(std::move(lhs_copy), binary_op, std::move(rhs));
    return std::make_unique<AssignmentExpr>(std::move(lhs), std::move(binary));
  }
  return lhs;
}

std::unique_ptr<Expr> Parser::parseComma() {
  auto left = parseAssignment();
  if (!left) {
    return nullptr;
  }

  while (peek() && peek()->tokentype == TokenType::COMMA) {
    TokenType op = consume()->tokentype;
    auto right = parseAssignment();
    if (!right) {
      error("Expected expression after ','");
      return nullptr;
    }

    left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseExpr() {
  return parseComma();
}

std::optional<std::vector<std::unique_ptr<Expr>>> Parser::parseArguments() {
  std::vector<std::unique_ptr<Expr>> args;

  while (true) {
    auto expr = parseAssignment();

    if (!expr) {
      return std::nullopt;
    }

    args.push_back(std::move(expr));

    if (peek() && peek()->tokentype == TokenType::COMMA) {
      consume();
      continue;
    }

    break;
  }

  return args;
}