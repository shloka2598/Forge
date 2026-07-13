#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

#include "types/Types.h"

TEST_CASE("Semantic assigns correct type to integer addition") {
  auto result = analyze(R"(
int main() {
    return 10 + 20;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::BINARY);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::PLUS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to integer subtraction") {
  auto result = analyze(R"(
int main() {
    return 20 - 10;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::MINUS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to integer multiplication") {
  auto result = analyze(R"(
int main() {
    return 6 * 7;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::MULTIPLY);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to integer division") {
  auto result = analyze(R"(
int main() {
    return 20 / 5;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::DIVIDE);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to integer modulo") {
  auto result = analyze(R"(
int main() {
    return 20 % 3;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::MODULO);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns float type to float addition") {
  auto result = analyze(R"(
float main() {
    return 1.5f + 2.5f;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::BINARY);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::PLUS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns double type to double subtraction") {
  auto result = analyze(R"(
double main() {
    return 5.0 - 2.0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::MINUS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns float type to float multiplication") {
  auto result = analyze(R"(
float main() {
    return 3.0f * 4.0f;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::MULTIPLY);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns double type to double division") {
  auto result = analyze(R"(
double main() {
    return 10.0 / 4.0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::DIVIDE);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic promotes int and float to float") {
  auto result = analyze(R"(
float main() {
    return 10 + 2.5f;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::BINARY);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::PLUS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic promotes float and int to float") {
  auto result = analyze(R"(
float main() {
    return 2.5f + 10;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
}

TEST_CASE("Semantic promotes int and double to double") {
  auto result = analyze(R"(
double main() {
    return 10 + 2.5;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);
}

TEST_CASE("Semantic promotes double and int to double") {
  auto result = analyze(R"(
double main() {
    return 2.5 + 10;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);
}

TEST_CASE("Semantic promotes float and double to double") {
  auto result = analyze(R"(
double main() {
    return 2.5f + 4.0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);
}

TEST_CASE("Semantic promotes double and float to double") {
  auto result = analyze(R"(
double main() {
    return 4.0 + 2.5f;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);
}

TEST_CASE("Semantic promotes char and int to int") {
  auto result = analyze(R"(
int main() {
    return 'A' + 10;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic promotes char and float to float") {
  auto result = analyze(R"(
float main() {
    return 'A' + 2.5f;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
}

TEST_CASE("Semantic assigns int type to less-than expression") {
  auto result = analyze(R"(
int main() {
    return 10 < 20;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::BINARY);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::SMALLER_THAN);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns int type to less-than-or-equal expression") {
  auto result = analyze(R"(
int main() {
    return 10 <= 20;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  REQUIRE(expr->op == TokenType::SMALLER_THAN_EQUAL_THAN);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns int type to greater-than expression") {
  auto result = analyze(R"(
int main() {
    return 20 > 10;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  REQUIRE(expr->op == TokenType::GREATER_THAN);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns int type to greater-than-or-equal expression") {
  auto result = analyze(R"(
int main() {
    return 20 >= 10;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  REQUIRE(expr->op == TokenType::GREATER_THAN_EQUAL_THAN);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns int type to equality expression") {
  auto result = analyze(R"(
int main() {
    return 10 == 10;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  REQUIRE(expr->op == TokenType::DOUBLE_EQUALS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns int type to inequality expression") {
  auto result = analyze(R"(
int main() {
    return 10 != 20;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  REQUIRE(expr->op == TokenType::NOT_EQUALS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic promotes int and float before less-than comparison") {
  auto result = analyze(R"(
int main() {
    return 10 < 2.5f;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  REQUIRE(expr->op == TokenType::SMALLER_THAN);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic promotes float and int before greater-than comparison") {
  auto result = analyze(R"(
int main() {
    return 2.5f > 10;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  REQUIRE(expr->op == TokenType::GREATER_THAN);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic promotes int and double before equality comparison") {
  auto result = analyze(R"(
int main() {
    return 10 == 10.0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  REQUIRE(expr->op == TokenType::DOUBLE_EQUALS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic promotes float and double before inequality comparison") {
  auto result = analyze(R"(
int main() {
    return 2.5f != 2.5;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  REQUIRE(expr->op == TokenType::NOT_EQUALS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic promotes char and int before equality comparison") {
  auto result = analyze(R"(
int main() {
    return 'A' == 65;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  REQUIRE(expr->op == TokenType::DOUBLE_EQUALS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic promotes char and float before comparison") {
  auto result = analyze(R"(
int main() {
    return 'A' < 70.0f;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  REQUIRE(expr->op == TokenType::SMALLER_THAN);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic assigns int type to logical AND") {
  auto result = analyze(R"(
int main() {
    return 1 && 2;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::BINARY);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::DOUBLE_AMPERSAND);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns int type to logical OR") {
  auto result = analyze(R"(
int main() {
    return 1 || 2;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  REQUIRE(expr->op == TokenType::DOUBLE_PIPE);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic allows float operands in logical AND") {
  auto result = analyze(R"(
int main() {
    return 1.0f && 2.0f;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic allows double operands in logical OR") {
  auto result = analyze(R"(
int main() {
    return 1.0 || 2.0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic allows pointer operands in logical AND") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    int *p = &x;
    return p && p;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  auto *expr = get_expr<BinaryExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic allows pointer operands in logical OR") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    int *p = &x;
    return p || p;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  auto *expr = get_expr<BinaryExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic assigns int type to bitwise AND") {
  auto result = analyze(R"(
int main() {
    return 6 & 3;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::BINARY);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::AMPERSAND);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns int type to bitwise OR") {
  auto result = analyze(R"(
int main() {
    return 6 | 3;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  REQUIRE(expr->op == TokenType::PIPE);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns int type to bitwise XOR") {
  auto result = analyze(R"(
int main() {
    return 6 ^ 3;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  REQUIRE(expr->op == TokenType::CARET);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic promotes char operands in bitwise AND") {
  auto result = analyze(R"(
int main() {
    return 'A' & 'B';
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic promotes char and int in bitwise OR") {
  auto result = analyze(R"(
int main() {
    return 'A' | 1;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic promotes char and int in bitwise XOR") {
  auto result = analyze(R"(
int main() {
    return 'A' ^ 1;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic assigns int type to left shift") {
  auto result = analyze(R"(
int main() {
    return 1 << 4;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::BINARY);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::LEFT_SHIFT);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns int type to right shift") {
  auto result = analyze(R"(
int main() {
    return 16 >> 2;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  REQUIRE(expr->op == TokenType::RIGHT_SHIFT);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic promotes char operand in left shift") {
  auto result = analyze(R"(
int main() {
    return 'A' << 2;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic promotes char operand in right shift") {
  auto result = analyze(R"(
int main() {
    return 'A' >> 1;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<BinaryExpr>(get_return_stmt(result));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic allows variable left shift") {
  auto result = analyze(R"(
int main() {
    int x = 10;
    return x << 3;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::LEFT_SHIFT);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic allows variable right shift") {
  auto result = analyze(R"(
int main() {
    int x = 10;
    return x >> 3;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::RIGHT_SHIFT);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic assigns pointer type to pointer plus integer") {
  auto result = analyze(R"(
int *main() {
    int arr[10];
    int *p = arr;
    return p + 2;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::BINARY);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::PLUS);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns pointer type to integer plus pointer") {
  auto result = analyze(R"(
int *main() {
    int arr[10];
    int *p = arr;
    return 2 + p;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::PLUS);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns pointer type to pointer minus integer") {
  auto result = analyze(R"(
int *main() {
    int arr[10];
    int *p = arr + 5;
    return p - 2;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::MINUS);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns integer type to pointer subtraction") {
  auto result = analyze(R"(
int main() {
    int arr[10];
    int *a = arr;
    int *b = arr + 5;
    return b - a;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 3);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::MINUS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::LONG);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic decays array in pointer arithmetic") {
  auto result = analyze(R"(
int *main() {
    int arr[10];
    return arr + 3;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::PLUS);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}