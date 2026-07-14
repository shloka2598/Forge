#pragma once

#include "../lexer/lexer.h"
#include "../types/ParsedTypes.h"
#include "../types/Types.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

enum class ExprType {
  INT_LITERAL,
  DOUBLE_LITERAL,
  FLOAT_LITERAL,
  CHAR_LITERAL,
  STRING_LITERAL,
  IDENTIFIER,
  BINARY,
  UNARY,
  PRE_INCREMENT,
  PRE_DECREMENT,
  POST_INCREMENT,
  POST_DECREMENT,
  FUNCTION_CALL,
  CAST,
  ARRAY_ACCESS,
  ASSIGNMENT,
  CONDITIONAL,
  MEMBER_ACCESS,
  SIZEOF
};

enum class ValueCategory {
  LVALUE,
  RVALUE
};

struct Expr {
  Token token;

  Type *type = nullptr;
  ValueCategory value_category = ValueCategory::RVALUE;

  virtual ExprType expr_type() const = 0;
  virtual void show_expr(int indent = 0) const = 0;
  virtual std::unique_ptr<Expr> clone() const = 0;

  inline void print_indent(int indent) const {
    for (int i = 0; i < indent; i++) {
      std::cout << "  ";
    }
  }

  inline void show_type(int indent = 0) const {
    print_indent(indent);

    if (type) {
      std::cout << "Type = " << type->to_string();
      std::cout << " (" << (value_category == ValueCategory::LVALUE ? "lvalue" : "rvalue") << ")\n";
    } else {
      std::cout << "Type = <unknown>\n";
    }
  }

  virtual ~Expr() = default;
};

struct IntLetExpr : Expr {
  int64_t value;

  explicit IntLetExpr(Token token, int64_t value) : value{value} {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    return ExprType::INT_LITERAL;
  }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<IntLetExpr>(token, value);
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    std::cout << "IntLiteral(" << value << ")" << std::endl;
    show_type(indent);
  }
};

struct DoubleLetExpr : Expr {
  double value;

  explicit DoubleLetExpr(Token token, double value) : value{value} {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    return ExprType::DOUBLE_LITERAL;
  }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<DoubleLetExpr>(token, value);
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    std::cout << "DoubleLiteral(" << value << ")" << std::endl;
    show_type(indent);
  }
};

struct FloatLetExpr : Expr {
  float value;

  explicit FloatLetExpr(Token token, float value) : value{value} {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    return ExprType::FLOAT_LITERAL;
  }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<FloatLetExpr>(token, value);
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    std::cout << "FloatLiteral(" << value << ")" << std::endl;
    show_type(indent);
  }
};

struct CharLetExpr : Expr {
  char value;

  explicit CharLetExpr(Token token, char value) : value{value} {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    return ExprType::CHAR_LITERAL;
  }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<CharLetExpr>(token, value);
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    std::cout << "CharLiteral(" << value << ")" << std::endl;
    show_type(indent);
  }
};

struct StringLiteralExpr : Expr {
  std::string value;

  explicit StringLiteralExpr(Token token, std::string value) : value(std::move(value)) {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    return ExprType::STRING_LITERAL;
  }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<StringLiteralExpr>(token, value);
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    std::cout << "StringLiteral(\"" << value << "\")\n";
    show_type(indent);
  }
};

struct IdentifierExpr : Expr {
  std::string identifier_name;

  explicit IdentifierExpr(Token token, const std::string &name) : identifier_name{name} {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    return ExprType::IDENTIFIER;
  }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<IdentifierExpr>(token, identifier_name);
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    std::cout << "Identifier(" << identifier_name << ")" << std::endl;
    show_type(indent);
  }
};

struct BinaryExpr : Expr {
  std::unique_ptr<Expr> left_expr;
  TokenType op;
  std::unique_ptr<Expr> right_expr;

  BinaryExpr(Token token, std::unique_ptr<Expr> left, TokenType op, std::unique_ptr<Expr> right) : left_expr{std::move(left)}, op{op}, right_expr{std::move(right)} {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    return ExprType::BINARY;
  }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<BinaryExpr>(token, left_expr->clone(), op, right_expr->clone());
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    std::cout << "BinaryExpr(" << op << ")\n";
    show_type(indent);

    if (left_expr) {
      left_expr->show_expr(indent + 1);
    }

    if (right_expr) {
      right_expr->show_expr(indent + 1);
    }
  }
};

struct UnaryExpr : Expr {
  TokenType op;
  std::unique_ptr<Expr> right_expr;

  UnaryExpr(Token token, TokenType op, std::unique_ptr<Expr> right) : op{op}, right_expr{std::move(right)} {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    return ExprType::UNARY;
  }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<UnaryExpr>(token, op, right_expr->clone());
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    std::cout << "UnaryExpr(" << op << ")\n";
    show_type(indent);

    if (right_expr) {
      right_expr->show_expr(indent + 1);
    }
  }
};

struct IncrementExpr : Expr {
  bool is_prefix;
  bool is_increment;
  std::unique_ptr<Expr> operand;
  IncrementExpr(Token token, bool prefix, bool increment, std::unique_ptr<Expr> expr)
      : is_prefix(prefix), is_increment(increment), operand(std::move(expr)) {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    if (is_prefix) {
      return is_increment ? ExprType::PRE_INCREMENT : ExprType::PRE_DECREMENT;
    }
    return is_increment ? ExprType::POST_INCREMENT : ExprType::POST_DECREMENT;
  }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<IncrementExpr>(token, is_prefix, is_increment, operand->clone());
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    if (is_prefix) {
      std::cout << (is_increment ? "PreIncrement\n" : "PreDecrement\n");
    } else {
      std::cout << (is_increment ? "PostIncrement\n" : "PostDecrement\n");
    }
    show_type(indent);
    if (operand) {
      operand->show_expr(indent + 1);
    }
  }
};

struct FunctionCallExpr : Expr {
  std::string function_name;
  std::vector<std::unique_ptr<Expr>> arguments;

  FunctionCallExpr(Token token, std::string name, std::vector<std::unique_ptr<Expr>> args) : function_name(std::move(name)), arguments(std::move(args)) {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    return ExprType::FUNCTION_CALL;
  }

  std::unique_ptr<Expr> clone() const override {
    std::vector<std::unique_ptr<Expr>> args;
    for (const auto &arg : arguments) {
      args.push_back(arg->clone());
    }
    return std::make_unique<FunctionCallExpr>(token, function_name, std::move(args));
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    std::cout << "FunctionCall(" << function_name << ")\n";
    show_type(indent);
    for (const auto &arg : arguments) {
      arg->show_expr(indent + 1);
    }
  }
};

struct CastExpr : Expr {
  ParsedType target_type;
  std::unique_ptr<Expr> expr;

  CastExpr(Token token, ParsedType type, std::unique_ptr<Expr> expr) : target_type(type), expr(std::move(expr)) {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    return ExprType::CAST;
  }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<CastExpr>(token, target_type, expr->clone());
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    std::cout << "Cast\n";

    show_type(indent);

    print_indent(indent + 1);
    std::cout << "Target Type\n";
    target_type.show(indent + 2);

    print_indent(indent + 1);
    std::cout << "Expression\n";

    if (expr) {
      expr->show_expr(indent + 2);
    }
  }
};

struct ArrayAccessExpr : Expr {
  std::unique_ptr<Expr> base_expr;
  std::unique_ptr<Expr> index_expr;

  ArrayAccessExpr(Token token, std::unique_ptr<Expr> base, std::unique_ptr<Expr> index)
      : base_expr(std::move(base)), index_expr(std::move(index)) {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    return ExprType::ARRAY_ACCESS;
  }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<ArrayAccessExpr>(token, base_expr->clone(), index_expr->clone());
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    std::cout << "ArrayAccess\n";

    show_type(indent);

    print_indent(indent + 1);
    std::cout << "Base\n";

    if (base_expr) {
      base_expr->show_expr(indent + 2);
    }

    print_indent(indent + 1);
    std::cout << "Index\n";

    if (index_expr) {
      index_expr->show_expr(indent + 2);
    }
  }
};

struct AssignmentExpr : Expr {
  std::unique_ptr<Expr> lhs;
  std::unique_ptr<Expr> rhs;

  AssignmentExpr(Token token, std::unique_ptr<Expr> lhs_, std::unique_ptr<Expr> rhs_)
      : lhs(std::move(lhs_)), rhs(std::move(rhs_)) {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    return ExprType::ASSIGNMENT;
  }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<AssignmentExpr>(token, lhs->clone(), rhs->clone());
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    std::cout << "AssignmentExpr\n";
    show_type(indent);
    print_indent(indent + 1);
    std::cout << "LHS\n";

    if (lhs) {
      lhs->show_expr(indent + 2);
    }

    print_indent(indent + 1);
    std::cout << "RHS\n";

    if (rhs) {
      rhs->show_expr(indent + 2);
    }
  }
};

struct ConditionalExpr : Expr {
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Expr> true_expr;
  std::unique_ptr<Expr> false_expr;

  ConditionalExpr(Token token, std::unique_ptr<Expr> condition_, std::unique_ptr<Expr> true_expr_,
                  std::unique_ptr<Expr> false_expr_) : condition{std::move(condition_)}, true_expr{std::move(true_expr_)}, false_expr{std::move(false_expr_)} {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    return ExprType::CONDITIONAL;
  }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<ConditionalExpr>(token, condition->clone(), true_expr->clone(), false_expr->clone());
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    std::cout << "Conditional\n";
    show_type(indent);

    print_indent(indent + 1);
    std::cout << "Condition\n";

    if (condition) {
      condition->show_expr(indent + 2);
    }

    print_indent(indent + 1);
    std::cout << "True Expression\n";

    if (true_expr) {
      true_expr->show_expr(indent + 2);
    }

    print_indent(indent + 1);
    std::cout << "False Expression\n";

    if (false_expr) {
      false_expr->show_expr(indent + 2);
    }
  }
};

struct MemberAccessExpr : Expr {
  std::unique_ptr<Expr> base_expr;
  std::string member_name;
  TokenType op;

  MemberAccessExpr(Token token, std::unique_ptr<Expr> base_expr_, std::string member_name_, TokenType op_) : base_expr(std::move(base_expr_)), member_name(std::move(member_name_)), op(op_) {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    return ExprType::MEMBER_ACCESS;
  }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<MemberAccessExpr>(token, base_expr->clone(), member_name, op);
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    std::cout << "Member Access ";
    if (op == TokenType::DOT) {
      std::cout << "(.)\n";
    } else {
      std::cout << "(->)\n";
    }
    show_type(indent);
    print_indent(indent + 1);
    std::cout << "Base Expression\n";
    if (base_expr) {
      base_expr->show_expr(indent + 2);
    }
    print_indent(indent + 1);
    std::cout << "Member Name : " << member_name << "\n";
  }
};

struct SizeofExpr : Expr {
  std::optional<ParsedType> parsed_type;
  std::unique_ptr<Expr> expr;

  explicit SizeofExpr(Token token, ParsedType type) : parsed_type(std::move(type)) {
    this->token = std::move(token);
  }

  explicit SizeofExpr(Token token, std::unique_ptr<Expr> expr_) : expr(std::move(expr_)) {
    this->token = std::move(token);
  }

  ExprType expr_type() const override {
    return ExprType::SIZEOF;
  }

  std::unique_ptr<Expr> clone() const override {
    if (parsed_type.has_value()) {
      return std::make_unique<SizeofExpr>(token, *parsed_type);
    }

    return std::make_unique<SizeofExpr>(token, expr->clone());
  }

  void show_expr(int indent = 0) const override {
    print_indent(indent);
    std::cout << "Sizeof Expression\n";

    show_type(indent);

    if (parsed_type.has_value()) {
      print_indent(indent + 1);
      std::cout << "Type Operand\n";
      parsed_type->show(indent + 2);
    } else {
      print_indent(indent + 1);
      std::cout << "Expression Operand\n";

      if (expr) {
        expr->show_expr(indent + 2);
      }
    }
  }
};