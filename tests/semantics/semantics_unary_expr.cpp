#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

#include "types/Types.h"

TEST_CASE("Semantic assigns correct type to unary plus") {
  auto result = analyze(R"(
int main() {
    return +42;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::UNARY);

  auto *expr = get_expr<UnaryExpr>(ret);

  REQUIRE(expr->op == TokenType::PLUS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to unary minus") {
  auto result = analyze(R"(
int main() {
    return -42;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::UNARY);

  auto *expr = get_expr<UnaryExpr>(ret);

  REQUIRE(expr->op == TokenType::MINUS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to logical not") {
  auto result = analyze(R"(
int main() {
    return !42;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::UNARY);

  auto *expr = get_expr<UnaryExpr>(ret);

  REQUIRE(expr->op == TokenType::EXCLAMATION);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to bitwise not") {
  auto result = analyze(R"(
int main() {
    return ~42;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::UNARY);

  auto *expr = get_expr<UnaryExpr>(ret);

  REQUIRE(expr->op == TokenType::TILDE);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to address-of expression") {
  auto result = analyze(R"(
int *main() {
    int x = 42;
    return &x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::UNARY);

  auto *expr = get_expr<UnaryExpr>(ret);

  REQUIRE(expr->op == TokenType::AMPERSAND);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to dereference expression") {
  auto result = analyze(R"(
int main() {
    int x = 42;
    int *ptr = &x;
    return *ptr;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::UNARY);

  auto *expr = get_expr<UnaryExpr>(ret);

  REQUIRE(expr->op == TokenType::MULTIPLY);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic assigns correct type to prefix increment") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    return ++x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::PRE_INCREMENT);

  auto *expr = get_expr<IncrementExpr>(ret);

  REQUIRE(expr->is_prefix);
  REQUIRE(expr->is_increment);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to prefix decrement") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    return --x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::PRE_DECREMENT);

  auto *expr = get_expr<IncrementExpr>(ret);

  REQUIRE(expr->is_prefix);
  REQUIRE_FALSE(expr->is_increment);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to postfix increment") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    return x++;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::POST_INCREMENT);

  auto *expr = get_expr<IncrementExpr>(ret);

  REQUIRE(expr->is_increment);
  REQUIRE_FALSE(expr->is_prefix);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to postfix decrement") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    return x--;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::POST_DECREMENT);

  auto *expr = get_expr<IncrementExpr>(ret);

  REQUIRE_FALSE(expr->is_increment);
  REQUIRE_FALSE(expr->is_prefix);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns float type to unary plus") {
  auto result = analyze(R"(
float main() {
    float x = 3.5f;
    return +x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<UnaryExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns double type to unary minus") {
  auto result = analyze(R"(
double main() {
    double x = 3.14;
    return -x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<UnaryExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns pointer type to address-of expression") {
  auto result = analyze(R"(
int *main() {
    int x = 5;
    return &x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<UnaryExpr>(ret);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic dereference preserves pointee type") {
  auto result = analyze(R"(
float main() {
    float x = 1.5f;
    float *p = &x;
    return *p;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  auto *expr = get_expr<UnaryExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic double pointer dereference preserves pointee type") {
  auto result = analyze(R"(
int *main() {
    int x = 5;
    int *p = &x;
    int **pp = &p;
    return *pp;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 3);

  auto *expr = get_expr<UnaryExpr>(ret);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic assigns pointer type to address of array") {
  auto result = analyze(R"(
int main() {
    int arr[5];
    &arr;
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 3);

  auto *expr_stmt = get_stmt<ExpressionStmt>(fn, 1);

  REQUIRE(expr_stmt->expr->expr_type() == ExprType::UNARY);

  auto *expr = get_expr<UnaryExpr>(expr_stmt);

  auto *type = get_type<PointerType>(expr);

  auto *array = dynamic_cast<ArrayType *>(type->pointee_type);

  REQUIRE(array != nullptr);
  REQUIRE(array->size == 5);

  auto *base = dynamic_cast<BuiltinType *>(array->element_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns pointer to pointer type after address-of") {
  auto result = analyze(R"(
int **main() {
    int *ptr;
    return &ptr;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<UnaryExpr>(ret);

  auto *type = get_type<PointerType>(expr);

  auto *inner = dynamic_cast<PointerType *>(type->pointee_type);

  REQUIRE(inner != nullptr);

  auto *base = dynamic_cast<BuiltinType *>(inner->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic preserves struct pointer type after address-of") {
  auto result = analyze(R"(
struct Point {
    int x;
};

struct Point *main() {
    struct Point p;
    return &p;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<UnaryExpr>(ret);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<StructType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->name == "Point");

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic preserves union pointer type after address-of") {
  auto result = analyze(R"(
union Data {
    int x;
    float y;
};

union Data *main() {
    union Data d;
    return &d;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<UnaryExpr>(ret);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<UnionType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->name == "Data");

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic preserves enum pointer type after address-of") {
  auto result = analyze(R"(
enum Color {
    RED,
    GREEN
};

enum Color *main() {
    enum Color c = RED;
    return &c;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<UnaryExpr>(ret);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<EnumType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->name == "Color");

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}