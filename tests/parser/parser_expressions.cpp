#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "common/test_utils.h"

TEST_CASE("Parser parses int literal expression in return stmt") {
  auto result = parse("int main() { return 10; }");
  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::INT_LITERAL);

  IntLetExpr *int_lit_expr = dynamic_cast<IntLetExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(int_lit_expr != nullptr);
  REQUIRE(int_lit_expr->value == 10);
}

TEST_CASE("Parser parses double literal expression in return stmt") {
  auto result = parse("int main() { return 69.79; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::DOUBLE_LITERAL);

  DoubleLetExpr *double_lit_expr = dynamic_cast<DoubleLetExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(double_lit_expr != nullptr);
  REQUIRE(double_lit_expr->value == 69.79);
}

TEST_CASE("Parser parses float literal expression in return stmt") {
  auto result = parse("int main() { return 69.79f; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::FLOAT_LITERAL);

  FloatLetExpr *float_lit_expr = dynamic_cast<FloatLetExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(float_lit_expr != nullptr);
  REQUIRE(float_lit_expr->value == 69.79f);
}

TEST_CASE("Parser parses char literal expression in return stmt") {
  auto result = parse("int main() { return 'r'; }");
  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::CHAR_LITERAL);

  CharLetExpr *char_lit_expr = dynamic_cast<CharLetExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(char_lit_expr != nullptr);
  REQUIRE(char_lit_expr->value == 'r');
}

TEST_CASE("Parser parses string literal expression in return stmt") {
  auto result = parse("int main() { return \"some_literal_name\"; }");
  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::STRING_LITERAL);

  StringLiteralExpr *string_literal_expr = dynamic_cast<StringLiteralExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(string_literal_expr != nullptr);
  REQUIRE(string_literal_expr->value == "some_literal_name");
}

TEST_CASE("Parser parses identifier expression in return stmt") {
  auto result = parse("int main() { return variable_name; }");
  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::IDENTIFIER);

  IdentifierExpr *identifier_expr = dynamic_cast<IdentifierExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(identifier_expr != nullptr);
  REQUIRE(identifier_expr->identifier_name == "variable_name");
}

TEST_CASE("Parser parses int literal expression inside parenthesis in return stmt") {
  auto result = parse("int main() { return (10); }");
  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::INT_LITERAL);

  IntLetExpr *int_let_expr = dynamic_cast<IntLetExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(int_let_expr != nullptr);
  REQUIRE(int_let_expr->value == 10);
}

TEST_CASE("Parser parses binary expressions with integer literal operands") {
  auto [expr, op] = GENERATE(
      std::pair{"10 + 20", TokenType::PLUS},
      std::pair{"10 - 20", TokenType::MINUS},
      std::pair{"10 * 20", TokenType::MULTIPLY},
      std::pair{"10 / 20", TokenType::DIVIDE},
      std::pair{"10 % 20", TokenType::MODULO});

  DYNAMIC_SECTION("Operator = " << op) {
    std::string source = "int main() { return ";
    source += expr;
    source += "; }";

    auto result = parse(source);

    ReturnStmt *return_stmt = get_return_stmt(result);

    REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::BINARY);

    BinaryExpr *binary_expr = dynamic_cast<BinaryExpr *>(return_stmt->expr_ptr.get());

    REQUIRE(binary_expr != nullptr);
    REQUIRE(binary_expr->op == op);

    REQUIRE(binary_expr->left_expr != nullptr);
    REQUIRE(binary_expr->right_expr != nullptr);

    REQUIRE(binary_expr->left_expr->expr_type() == ExprType::INT_LITERAL);
    REQUIRE(binary_expr->right_expr->expr_type() == ExprType::INT_LITERAL);

    IntLetExpr *left_expr = dynamic_cast<IntLetExpr *>(binary_expr->left_expr.get());
    IntLetExpr *right_expr = dynamic_cast<IntLetExpr *>(binary_expr->right_expr.get());

    REQUIRE(left_expr != nullptr);
    REQUIRE(right_expr != nullptr);

    REQUIRE(left_expr->value == 10);
    REQUIRE(right_expr->value == 20);
  }
}

TEST_CASE("Parser parses binary expressions with identifier operands") {
  auto [expr, op] = GENERATE(
      std::pair{"identifier_1 + identifier_2", TokenType::PLUS},
      std::pair{"identifier_1 - identifier_2", TokenType::MINUS},
      std::pair{"identifier_1 * identifier_2", TokenType::MULTIPLY},
      std::pair{"identifier_1 / identifier_2", TokenType::DIVIDE},
      std::pair{"identifier_1 % identifier_2", TokenType::MODULO});

  DYNAMIC_SECTION("Operator = " << op) {
    std::string source = "int main() { return ";
    source += expr;
    source += "; }";

    auto result = parse(source);

    ReturnStmt *return_stmt = get_return_stmt(result);

    REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::BINARY);

    BinaryExpr *binary_expr = dynamic_cast<BinaryExpr *>(return_stmt->expr_ptr.get());

    REQUIRE(binary_expr != nullptr);
    REQUIRE(binary_expr->op == op);

    REQUIRE(binary_expr->left_expr != nullptr);
    REQUIRE(binary_expr->right_expr != nullptr);

    REQUIRE(binary_expr->left_expr->expr_type() == ExprType::IDENTIFIER);
    REQUIRE(binary_expr->right_expr->expr_type() == ExprType::IDENTIFIER);

    IdentifierExpr *left_expr = dynamic_cast<IdentifierExpr *>(binary_expr->left_expr.get());
    IdentifierExpr *right_expr = dynamic_cast<IdentifierExpr *>(binary_expr->right_expr.get());

    REQUIRE(left_expr != nullptr);
    REQUIRE(right_expr != nullptr);

    REQUIRE(left_expr->identifier_name == "identifier_1");
    REQUIRE(right_expr->identifier_name == "identifier_2");
  }
}

TEST_CASE("Parser respects multiplication precedence over addition") {
  auto result = parse("int main() { return 1 + 2 * 3; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *root = dynamic_cast<BinaryExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(root != nullptr);
  REQUIRE(root->op == TokenType::PLUS);

  auto *left = dynamic_cast<IntLetExpr *>(root->left_expr.get());
  auto *right = dynamic_cast<BinaryExpr *>(root->right_expr.get());

  REQUIRE(left != nullptr);
  REQUIRE(right != nullptr);

  REQUIRE(left->value == 1);
  REQUIRE(right->op == TokenType::MULTIPLY);

  auto *mul_left = dynamic_cast<IntLetExpr *>(right->left_expr.get());
  auto *mul_right = dynamic_cast<IntLetExpr *>(right->right_expr.get());

  REQUIRE(mul_left != nullptr);
  REQUIRE(mul_right != nullptr);

  REQUIRE(mul_left->value == 2);
  REQUIRE(mul_right->value == 3);
}

TEST_CASE("Parser respects parenthesized expressions") {
  auto result = parse("int main() { return (1 + 2) * 3; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *root = dynamic_cast<BinaryExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(root != nullptr);
  REQUIRE(root->op == TokenType::MULTIPLY);

  auto *left = dynamic_cast<BinaryExpr *>(root->left_expr.get());
  auto *right = dynamic_cast<IntLetExpr *>(root->right_expr.get());

  REQUIRE(left != nullptr);
  REQUIRE(right != nullptr);

  REQUIRE(left->op == TokenType::PLUS);
  REQUIRE(right->value == 3);

  auto *plus_left = dynamic_cast<IntLetExpr *>(left->left_expr.get());
  auto *plus_right = dynamic_cast<IntLetExpr *>(left->right_expr.get());

  REQUIRE(plus_left != nullptr);
  REQUIRE(plus_right != nullptr);

  REQUIRE(plus_left->value == 1);
  REQUIRE(plus_right->value == 2);
}

TEST_CASE("Parser respects multiplication precedence before addition") {
  auto result = parse("int main() { return 1 * 2 + 3; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *root = dynamic_cast<BinaryExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(root != nullptr);
  REQUIRE(root->op == TokenType::PLUS);

  auto *left = dynamic_cast<BinaryExpr *>(root->left_expr.get());
  auto *right = dynamic_cast<IntLetExpr *>(root->right_expr.get());

  REQUIRE(left != nullptr);
  REQUIRE(right != nullptr);

  REQUIRE(left->op == TokenType::MULTIPLY);
  REQUIRE(right->value == 3);

  auto *mul_left = dynamic_cast<IntLetExpr *>(left->left_expr.get());
  auto *mul_right = dynamic_cast<IntLetExpr *>(left->right_expr.get());

  REQUIRE(mul_left != nullptr);
  REQUIRE(mul_right != nullptr);

  REQUIRE(mul_left->value == 1);
  REQUIRE(mul_right->value == 2);
}

TEST_CASE("Parser parses subtraction as left associative") {
  auto result = parse("int main() { return 1 - 2 - 3; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *root = dynamic_cast<BinaryExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(root != nullptr);
  REQUIRE(root->op == TokenType::MINUS);

  auto *left = dynamic_cast<BinaryExpr *>(root->left_expr.get());
  auto *right = dynamic_cast<IntLetExpr *>(root->right_expr.get());

  REQUIRE(left != nullptr);
  REQUIRE(right != nullptr);

  REQUIRE(left->op == TokenType::MINUS);
  REQUIRE(right->value == 3);

  auto *sub_left = dynamic_cast<IntLetExpr *>(left->left_expr.get());
  auto *sub_right = dynamic_cast<IntLetExpr *>(left->right_expr.get());

  REQUIRE(sub_left != nullptr);
  REQUIRE(sub_right != nullptr);

  REQUIRE(sub_left->value == 1);
  REQUIRE(sub_right->value == 2);
}

TEST_CASE("Parser parses unary plus expression") {
  auto result = parse("int main() { return +x; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::UNARY);

  auto *unary = dynamic_cast<UnaryExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(unary != nullptr);
  REQUIRE(unary->op == TokenType::PLUS);

  REQUIRE(unary->right_expr->expr_type() == ExprType::IDENTIFIER);

  auto *id = dynamic_cast<IdentifierExpr *>(unary->right_expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "x");
}

TEST_CASE("Parser parses unary minus expression") {
  auto result = parse("int main() { return -x; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::UNARY);

  auto *unary = dynamic_cast<UnaryExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(unary != nullptr);
  REQUIRE(unary->op == TokenType::MINUS);

  auto *id = dynamic_cast<IdentifierExpr *>(unary->right_expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "x");
}

TEST_CASE("Parser parses logical not expression") {
  auto result = parse("int main() { return !x; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::UNARY);

  auto *unary = dynamic_cast<UnaryExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(unary != nullptr);
  REQUIRE(unary->op == TokenType::EXCLAMATION);

  auto *id = dynamic_cast<IdentifierExpr *>(unary->right_expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "x");
}

TEST_CASE("Parser parses bitwise not expression") {
  auto result = parse("int main() { return ~x; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::UNARY);

  auto *unary = dynamic_cast<UnaryExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(unary != nullptr);
  REQUIRE(unary->op == TokenType::TILDE);

  auto *id = dynamic_cast<IdentifierExpr *>(unary->right_expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "x");
}

TEST_CASE("Parser parses dereference expression") {
  auto result = parse("int main() { return *ptr; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::UNARY);

  auto *unary = dynamic_cast<UnaryExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(unary != nullptr);
  REQUIRE(unary->op == TokenType::MULTIPLY);

  auto *id = dynamic_cast<IdentifierExpr *>(unary->right_expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "ptr");
}

TEST_CASE("Parser parses address-of expression") {
  auto result = parse("int main() { return &x; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::UNARY);

  auto *unary = dynamic_cast<UnaryExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(unary != nullptr);
  REQUIRE(unary->op == TokenType::AMPERSAND);

  auto *id = dynamic_cast<IdentifierExpr *>(unary->right_expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "x");
}

TEST_CASE("Parser parses prefix increment expression") {
  auto result = parse("int main() { return ++x; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::PRE_INCREMENT);

  auto *increment_expr = dynamic_cast<IncrementExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(increment_expr != nullptr);
  REQUIRE(increment_expr->is_increment == true);
  REQUIRE(increment_expr->is_prefix == true);

  auto *id = dynamic_cast<IdentifierExpr *>(increment_expr->operand.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "x");
}

TEST_CASE("Parser parses prefix decrement expression") {
  auto result = parse("int main() { return --x; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::PRE_DECREMENT);

  auto *decrement_expr = dynamic_cast<IncrementExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(decrement_expr != nullptr);
  REQUIRE(decrement_expr->is_increment == false);
  REQUIRE(decrement_expr->is_prefix == true);

  auto *id = dynamic_cast<IdentifierExpr *>(decrement_expr->operand.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "x");
}

TEST_CASE("Parser parses postfix increment expression") {
  auto result = parse("int main() { return x++; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::POST_INCREMENT);

  auto *increment_expr = dynamic_cast<IncrementExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(increment_expr != nullptr);
  REQUIRE(increment_expr->is_increment == true);
  REQUIRE(increment_expr->is_prefix == false);

  auto *id = dynamic_cast<IdentifierExpr *>(increment_expr->operand.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "x");
}

TEST_CASE("Parser parses postfix decrement expression") {
  auto result = parse("int main() { return x--; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::POST_DECREMENT);

  auto *decrement_expr = dynamic_cast<IncrementExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(decrement_expr != nullptr);
  REQUIRE(decrement_expr->is_increment == false);
  REQUIRE(decrement_expr->is_prefix == false);

  auto *id = dynamic_cast<IdentifierExpr *>(decrement_expr->operand.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "x");
}

TEST_CASE("Parser parses function call with no arguments") {
  auto result = parse("int main() { return foo(); }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::FUNCTION_CALL);

  auto *call = dynamic_cast<FunctionCallExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->function_name == "foo");
  REQUIRE(call->arguments.empty());
}

TEST_CASE("Parser parses function call with one integer argument") {
  auto result = parse("int main() { return foo(10); }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *call = dynamic_cast<FunctionCallExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->function_name == "foo");
  REQUIRE(call->arguments.size() == 1);

  REQUIRE(call->arguments[0]->expr_type() == ExprType::INT_LITERAL);

  auto *arg = dynamic_cast<IntLetExpr *>(call->arguments[0].get());

  REQUIRE(arg != nullptr);
  REQUIRE(arg->value == 10);
}

TEST_CASE("Parser parses function call with multiple literal arguments") {
  auto result = parse(R"(int main() { return foo(10, 3.14, 'a', "hello"); })");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *call = dynamic_cast<FunctionCallExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->function_name == "foo");
  REQUIRE(call->arguments.size() == 4);

  REQUIRE(call->arguments[0]->expr_type() == ExprType::INT_LITERAL);
  REQUIRE(call->arguments[1]->expr_type() == ExprType::DOUBLE_LITERAL);
  REQUIRE(call->arguments[2]->expr_type() == ExprType::CHAR_LITERAL);
  REQUIRE(call->arguments[3]->expr_type() == ExprType::STRING_LITERAL);
}

TEST_CASE("Parser parses function call with identifier arguments") {
  auto result = parse("int main() { return foo(a, b, c); }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *call = dynamic_cast<FunctionCallExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->arguments.size() == 3);

  auto *a = dynamic_cast<IdentifierExpr *>(call->arguments[0].get());
  auto *b = dynamic_cast<IdentifierExpr *>(call->arguments[1].get());
  auto *c = dynamic_cast<IdentifierExpr *>(call->arguments[2].get());

  REQUIRE(a != nullptr);
  REQUIRE(b != nullptr);
  REQUIRE(c != nullptr);

  REQUIRE(a->identifier_name == "a");
  REQUIRE(b->identifier_name == "b");
  REQUIRE(c->identifier_name == "c");
}

TEST_CASE("Parser parses function call with binary expression arguments") {
  auto result = parse("int main() { return foo(1 + 2, x * y); }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *call = dynamic_cast<FunctionCallExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(call != nullptr);
  REQUIRE(call->arguments.size() == 2);

  auto *first = dynamic_cast<BinaryExpr *>(call->arguments[0].get());
  auto *second = dynamic_cast<BinaryExpr *>(call->arguments[1].get());

  REQUIRE(first != nullptr);
  REQUIRE(second != nullptr);

  REQUIRE(first->op == TokenType::PLUS);
  REQUIRE(second->op == TokenType::MULTIPLY);
}

TEST_CASE("Parser parses nested function calls") {
  auto result = parse("int main() { return foo(bar()); }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *outer = dynamic_cast<FunctionCallExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(outer != nullptr);

  REQUIRE(outer->function_name == "foo");
  REQUIRE(outer->arguments.size() == 1);

  auto *inner = dynamic_cast<FunctionCallExpr *>(outer->arguments[0].get());

  REQUIRE(inner != nullptr);

  REQUIRE(inner->function_name == "bar");
  REQUIRE(inner->arguments.empty());
}

TEST_CASE("Parser parses nested function calls with arguments") {
  auto result = parse("int main() { return foo(bar(x), baz(y)); }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *outer = dynamic_cast<FunctionCallExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(outer != nullptr);
  REQUIRE(outer->arguments.size() == 2);

  auto *bar = dynamic_cast<FunctionCallExpr *>(outer->arguments[0].get());
  auto *baz = dynamic_cast<FunctionCallExpr *>(outer->arguments[1].get());

  REQUIRE(bar != nullptr);
  REQUIRE(baz != nullptr);

  REQUIRE(bar->function_name == "bar");
  REQUIRE(baz->function_name == "baz");

  REQUIRE(bar->arguments.size() == 1);
  REQUIRE(baz->arguments.size() == 1);
}

// TODO: have adding support for calling result of a function, also have to change functionCallExpr node to store std::unique_ptr<Expr> instead of a std::string name;

// TEST_CASE("Parser parses function call returning another function call") {
//   auto result = parse("int main() { return foo()(10); }");

//   ReturnStmt *return_stmt = get_return_stmt(result);

//   REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::FUNCTION_CALL);
// }

TEST_CASE("Parser parses array access with integer index") {
  auto result = parse("int main() { return arr[10]; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::ARRAY_ACCESS);

  auto *array = dynamic_cast<ArrayAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(array != nullptr);

  REQUIRE(array->base_expr->expr_type() == ExprType::IDENTIFIER);
  REQUIRE(array->index_expr->expr_type() == ExprType::INT_LITERAL);

  auto *base = dynamic_cast<IdentifierExpr *>(array->base_expr.get());
  auto *index = dynamic_cast<IntLetExpr *>(array->index_expr.get());

  REQUIRE(base != nullptr);
  REQUIRE(index != nullptr);

  REQUIRE(base->identifier_name == "arr");
  REQUIRE(index->value == 10);
}

TEST_CASE("Parser parses array access with identifier index") {
  auto result = parse("int main() { return arr[i]; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *array = dynamic_cast<ArrayAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(array != nullptr);

  REQUIRE(array->base_expr->expr_type() == ExprType::IDENTIFIER);
  REQUIRE(array->index_expr->expr_type() == ExprType::IDENTIFIER);

  auto *base = dynamic_cast<IdentifierExpr *>(array->base_expr.get());
  auto *index = dynamic_cast<IdentifierExpr *>(array->index_expr.get());

  REQUIRE(base != nullptr);
  REQUIRE(index != nullptr);

  REQUIRE(base->identifier_name == "arr");
  REQUIRE(index->identifier_name == "i");
}

TEST_CASE("Parser parses array access with binary index expression") {
  auto result = parse("int main() { return arr[i + 1]; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *array = dynamic_cast<ArrayAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(array != nullptr);

  REQUIRE(array->index_expr->expr_type() == ExprType::BINARY);

  auto *binary = dynamic_cast<BinaryExpr *>(array->index_expr.get());

  REQUIRE(binary != nullptr);
  REQUIRE(binary->op == TokenType::PLUS);
}

TEST_CASE("Parser parses multidimensional array access") {
  auto result = parse("int main() { return arr[1][2]; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *outer = dynamic_cast<ArrayAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(outer != nullptr);

  REQUIRE(outer->index_expr->expr_type() == ExprType::INT_LITERAL);

  auto *inner = dynamic_cast<ArrayAccessExpr *>(outer->base_expr.get());

  REQUIRE(inner != nullptr);

  auto *base = dynamic_cast<IdentifierExpr *>(inner->base_expr.get());

  REQUIRE(base != nullptr);
  REQUIRE(base->identifier_name == "arr");
}

TEST_CASE("Parser parses array access on function call result") {
  auto result = parse("int main() { return foo()[5]; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *array = dynamic_cast<ArrayAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(array != nullptr);

  REQUIRE(array->base_expr->expr_type() == ExprType::FUNCTION_CALL);

  auto *call = dynamic_cast<FunctionCallExpr *>(array->base_expr.get());

  REQUIRE(call != nullptr);
  REQUIRE(call->function_name == "foo");
}

TEST_CASE("Parser parses function call as array index") {
  auto result = parse("int main() { return arr[foo()]; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *array = dynamic_cast<ArrayAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(array != nullptr);

  REQUIRE(array->index_expr->expr_type() == ExprType::FUNCTION_CALL);

  auto *call = dynamic_cast<FunctionCallExpr *>(array->index_expr.get());

  REQUIRE(call != nullptr);
  REQUIRE(call->function_name == "foo");
}

TEST_CASE("Parser parses parenthesized array base") {
  auto result = parse("int main() { return (arr)[0]; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *array = dynamic_cast<ArrayAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(array != nullptr);

  auto *base = dynamic_cast<IdentifierExpr *>(array->base_expr.get());

  REQUIRE(base != nullptr);
  REQUIRE(base->identifier_name == "arr");
}

TEST_CASE("Parser parses complex array index expression") {
  auto result = parse("int main() { return arr[a + b * c]; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *array = dynamic_cast<ArrayAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(array != nullptr);

  auto *plus = dynamic_cast<BinaryExpr *>(array->index_expr.get());

  REQUIRE(plus != nullptr);
  REQUIRE(plus->op == TokenType::PLUS);

  auto *multiply = dynamic_cast<BinaryExpr *>(plus->right_expr.get());

  REQUIRE(multiply != nullptr);
  REQUIRE(multiply->op == TokenType::MULTIPLY);
}

TEST_CASE("Parser parses array access inside binary expression") {
  auto result = parse("int main() { return arr[0] + arr[1]; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *binary = dynamic_cast<BinaryExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(binary != nullptr);

  REQUIRE(binary->left_expr->expr_type() == ExprType::ARRAY_ACCESS);
  REQUIRE(binary->right_expr->expr_type() == ExprType::ARRAY_ACCESS);
}

TEST_CASE("Parser parses member access expression") {
  auto result = parse("int main() { return obj.x; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::MEMBER_ACCESS);

  auto *member = dynamic_cast<MemberAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(member != nullptr);

  REQUIRE(member->op == TokenType::DOT);
  REQUIRE(member->member_name == "x");

  REQUIRE(member->base_expr->expr_type() == ExprType::IDENTIFIER);

  auto *base = dynamic_cast<IdentifierExpr *>(member->base_expr.get());

  REQUIRE(base != nullptr);
  REQUIRE(base->identifier_name == "obj");
}

TEST_CASE("Parser parses pointer member access expression") {
  auto result = parse("int main() { return ptr->x; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *member = dynamic_cast<MemberAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(member != nullptr);

  REQUIRE(member->op == TokenType::ARROW);
  REQUIRE(member->member_name == "x");

  auto *base = dynamic_cast<IdentifierExpr *>(member->base_expr.get());

  REQUIRE(base != nullptr);
  REQUIRE(base->identifier_name == "ptr");
}

TEST_CASE("Parser parses chained member access expression") {
  auto result = parse("int main() { return obj.a.b; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *outer = dynamic_cast<MemberAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(outer != nullptr);

  REQUIRE(outer->op == TokenType::DOT);
  REQUIRE(outer->member_name == "b");

  auto *inner = dynamic_cast<MemberAccessExpr *>(outer->base_expr.get());

  REQUIRE(inner != nullptr);

  REQUIRE(inner->op == TokenType::DOT);
  REQUIRE(inner->member_name == "a");

  auto *base = dynamic_cast<IdentifierExpr *>(inner->base_expr.get());

  REQUIRE(base != nullptr);
  REQUIRE(base->identifier_name == "obj");
}

TEST_CASE("Parser parses chained pointer member access expression") {
  auto result = parse("int main() { return ptr->a->b; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *outer = dynamic_cast<MemberAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(outer != nullptr);

  REQUIRE(outer->op == TokenType::ARROW);
  REQUIRE(outer->member_name == "b");

  auto *inner = dynamic_cast<MemberAccessExpr *>(outer->base_expr.get());

  REQUIRE(inner != nullptr);

  REQUIRE(inner->op == TokenType::ARROW);
  REQUIRE(inner->member_name == "a");

  auto *base = dynamic_cast<IdentifierExpr *>(inner->base_expr.get());

  REQUIRE(base != nullptr);
  REQUIRE(base->identifier_name == "ptr");
}

TEST_CASE("Parser parses member access on array element") {
  auto result = parse("int main() { return arr[5].x; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *member = dynamic_cast<MemberAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(member != nullptr);

  REQUIRE(member->member_name == "x");
  REQUIRE(member->op == TokenType::DOT);

  REQUIRE(member->base_expr->expr_type() == ExprType::ARRAY_ACCESS);

  auto *array = dynamic_cast<ArrayAccessExpr *>(member->base_expr.get());

  REQUIRE(array != nullptr);
}

TEST_CASE("Parser parses array access on member expression") {
  auto result = parse("int main() { return obj.arr[5]; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *array = dynamic_cast<ArrayAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(array != nullptr);

  REQUIRE(array->base_expr->expr_type() == ExprType::MEMBER_ACCESS);

  auto *member = dynamic_cast<MemberAccessExpr *>(array->base_expr.get());

  REQUIRE(member != nullptr);

  REQUIRE(member->member_name == "arr");
  REQUIRE(member->op == TokenType::DOT);
}

TEST_CASE("Parser parses member access on function call") {
  auto result = parse("int main() { return foo().x; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *member = dynamic_cast<MemberAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(member != nullptr);

  REQUIRE(member->member_name == "x");

  REQUIRE(member->base_expr->expr_type() == ExprType::FUNCTION_CALL);

  auto *call = dynamic_cast<FunctionCallExpr *>(member->base_expr.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->function_name == "foo");
}

TEST_CASE("Parser parses member access as function argument") {
  auto result = parse("int main() { return foo(obj.x); }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  auto *call = dynamic_cast<FunctionCallExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->arguments.size() == 1);

  REQUIRE(call->arguments[0]->expr_type() == ExprType::MEMBER_ACCESS);

  auto *member = dynamic_cast<MemberAccessExpr *>(call->arguments[0].get());

  REQUIRE(member != nullptr);

  REQUIRE(member->member_name == "x");
}

TEST_CASE("Parser parses chained postfix expression") {
  auto result = parse("int main() { return foo().a[5].b->c[10].d; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::MEMBER_ACCESS);

  auto *member_d = dynamic_cast<MemberAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(member_d != nullptr);
  REQUIRE(member_d->op == TokenType::DOT);
  REQUIRE(member_d->member_name == "d");

  // foo().a[5].b->c[10]
  REQUIRE(member_d->base_expr->expr_type() == ExprType::ARRAY_ACCESS);

  auto *array_10 = dynamic_cast<ArrayAccessExpr *>(member_d->base_expr.get());

  REQUIRE(array_10 != nullptr);

  // index = 10
  REQUIRE(array_10->index_expr->expr_type() == ExprType::INT_LITERAL);

  auto *index10 = dynamic_cast<IntLetExpr *>(array_10->index_expr.get());

  REQUIRE(index10 != nullptr);
  REQUIRE(index10->value == 10);

  // foo().a[5].b->c
  REQUIRE(array_10->base_expr->expr_type() == ExprType::MEMBER_ACCESS);

  auto *member_c = dynamic_cast<MemberAccessExpr *>(array_10->base_expr.get());

  REQUIRE(member_c != nullptr);
  REQUIRE(member_c->op == TokenType::ARROW);
  REQUIRE(member_c->member_name == "c");

  // foo().a[5].b
  REQUIRE(member_c->base_expr->expr_type() == ExprType::MEMBER_ACCESS);

  auto *member_b = dynamic_cast<MemberAccessExpr *>(member_c->base_expr.get());

  REQUIRE(member_b != nullptr);
  REQUIRE(member_b->op == TokenType::DOT);
  REQUIRE(member_b->member_name == "b");

  // foo().a[5]
  REQUIRE(member_b->base_expr->expr_type() == ExprType::ARRAY_ACCESS);

  auto *array_5 = dynamic_cast<ArrayAccessExpr *>(member_b->base_expr.get());

  REQUIRE(array_5 != nullptr);

  REQUIRE(array_5->index_expr->expr_type() == ExprType::INT_LITERAL);

  auto *index5 = dynamic_cast<IntLetExpr *>(array_5->index_expr.get());

  REQUIRE(index5 != nullptr);
  REQUIRE(index5->value == 5);

  // foo().a
  REQUIRE(array_5->base_expr->expr_type() == ExprType::MEMBER_ACCESS);

  auto *member_a = dynamic_cast<MemberAccessExpr *>(array_5->base_expr.get());

  REQUIRE(member_a != nullptr);
  REQUIRE(member_a->op == TokenType::DOT);
  REQUIRE(member_a->member_name == "a");

  // foo()
  REQUIRE(member_a->base_expr->expr_type() == ExprType::FUNCTION_CALL);

  auto *call = dynamic_cast<FunctionCallExpr *>(member_a->base_expr.get());

  REQUIRE(call != nullptr);
  REQUIRE(call->function_name == "foo");
  REQUIRE(call->arguments.empty());
}

TEST_CASE("Parser parses complex chained postfix expression") {
  auto result = parse("int main() { return foo(bar())[i].next->value.field; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::MEMBER_ACCESS);

  // .field
  auto *member_field = dynamic_cast<MemberAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(member_field != nullptr);
  REQUIRE(member_field->op == TokenType::DOT);
  REQUIRE(member_field->member_name == "field");

  // ->value
  REQUIRE(member_field->base_expr->expr_type() == ExprType::MEMBER_ACCESS);

  auto *member_value = dynamic_cast<MemberAccessExpr *>(member_field->base_expr.get());

  REQUIRE(member_value != nullptr);
  REQUIRE(member_value->op == TokenType::ARROW);
  REQUIRE(member_value->member_name == "value");

  // .next
  REQUIRE(member_value->base_expr->expr_type() == ExprType::MEMBER_ACCESS);

  auto *member_next = dynamic_cast<MemberAccessExpr *>(member_value->base_expr.get());

  REQUIRE(member_next != nullptr);
  REQUIRE(member_next->op == TokenType::DOT);
  REQUIRE(member_next->member_name == "next");

  // foo(bar())[i]
  REQUIRE(member_next->base_expr->expr_type() == ExprType::ARRAY_ACCESS);

  auto *array = dynamic_cast<ArrayAccessExpr *>(member_next->base_expr.get());

  REQUIRE(array != nullptr);

  // index = i
  REQUIRE(array->index_expr->expr_type() == ExprType::IDENTIFIER);

  auto *index = dynamic_cast<IdentifierExpr *>(array->index_expr.get());

  REQUIRE(index != nullptr);
  REQUIRE(index->identifier_name == "i");

  // foo(bar())
  REQUIRE(array->base_expr->expr_type() == ExprType::FUNCTION_CALL);

  auto *foo_call = dynamic_cast<FunctionCallExpr *>(array->base_expr.get());

  REQUIRE(foo_call != nullptr);

  REQUIRE(foo_call->function_name == "foo");
  REQUIRE(foo_call->arguments.size() == 1);

  // bar()
  REQUIRE(foo_call->arguments[0]->expr_type() == ExprType::FUNCTION_CALL);

  auto *bar_call = dynamic_cast<FunctionCallExpr *>(foo_call->arguments[0].get());

  REQUIRE(bar_call != nullptr);

  REQUIRE(bar_call->function_name == "bar");
  REQUIRE(bar_call->arguments.empty());
}

TEST_CASE("Parser parses cast to int") {
  auto result = parse("int main() { return (int)x; }");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::CAST);

  auto *cast = dynamic_cast<CastExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->target_type.datatype == DataType::INT);

  REQUIRE(cast->expr->expr_type() == ExprType::IDENTIFIER);

  auto *id = dynamic_cast<IdentifierExpr *>(cast->expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "x");
}

TEST_CASE("Parser parses cast to double") {
  auto result = parse("int main() { return (double)x; }");

  auto *cast = dynamic_cast<CastExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->target_type.datatype == DataType::DOUBLE);
}

TEST_CASE("Parser parses cast to float") {
  auto result = parse("int main() { return (float)x; }");

  auto *cast = dynamic_cast<CastExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->target_type.datatype == DataType::FLOAT);
}

TEST_CASE("Parser parses cast to char") {
  auto result = parse("int main() { return (char)x; }");

  auto *cast = dynamic_cast<CastExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->target_type.datatype == DataType::CHAR);
}

TEST_CASE("Parser parses pointer cast") {
  auto result = parse("int main() { return (int*)ptr; }");

  auto *cast = dynamic_cast<CastExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->target_type.datatype == DataType::INT);
  REQUIRE(cast->target_type.pointer_depth == 1);

  auto *id = dynamic_cast<IdentifierExpr *>(cast->expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "ptr");
}

TEST_CASE("Parser parses multi pointer cast") {
  auto result = parse("int main() { return (char***)ptr; }");

  auto *cast = dynamic_cast<CastExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->target_type.datatype == DataType::CHAR);
  REQUIRE(cast->target_type.pointer_depth == 3);
}

/*
// TODO: Add support for array type cast

TEST_CASE("Parser parses array type cast") {
    auto result = parse("int main() { return (int[10])x; }");

    auto *cast = dynamic_cast<CastExpr *>(get_return_stmt(result)->expr_ptr.get());

    REQUIRE(cast != nullptr);

    REQUIRE(cast->target_type.datatype == DataType::INT);
    REQUIRE(cast->target_type.dimensions == std::vector<size_t>{10});
}
*/

TEST_CASE("Parser parses cast of literal") {
  auto result = parse("int main() { return (double)10; }");

  auto *cast = dynamic_cast<CastExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->expr->expr_type() == ExprType::INT_LITERAL);

  auto *lit = dynamic_cast<IntLetExpr *>(cast->expr.get());

  REQUIRE(lit != nullptr);
  REQUIRE(lit->value == 10);
}

TEST_CASE("Parser parses cast of binary expression(double)") {
  auto result = parse("int main() { return (double)(a + b); }");

  auto *cast = dynamic_cast<CastExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->expr->expr_type() == ExprType::BINARY);

  auto *binary = dynamic_cast<BinaryExpr *>(cast->expr.get());

  REQUIRE(binary != nullptr);
  REQUIRE(binary->op == TokenType::PLUS);
}

TEST_CASE("Parser parses nested cast expressions") {
  auto result = parse("int main() { return (int)(double)x; }");

  auto *outer = dynamic_cast<CastExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(outer != nullptr);

  REQUIRE(outer->target_type.datatype == DataType::INT);

  REQUIRE(outer->expr->expr_type() == ExprType::CAST);

  auto *inner = dynamic_cast<CastExpr *>(outer->expr.get());

  REQUIRE(inner != nullptr);

  REQUIRE(inner->target_type.datatype == DataType::DOUBLE);
}

TEST_CASE("Parser parses cast expression as function argument") {
  auto result = parse("int main() { return foo((int)x); }");

  auto *call = dynamic_cast<FunctionCallExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->arguments.size() == 1);

  REQUIRE(call->arguments[0]->expr_type() == ExprType::CAST);
}

TEST_CASE("Parser parses member access after cast") {
  auto result = parse(R"(
    struct Foo {
        int x;
    };

    int main() {
        return ((struct Foo*)ptr)->x;
    }
)");

  ReturnStmt *return_stmt = get_return_stmt(result);

  REQUIRE(return_stmt->expr_ptr->expr_type() == ExprType::MEMBER_ACCESS);

  auto *member = dynamic_cast<MemberAccessExpr *>(return_stmt->expr_ptr.get());

  REQUIRE(member != nullptr);
  REQUIRE(member->op == TokenType::ARROW);
  REQUIRE(member->member_name == "x");

  REQUIRE(member->base_expr->expr_type() == ExprType::CAST);

  auto *cast = dynamic_cast<CastExpr *>(member->base_expr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->target_type.datatype == DataType::STRUCT);
  REQUIRE(cast->target_type.custom_name == "Foo");
  REQUIRE(cast->target_type.pointer_depth == 1);

  REQUIRE(cast->expr->expr_type() == ExprType::IDENTIFIER);

  auto *id = dynamic_cast<IdentifierExpr *>(cast->expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "ptr");
}

/*
// TODO: add support for casting to array types

TEST_CASE("Parser parses array access after cast") {
   auto result = parse("int main() { return ((int*)ptr)[5]; }");
   auto *array = dynamic_cast<ArrayAccessExpr *>(get_return_stmt(result)->expr_ptr.get());
   REQUIRE(array != nullptr);
   REQUIRE(array->base_expr->expr_type() == ExprType::CAST);
   auto *cast = dynamic_cast<CastExpr *>(array->base_expr.get());
   REQUIRE(cast != nullptr);
   REQUIRE(cast->target_type.pointer_depth == 1);
 }
*/

TEST_CASE("Parser parses simple assignment") {
  auto result = parse("int main() { return a = b; }");

  auto *assign = dynamic_cast<AssignmentExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(assign != nullptr);

  auto *lhs = dynamic_cast<IdentifierExpr *>(assign->lhs.get());
  auto *rhs = dynamic_cast<IdentifierExpr *>(assign->rhs.get());

  REQUIRE(lhs != nullptr);
  REQUIRE(rhs != nullptr);

  REQUIRE(lhs->identifier_name == "a");
  REQUIRE(rhs->identifier_name == "b");
}

TEST_CASE("Parser parses assignment with literal rhs") {
  auto result = parse("int main() { return a = 10; }");

  auto *assign = dynamic_cast<AssignmentExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(assign != nullptr);

  REQUIRE(assign->lhs->expr_type() == ExprType::IDENTIFIER);
  REQUIRE(assign->rhs->expr_type() == ExprType::INT_LITERAL);

  auto *rhs = dynamic_cast<IntLetExpr *>(assign->rhs.get());

  REQUIRE(rhs != nullptr);
  REQUIRE(rhs->value == 10);
}

TEST_CASE("Parser parses assignment with binary rhs") {
  auto result = parse("int main() { return a = b + c; }");

  auto *assign = dynamic_cast<AssignmentExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(assign != nullptr);

  REQUIRE(assign->rhs->expr_type() == ExprType::BINARY);

  auto *binary = dynamic_cast<BinaryExpr *>(assign->rhs.get());

  REQUIRE(binary != nullptr);
  REQUIRE(binary->op == TokenType::PLUS);
}

TEST_CASE("Parser parses chained assignments") {
  auto result = parse("int main() { return a = b = c; }");

  auto *outer = dynamic_cast<AssignmentExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(outer != nullptr);

  auto *lhs = dynamic_cast<IdentifierExpr *>(outer->lhs.get());
  auto *inner = dynamic_cast<AssignmentExpr *>(outer->rhs.get());

  REQUIRE(lhs != nullptr);
  REQUIRE(inner != nullptr);

  REQUIRE(lhs->identifier_name == "a");

  auto *inner_lhs = dynamic_cast<IdentifierExpr *>(inner->lhs.get());
  auto *inner_rhs = dynamic_cast<IdentifierExpr *>(inner->rhs.get());

  REQUIRE(inner_lhs != nullptr);
  REQUIRE(inner_rhs != nullptr);

  REQUIRE(inner_lhs->identifier_name == "b");
  REQUIRE(inner_rhs->identifier_name == "c");
}

TEST_CASE("Parser parses assignment precedence") {
  auto result = parse("int main() { return a = b + c * d; }");

  auto *assign = dynamic_cast<AssignmentExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(assign != nullptr);

  auto *plus = dynamic_cast<BinaryExpr *>(assign->rhs.get());

  REQUIRE(plus != nullptr);
  REQUIRE(plus->op == TokenType::PLUS);

  auto *mul = dynamic_cast<BinaryExpr *>(plus->right_expr.get());

  REQUIRE(mul != nullptr);
  REQUIRE(mul->op == TokenType::MULTIPLY);
}

TEST_CASE("Parser parses assignment to array element") {
  auto result = parse("int main() { return arr[i] = value; }");

  auto *assign = dynamic_cast<AssignmentExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(assign != nullptr);
  REQUIRE(assign->lhs->expr_type() == ExprType::ARRAY_ACCESS);
}

TEST_CASE("Parser parses assignment to member") {
  auto result = parse("int main() { return obj.field = value; }");

  auto *assign = dynamic_cast<AssignmentExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(assign != nullptr);
  REQUIRE(assign->lhs->expr_type() == ExprType::MEMBER_ACCESS);
}

TEST_CASE("Parser parses assignment through pointer member") {
  auto result = parse("int main() { return ptr->field = value; }");

  auto *assign = dynamic_cast<AssignmentExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(assign != nullptr);

  auto *member = dynamic_cast<MemberAccessExpr *>(assign->lhs.get());

  REQUIRE(member != nullptr);
  REQUIRE(member->op == TokenType::ARROW);
}

TEST_CASE("Parser parses assignment through dereference") {
  auto result = parse("int main() { return *ptr = value; }");

  auto *assign = dynamic_cast<AssignmentExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(assign != nullptr);

  auto *unary = dynamic_cast<UnaryExpr *>(assign->lhs.get());

  REQUIRE(unary != nullptr);
  REQUIRE(unary->op == TokenType::MULTIPLY);
}

TEST_CASE("Parser parses parenthesized assignment") {
  auto result = parse("int main() { return (a = b); }");

  auto *assign = dynamic_cast<AssignmentExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(assign != nullptr);

  auto *lhs = dynamic_cast<IdentifierExpr *>(assign->lhs.get());
  auto *rhs = dynamic_cast<IdentifierExpr *>(assign->rhs.get());

  REQUIRE(lhs != nullptr);
  REQUIRE(rhs != nullptr);

  REQUIRE(lhs->identifier_name == "a");
  REQUIRE(rhs->identifier_name == "b");
}

TEST_CASE("Parser desugars compound assignments") {
  auto [expr, binary_op] = GENERATE(
      std::pair{"a += b", TokenType::PLUS},
      std::pair{"a -= b", TokenType::MINUS},
      std::pair{"a *= b", TokenType::MULTIPLY},
      std::pair{"a /= b", TokenType::DIVIDE},
      std::pair{"a %= b", TokenType::MODULO},
      std::pair{"a <<= b", TokenType::LEFT_SHIFT},
      std::pair{"a >>= b", TokenType::RIGHT_SHIFT},
      std::pair{"a &= b", TokenType::AMPERSAND},
      std::pair{"a |= b", TokenType::PIPE},
      std::pair{"a ^= b", TokenType::CARET});

  DYNAMIC_SECTION(expr) {
    std::string source = "int main() { return ";
    source += expr;
    source += "; }";

    auto result = parse(source);

    auto *assign = dynamic_cast<AssignmentExpr *>(get_return_stmt(result)->expr_ptr.get());

    REQUIRE(assign != nullptr);

    auto *lhs = dynamic_cast<IdentifierExpr *>(assign->lhs.get());
    REQUIRE(lhs != nullptr);
    REQUIRE(lhs->identifier_name == "a");

    auto *binary = dynamic_cast<BinaryExpr *>(assign->rhs.get());

    REQUIRE(binary != nullptr);
    REQUIRE(binary->op == binary_op);

    auto *binary_lhs = dynamic_cast<IdentifierExpr *>(binary->left_expr.get());
    auto *binary_rhs = dynamic_cast<IdentifierExpr *>(binary->right_expr.get());

    REQUIRE(binary_lhs != nullptr);
    REQUIRE(binary_rhs != nullptr);

    REQUIRE(binary_lhs->identifier_name == "a");
    REQUIRE(binary_rhs->identifier_name == "b");
  }
}

TEST_CASE("Parser parses basic conditional expression") {
  auto result = parse("int main() { return a ? b : c; }");

  auto *cond = dynamic_cast<ConditionalExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cond != nullptr);

  auto *condition = dynamic_cast<IdentifierExpr *>(cond->condition.get());
  auto *true_expr = dynamic_cast<IdentifierExpr *>(cond->true_expr.get());
  auto *false_expr = dynamic_cast<IdentifierExpr *>(cond->false_expr.get());

  REQUIRE(condition != nullptr);
  REQUIRE(true_expr != nullptr);
  REQUIRE(false_expr != nullptr);

  REQUIRE(condition->identifier_name == "a");
  REQUIRE(true_expr->identifier_name == "b");
  REQUIRE(false_expr->identifier_name == "c");
}

TEST_CASE("Parser parses conditional with literal branches") {
  auto result = parse("int main() { return a ? 10 : 20; }");

  auto *cond = dynamic_cast<ConditionalExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cond != nullptr);

  REQUIRE(cond->condition->expr_type() == ExprType::IDENTIFIER);
  REQUIRE(cond->true_expr->expr_type() == ExprType::INT_LITERAL);
  REQUIRE(cond->false_expr->expr_type() == ExprType::INT_LITERAL);
}

TEST_CASE("Parser parses binary condition in conditional") {
  auto result = parse("int main() { return a + b ? c : d; }");

  auto *cond = dynamic_cast<ConditionalExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cond != nullptr);

  auto *binary = dynamic_cast<BinaryExpr *>(cond->condition.get());

  REQUIRE(binary != nullptr);
  REQUIRE(binary->op == TokenType::PLUS);
}

TEST_CASE("Parser parses binary true branch") {
  auto result = parse("int main() { return a ? b + c : d; }");

  auto *cond = dynamic_cast<ConditionalExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cond != nullptr);

  auto *binary = dynamic_cast<BinaryExpr *>(cond->true_expr.get());

  REQUIRE(binary != nullptr);
  REQUIRE(binary->op == TokenType::PLUS);
}

TEST_CASE("Parser parses binary false branch") {
  auto result = parse("int main() { return a ? b : c + d; }");

  auto *cond = dynamic_cast<ConditionalExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cond != nullptr);

  auto *binary = dynamic_cast<BinaryExpr *>(cond->false_expr.get());

  REQUIRE(binary != nullptr);
  REQUIRE(binary->op == TokenType::PLUS);
}

TEST_CASE("Parser parses nested conditional expressions") {
  auto result = parse("int main() { return a ? b : c ? d : e; }");

  auto *outer = dynamic_cast<ConditionalExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(outer != nullptr);

  auto *inner = dynamic_cast<ConditionalExpr *>(outer->false_expr.get());

  REQUIRE(inner != nullptr);

  auto *cond = dynamic_cast<IdentifierExpr *>(inner->condition.get());

  REQUIRE(cond != nullptr);
  REQUIRE(cond->identifier_name == "c");
}

TEST_CASE("Parser parses parenthesized conditional") {
  auto result = parse("int main() { return (a ? b : c); }");

  REQUIRE(get_return_stmt(result)->expr_ptr->expr_type() == ExprType::CONDITIONAL);
}

TEST_CASE("Parser parses conditional precedence") {
  auto result = parse("int main() { return a + b ? c : d + e; }");

  auto *cond = dynamic_cast<ConditionalExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cond != nullptr);

  REQUIRE(cond->condition->expr_type() == ExprType::BINARY);
  REQUIRE(cond->false_expr->expr_type() == ExprType::BINARY);
}

TEST_CASE("Parser parses conditional inside binary expression") {
  auto result = parse("int main() { return (a ? b : c) + d; }");

  auto *binary = dynamic_cast<BinaryExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(binary != nullptr);

  REQUIRE(binary->left_expr->expr_type() == ExprType::CONDITIONAL);
}

TEST_CASE("Parser parses function calls in conditional") {
  auto result = parse("int main() { return a ? foo() : bar(); }");

  auto *cond = dynamic_cast<ConditionalExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cond != nullptr);

  REQUIRE(cond->true_expr->expr_type() == ExprType::FUNCTION_CALL);
  REQUIRE(cond->false_expr->expr_type() == ExprType::FUNCTION_CALL);
}

TEST_CASE("Parser parses member access in conditional") {
  auto result = parse("int main() { return a ? obj.x : ptr->x; }");

  auto *cond = dynamic_cast<ConditionalExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cond != nullptr);

  REQUIRE(cond->true_expr->expr_type() == ExprType::MEMBER_ACCESS);
  REQUIRE(cond->false_expr->expr_type() == ExprType::MEMBER_ACCESS);
}

TEST_CASE("Parser parses array access in conditional") {
  auto result = parse("int main() { return a ? arr[i] : arr[j]; }");

  auto *cond = dynamic_cast<ConditionalExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cond != nullptr);

  REQUIRE(cond->true_expr->expr_type() == ExprType::ARRAY_ACCESS);
  REQUIRE(cond->false_expr->expr_type() == ExprType::ARRAY_ACCESS);
}

TEST_CASE("Parser parses sizeof identifier") {
  auto result = parse("int main() { return sizeof x; }");

  auto *sizeof_expr = dynamic_cast<SizeofExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(sizeof_expr != nullptr);

  REQUIRE_FALSE(sizeof_expr->parsed_type.has_value());
  REQUIRE(sizeof_expr->expr != nullptr);
  REQUIRE(sizeof_expr->expr->expr_type() == ExprType::IDENTIFIER);

  auto *id = dynamic_cast<IdentifierExpr *>(sizeof_expr->expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "x");
}

TEST_CASE("Parser parses sizeof parenthesized identifier") {
  auto result = parse("int main() { return sizeof(x); }");

  auto *sizeof_expr = dynamic_cast<SizeofExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(sizeof_expr != nullptr);

  REQUIRE_FALSE(sizeof_expr->parsed_type.has_value());
  REQUIRE(sizeof_expr->expr->expr_type() == ExprType::IDENTIFIER);
}

TEST_CASE("Parser parses sizeof binary expression") {
  auto result = parse("int main() { return sizeof(a + b); }");

  auto *sizeof_expr = dynamic_cast<SizeofExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(sizeof_expr != nullptr);

  REQUIRE_FALSE(sizeof_expr->parsed_type.has_value());

  auto *binary = dynamic_cast<BinaryExpr *>(sizeof_expr->expr.get());

  REQUIRE(binary != nullptr);
  REQUIRE(binary->op == TokenType::PLUS);
}

TEST_CASE("Parser parses sizeof function call") {
  auto result = parse("int main() { return sizeof(foo()); }");

  auto *sizeof_expr = dynamic_cast<SizeofExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(sizeof_expr != nullptr);

  REQUIRE(sizeof_expr->expr->expr_type() == ExprType::FUNCTION_CALL);
}

TEST_CASE("Parser parses sizeof array access") {
  auto result = parse("int main() { return sizeof(arr[i]); }");

  auto *sizeof_expr = dynamic_cast<SizeofExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(sizeof_expr != nullptr);

  REQUIRE(sizeof_expr->expr->expr_type() == ExprType::ARRAY_ACCESS);
}

TEST_CASE("Parser parses sizeof member access") {
  auto result = parse("int main() { return sizeof(ptr->next); }");

  auto *sizeof_expr = dynamic_cast<SizeofExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(sizeof_expr != nullptr);

  REQUIRE(sizeof_expr->expr->expr_type() == ExprType::MEMBER_ACCESS);
}

TEST_CASE("Parser parses sizeof cast expression") {
  auto result = parse("int main() { return sizeof((int)x); }");

  auto *sizeof_expr = dynamic_cast<SizeofExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(sizeof_expr != nullptr);

  REQUIRE(sizeof_expr->expr->expr_type() == ExprType::CAST);
}

TEST_CASE("Parser parses sizeof primitive types") {
  auto [source, datatype] = GENERATE(
      std::pair{"sizeof(int)", DataType::INT},
      std::pair{"sizeof(char)", DataType::CHAR},
      std::pair{"sizeof(float)", DataType::FLOAT},
      std::pair{"sizeof(double)", DataType::DOUBLE},
      std::pair{"sizeof(void)", DataType::VOID});

  auto result = parse(std::string("int main() { return ") + source + "; }");

  auto *sizeof_expr = dynamic_cast<SizeofExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(sizeof_expr != nullptr);

  REQUIRE(sizeof_expr->parsed_type.has_value());
  REQUIRE(sizeof_expr->expr == nullptr);

  REQUIRE(sizeof_expr->parsed_type->datatype == datatype);
}

TEST_CASE("Parser parses sizeof pointer types") {
  auto result = parse("int main() { return sizeof(int***); }");

  auto *sizeof_expr = dynamic_cast<SizeofExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(sizeof_expr != nullptr);

  REQUIRE(sizeof_expr->parsed_type.has_value());

  REQUIRE(sizeof_expr->parsed_type->datatype == DataType::INT);
  REQUIRE(sizeof_expr->parsed_type->pointer_depth == 3);
}

/*
TODO: Add support for sizeof(array[][]) support
TEST_CASE("Parser parses sizeof array types") {
  auto result = parse("int main() { return sizeof(int[10][20]); }");

  auto *sizeof_expr = dynamic_cast<SizeofExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(sizeof_expr != nullptr);

  REQUIRE(sizeof_expr->parsed_type.has_value());

  REQUIRE(sizeof_expr->parsed_type->datatype == DataType::INT);
  REQUIRE(sizeof_expr->parsed_type->dimensions == std::vector<size_t>{10, 20});
}
*/

TEST_CASE("Parser parses sizeof struct type") {
  auto result = parse(R"(
    struct Foo { int x; };

    int main() {
      return sizeof(struct Foo);
    }
  )");

  auto *sizeof_expr = dynamic_cast<SizeofExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(sizeof_expr != nullptr);

  REQUIRE(sizeof_expr->parsed_type.has_value());

  REQUIRE(sizeof_expr->parsed_type->datatype == DataType::STRUCT);
  REQUIRE(sizeof_expr->parsed_type->custom_name == "Foo");
}

TEST_CASE("Parser parses sizeof union type") {
  auto result = parse(R"(
    union Foo { int x; };

    int main() {
      return sizeof(union Foo);
    }
  )");

  auto *sizeof_expr = dynamic_cast<SizeofExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(sizeof_expr != nullptr);

  REQUIRE(sizeof_expr->parsed_type.has_value());

  REQUIRE(sizeof_expr->parsed_type->datatype == DataType::UNION);
  REQUIRE(sizeof_expr->parsed_type->custom_name == "Foo");
}

TEST_CASE("Parser parses sizeof enum type") {
  auto result = parse(R"(
    enum Color { RED };

    int main() {
      return sizeof(enum Color);
    }
  )");

  auto *sizeof_expr = dynamic_cast<SizeofExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(sizeof_expr != nullptr);

  REQUIRE(sizeof_expr->parsed_type.has_value());

  REQUIRE(sizeof_expr->parsed_type->datatype == DataType::ENUM);
  REQUIRE(sizeof_expr->parsed_type->custom_name == "Color");
}

TEST_CASE("Parser parses dereference of pointer member access") {
  auto result = parse("int main() { return *ptr->next; }");

  auto *unary = dynamic_cast<UnaryExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(unary != nullptr);
  REQUIRE(unary->op == TokenType::MULTIPLY);

  REQUIRE(unary->right_expr->expr_type() == ExprType::MEMBER_ACCESS);

  auto *member = dynamic_cast<MemberAccessExpr *>(unary->right_expr.get());

  REQUIRE(member != nullptr);
  REQUIRE(member->op == TokenType::ARROW);
  REQUIRE(member->member_name == "next");
}

TEST_CASE("Parser parses address of member access") {
  auto result = parse("int main() { return &obj.field; }");

  auto *unary = dynamic_cast<UnaryExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(unary != nullptr);
  REQUIRE(unary->op == TokenType::AMPERSAND);

  REQUIRE(unary->right_expr->expr_type() == ExprType::MEMBER_ACCESS);
}

TEST_CASE("Parser parses dereference of array element") {
  auto result = parse("int main() { return *arr[i]; }");

  auto *unary = dynamic_cast<UnaryExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(unary != nullptr);
  REQUIRE(unary->op == TokenType::MULTIPLY);

  REQUIRE(unary->right_expr->expr_type() == ExprType::ARRAY_ACCESS);
}

TEST_CASE("Parser parses address of array element") {
  auto result = parse("int main() { return &arr[i]; }");

  auto *unary = dynamic_cast<UnaryExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(unary != nullptr);
  REQUIRE(unary->op == TokenType::AMPERSAND);

  REQUIRE(unary->right_expr->expr_type() == ExprType::ARRAY_ACCESS);
}

TEST_CASE("Parser parses cast before binary operator") {
  auto result = parse("int main() { return (int)x + y; }");

  auto *binary = dynamic_cast<BinaryExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(binary != nullptr);
  REQUIRE(binary->op == TokenType::PLUS);

  REQUIRE(binary->left_expr->expr_type() == ExprType::CAST);
  REQUIRE(binary->right_expr->expr_type() == ExprType::IDENTIFIER);
}

TEST_CASE("Parser parses cast of binary expression (int)") {
  auto result = parse("int main() { return (int)(x + y); }");

  auto *cast = dynamic_cast<CastExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->expr->expr_type() == ExprType::BINARY);
}

TEST_CASE("Parser parses assignment with conditional rhs") {
  auto result = parse("int main() { return a = b ? c : d; }");

  auto *assign = dynamic_cast<AssignmentExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(assign != nullptr);

  REQUIRE(assign->rhs->expr_type() == ExprType::CONDITIONAL);
}

TEST_CASE("Parser parses nested conditional in true branch") {
  auto result = parse("int main() { return a ? b ? c : d : e; }");

  auto *outer = dynamic_cast<ConditionalExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(outer != nullptr);

  REQUIRE(outer->true_expr->expr_type() == ExprType::CONDITIONAL);

  auto *inner = dynamic_cast<ConditionalExpr *>(outer->true_expr.get());

  REQUIRE(inner != nullptr);

  auto *cond = dynamic_cast<IdentifierExpr *>(inner->condition.get());

  REQUIRE(cond != nullptr);
  REQUIRE(cond->identifier_name == "b");
}

TEST_CASE("Parser parses sizeof precedence") {
  auto result = parse("int main() { return sizeof x + 1; }");

  auto *binary = dynamic_cast<BinaryExpr *>(get_return_stmt(result)->expr_ptr.get());

  REQUIRE(binary != nullptr);
  REQUIRE(binary->op == TokenType::PLUS);

  REQUIRE(binary->left_expr->expr_type() == ExprType::SIZEOF);
  REQUIRE(binary->right_expr->expr_type() == ExprType::INT_LITERAL);
}