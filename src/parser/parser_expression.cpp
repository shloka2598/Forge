#include "parser.h"
#include <cassert>

std::unique_ptr<Expr> Parser::parsePrimary() {
  if (!peek()) {
    error("Expected a primary expression before end of file.", std::nullopt);
    return nullptr;
  }

  TokenType tok = peek()->tokentype;

  if (tok == TokenType::INT_LET) {
    auto token = consume();
    return std::make_unique<IntLetExpr>(*token, std::stoull(token->value.value(), nullptr, 0));
  }
  if (tok == TokenType::DOUBLE_LET) {
    auto token = consume();
    return std::make_unique<DoubleLetExpr>(*token, std::stod(token->value.value()));
  }
  if (tok == TokenType::FLOAT_LET) {
    auto token = consume();
    return std::make_unique<FloatLetExpr>(*token, std::stof(token->value.value()));
  }
  if (tok == TokenType::CHAR_LET) {
    auto token = consume();
    return std::make_unique<CharLetExpr>(*token, token->value.value()[0]);
  }
  if (tok == TokenType::STRING_LET) {
    auto token = consume();
    return std::make_unique<StringLiteralExpr>(*token, token->value.value());
  }
  if (tok == TokenType::IDENTIFIER) {
    auto token = consume();
    return std::make_unique<IdentifierExpr>(*token, token->value.value());
  }
  if (tok == TokenType::PARENTHESIS_OPEN) {
    consume(); // (
    auto expr = parseExpr();
    if (!expr) {
      recoverUntil(TokenType::PARENTHESIS_CLOSE);

      if (!match(TokenType::PARENTHESIS_CLOSE)) {
        return nullptr;
      }

      return nullptr;
    }
    if (!match(TokenType::PARENTHESIS_CLOSE)) {
      return nullptr;
    }
    return expr;
  }

  error("Expected a primary expression.", peek());
  return nullptr;
}

std::unique_ptr<Expr> Parser::parseUnaryAndCasting() {
  if (!peek()) {
    error("Expected an expression before end of file.", std::nullopt);
    return nullptr;
  }
  TokenType tok = peek()->tokentype;

  if (tok == TokenType::PLUS_PLUS) {
    auto op = consume();
    auto expr = parseUnaryAndCasting();
    if (!expr) {
      error("Expected an expression after '++'.", op);
      return nullptr;
    }
    if (!isAssignable(expr.get())) {
      error("Operand of '++' must be assignable.", peek());
      return nullptr;
    }
    return std::make_unique<IncrementExpr>(*op, true, true, std::move(expr));
  }

  if (tok == TokenType::MINUS_MINUS) {
    auto op = consume();
    auto expr = parseUnaryAndCasting();
    if (!expr) {
      error("Expected an expression after '--'.", op);
      return nullptr;
    }
    if (!isAssignable(expr.get())) {
      error("Operand of '--' must be assignable.", peek());
      return nullptr;
    }
    return std::make_unique<IncrementExpr>(*op, true, false, std::move(expr));
  }

  // Unary +, -, !, *(dereference), &(address-of), ~
  if (tok == TokenType::PLUS || tok == TokenType::MINUS || tok == TokenType::EXCLAMATION || tok == TokenType::AMPERSAND || tok == TokenType::MULTIPLY || tok == TokenType::TILDE) {
    auto op = consume(); // op
    auto right_expr = parseUnaryAndCasting();
    if (!right_expr) {
      return nullptr;
    }
    return std::make_unique<UnaryExpr>(*op, tok, std::move(right_expr));
  }

  // sizeof
  if (tok == TokenType::SIZEOF) {
    auto op = consume();
    // sizeof(type)
    if (isCastExpression()) {
      consume(); // (
      ParsedType type = parseDatatype();
      if (type.datatype == DataType::INVALID) {
        recoverUntil(TokenType::PARENTHESIS_CLOSE);

        if (!match(TokenType::PARENTHESIS_CLOSE)) {
          return nullptr;
        }

        return nullptr;
      }

      parsePointerSuffix(type);

      if (!match(TokenType::PARENTHESIS_CLOSE)) {
        return nullptr;
      }

      return std::make_unique<SizeofExpr>(*op, std::move(type));
    }

    auto expr = parseUnaryAndCasting();

    if (!expr) {
      error("Expected an expression after 'sizeof'.", op);
      return nullptr;
    }

    return std::make_unique<SizeofExpr>(*op, std::move(expr));
  }

  // Cast expression
  if (isCastExpression()) {
    auto opening_paran = consume(); // (
    ParsedType type = parseDatatype();
    if (type.datatype == DataType::INVALID) {
      recoverUntil(TokenType::PARENTHESIS_CLOSE);

      if (!match(TokenType::PARENTHESIS_CLOSE)) {
        return nullptr;
      }

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
    return std::make_unique<CastExpr>(*opening_paran, std::move(type), std::move(expr));
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
          recoverUntil(TokenType::PARENTHESIS_CLOSE);

          if (!match(TokenType::PARENTHESIS_CLOSE)) {
            return nullptr;
          }

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
        error("Expected an identifier before '('.", peek());
        return nullptr;
      }
      expr = std::make_unique<FunctionCallExpr>(id->token, id->identifier_name, std::move(args));
      continue;
    }

    // Array Access
    if (peek()->tokentype == TokenType::SQUARE_BRACKETS_OPEN) {
      auto open_arr_brack = consume(); // [
      auto index_expr = parseExpr();
      if (!index_expr) {
        recoverUntil(TokenType::SQUARE_BRACKETS_CLOSE);
        if (!match(TokenType::SQUARE_BRACKETS_CLOSE)) {
          return nullptr;
        }
        return nullptr;
      }
      if (!match(TokenType::SQUARE_BRACKETS_CLOSE)) {
        return nullptr;
      }
      expr = std::make_unique<ArrayAccessExpr>(*open_arr_brack, std::move(expr), std::move(index_expr));
      continue;
    }

    // Member Access (structs and unions)
    if (peek()->tokentype == TokenType::DOT) {
      auto op = consume(); // .
      if (!peek() || peek()->tokentype != TokenType::IDENTIFIER) {
        error("Expected a member name after '.'.", peek());
        return nullptr;
      }

      std::string member = consume()->value.value();
      expr = std::make_unique<MemberAccessExpr>(*op, std::move(expr), member, TokenType::DOT);
      continue;
    }

    // Pointer Member Access (structs and unions)
    if (peek()->tokentype == TokenType::ARROW) {
      auto op = consume(); // ->
      if (!peek() || peek()->tokentype != TokenType::IDENTIFIER) {
        error("Expected a member name after '->'.", peek());
        return nullptr;
      }

      std::string member = consume()->value.value();
      expr = std::make_unique<MemberAccessExpr>(*op, std::move(expr), member, TokenType::ARROW);

      continue;
    }

    // Post ++
    if (peek()->tokentype == TokenType::PLUS_PLUS) {
      auto op = consume();

      if (!isAssignable(expr.get())) {
        error("Operand of postfix '++' must be assignable.", peek());
        return nullptr;
      }

      expr = std::make_unique<IncrementExpr>(*op, false, true, std::move(expr));
      continue;
    }

    // Post --
    if (peek()->tokentype == TokenType::MINUS_MINUS) {
      auto op = consume();
      if (!isAssignable(expr.get())) {
        error("Operand of postfix '--' must be assignable.", peek());
        return nullptr;
      }
      expr = std::make_unique<IncrementExpr>(*op, false, false, std::move(expr));
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

    auto op = consume();

    std::unique_ptr<Expr> right_expr = parseUnaryAndCasting();

    if (!right_expr) {
      return nullptr;
    }

    left_expr = std::make_unique<BinaryExpr>(*op, std::move(left_expr), op->tokentype, std::move(right_expr));
  }

  return left_expr;
}

std::unique_ptr<Expr> Parser::parseAdditionAndSubtraction() {
  std::unique_ptr<Expr> left_expr = parseMultiplicationAndDivision();

  if (!left_expr) {
    return nullptr;
  }

  while (peek() && (peek()->tokentype == TokenType::PLUS || peek()->tokentype == TokenType::MINUS)) {
    auto op = consume();

    std::unique_ptr<Expr> right_expr = parseMultiplicationAndDivision();

    if (!right_expr) {
      return nullptr;
    }

    left_expr = std::make_unique<BinaryExpr>(*op, std::move(left_expr), op->tokentype, std::move(right_expr));
  }

  return left_expr;
}

std::unique_ptr<Expr> Parser::parseShift() {
  auto left = parseAdditionAndSubtraction();
  if (!left) {
    return nullptr;
  }

  while (peek().has_value() && (peek()->tokentype == TokenType::LEFT_SHIFT || peek()->tokentype == TokenType::RIGHT_SHIFT)) {
    auto op = consume();
    auto right = parseAdditionAndSubtraction();

    if (!right) {
      return nullptr;
    }

    left = std::make_unique<BinaryExpr>(*op, std::move(left), op->tokentype, std::move(right));
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseRelational() {
  std::unique_ptr<Expr> left_expr = parseShift();

  if (!left_expr) {
    return nullptr;
  }

  while (peek() && (peek()->tokentype == TokenType::GREATER_THAN || peek()->tokentype == TokenType::SMALLER_THAN || peek()->tokentype == TokenType::GREATER_THAN_EQUAL_THAN || peek()->tokentype == TokenType::SMALLER_THAN_EQUAL_THAN)) {

    auto op = consume();

    std::unique_ptr<Expr> right_expr = parseShift();

    if (!right_expr) {
      return nullptr;
    }

    left_expr = std::make_unique<BinaryExpr>(*op, std::move(left_expr), op->tokentype, std::move(right_expr));
  }

  return left_expr;
}

std::unique_ptr<Expr> Parser::parseEquality() {
  auto left = parseRelational();
  if (!left) {
    return nullptr;
  }

  while (peek() && (peek()->tokentype == TokenType::DOUBLE_EQUALS || peek()->tokentype == TokenType::NOT_EQUALS)) {
    auto op = consume();

    auto right = parseRelational();
    if (!right) {
      return nullptr;
    }
    left = std::make_unique<BinaryExpr>(*op, std::move(left), op->tokentype, std::move(right));
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseBitwiseAnd() {
  auto left = parseEquality();
  if (!left) {
    return nullptr;
  }

  while (peek() && peek()->tokentype == TokenType::AMPERSAND) {
    auto op = consume();
    auto right = parseEquality();
    if (!right) {
      return nullptr;
    }
    left = std::make_unique<BinaryExpr>(*op, std::move(left), op->tokentype, std::move(right));
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseBitwiseXor() {
  auto left = parseBitwiseAnd();
  if (!left) {
    return nullptr;
  }

  while (peek() && peek()->tokentype == TokenType::CARET) {
    auto op = consume();
    auto right = parseBitwiseAnd();
    if (!right) {
      return nullptr;
    }
    left = std::make_unique<BinaryExpr>(*op, std::move(left), op->tokentype, std::move(right));
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseBitwiseOr() {
  auto left = parseBitwiseXor();
  if (!left) {
    return nullptr;
  }

  while (peek() && peek()->tokentype == TokenType::PIPE) {
    auto op = consume();
    auto right = parseBitwiseXor();
    if (!right) {
      return nullptr;
    }
    left = std::make_unique<BinaryExpr>(*op, std::move(left), op->tokentype, std::move(right));
  }

  return left;
}

std::unique_ptr<Expr> Parser::parseLogicalAnd() {
  std::unique_ptr<Expr> left_expr = parseBitwiseOr();

  if (!left_expr) {
    return nullptr;
  }

  while (peek() && peek()->tokentype == TokenType::DOUBLE_AMPERSAND) {
    auto op = consume();

    std::unique_ptr<Expr> right_expr = parseBitwiseOr();

    if (!right_expr) {
      return nullptr;
    }

    left_expr = std::make_unique<BinaryExpr>(*op, std::move(left_expr), op->tokentype, std::move(right_expr));
  }

  return left_expr;
}

std::unique_ptr<Expr> Parser::parseLogicalOr() {
  std::unique_ptr<Expr> left_expr = parseLogicalAnd();

  if (!left_expr) {
    return nullptr;
  }

  while (peek() && peek()->tokentype == TokenType::DOUBLE_PIPE) {
    auto op = consume();

    std::unique_ptr<Expr> right_expr = parseLogicalAnd();

    if (!right_expr) {
      return nullptr;
    }

    left_expr = std::make_unique<BinaryExpr>(*op, std::move(left_expr), op->tokentype, std::move(right_expr));
  }

  return left_expr;
}

std::unique_ptr<Expr> Parser::parseConditional() {
  auto condition = parseLogicalOr();
  if (!condition) {
    return nullptr;
  }

  if (peek().has_value() && peek()->tokentype == TokenType::QUESTION_MARK) {
    auto question_mark = consume(); // ?
    auto true_expr = parseAssignment();

    if (!true_expr) {
      error("Expected an expression after '?'.", peek());

      recoverUntil(TokenType::COLON);

      if (!match(TokenType::COLON)) {
        return nullptr;
      }

      auto false_expr = parseAssignment();

      if (!false_expr) {
        recoverExpression();
      }

      return nullptr;
    }

    if (!match(TokenType::COLON)) {
      return nullptr;
    }

    auto false_expr = parseAssignment();

    if (!false_expr) {
      error("Expected an expression after ':'.", peek());
      recoverExpression();
      return nullptr;
    }

    return std::make_unique<ConditionalExpr>(*question_mark, std::move(condition), std::move(true_expr), std::move(false_expr));
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
      error("Left-hand side of assignment must be assignable.", peek());
      return nullptr;
    }
    auto op = consume(); // =
    auto rhs = parseAssignment();
    if (!rhs) {
      error("Expected an expression after '='.", peek());
      return nullptr;
    }
    return std::make_unique<AssignmentExpr>(*op, std::move(lhs), std::move(rhs));
  } else if (peek() && ((peek()->tokentype == TokenType::PLUS_EQUALS) || (peek()->tokentype == TokenType::MINUS_EQUALS) || (peek()->tokentype == TokenType::MULTIPLY_EQUALS) || (peek()->tokentype == TokenType::DIVIDE_EQUALS) || (peek()->tokentype == TokenType::MOD_EQUALS) || (peek()->tokentype == TokenType::LEFT_SHIFT_EQUALS) || (peek()->tokentype == TokenType::RIGHT_SHIFT_EQUALS) || (peek()->tokentype == TokenType::AMPERSAND_EQUALS) || (peek()->tokentype == TokenType::PIPE_EQUALS) || (peek()->tokentype == TokenType::CARET_EQUALS))) {

    auto op = consume();
    TokenType binary_op = compound_to_binary(op->tokentype); // also consumes compound assignment operator

    assert(binary_op != TokenType::INVALID);

    if (!isAssignable(lhs.get())) {
      error("Left-hand side of assignment must be assignable.", peek());
      return nullptr;
    }

    auto lhs_copy = lhs->clone();
    auto rhs = parseAssignment();
    if (!rhs) {
      error("Expected an expression after compound assignment operator.", peek());
      return nullptr;
    }

    auto binary = std::make_unique<BinaryExpr>(*op, std::move(lhs_copy), binary_op, std::move(rhs));
    return std::make_unique<AssignmentExpr>(*op, std::move(lhs), std::move(binary));
  }
  return lhs;
}

std::unique_ptr<Expr> Parser::parseComma() {
  auto left = parseAssignment();
  if (!left) {
    return nullptr;
  }

  while (peek() && peek()->tokentype == TokenType::COMMA) {
    auto op = consume();
    auto right = parseAssignment();
    if (!right) {
      return nullptr;
    }

    left = std::make_unique<BinaryExpr>(*op, std::move(left), op->tokentype, std::move(right));
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
      recoverExpression();

      if (peek() && peek()->tokentype == TokenType::COMMA) {
        consume();
        continue;
      }

      break;
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