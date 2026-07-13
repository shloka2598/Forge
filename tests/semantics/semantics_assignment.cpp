#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

#include "types/Types.h"

TEST_CASE("Semantic assigns int type to assignment expression") {
  auto result = analyze(R"(
int main() {
    int x;
    return (x = 42);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::ASSIGNMENT);

  auto *expr = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  // C assignment expressions are rvalues
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns float type to assignment expression") {
  auto result = analyze(R"(
float main() {
    float x;
    return (x = 3.5f);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns double type to assignment expression") {
  auto result = analyze(R"(
double main() {
    double x;
    return (x = 3.14);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns pointer type to assignment expression") {
  auto result = analyze(R"(
int *main() {
    int value;
    int *ptr;
    return (ptr = &value);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  auto *expr = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns struct type to assignment expression") {
  auto result = analyze(R"(
struct Point {
    int x;
};

struct Point main() {
    struct Point a;
    struct Point b;
    return (a = b);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  auto *expr = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<StructType>(expr);

  REQUIRE(type->name == "Point");

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns through pointer dereference") {
  auto result = analyze(R"(
int main() {
    int x = 0;
    int *p = &x;
    return (*p = 42);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::ASSIGNMENT);

  auto *expr = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns through struct member") {
  auto result = analyze(R"(
struct Point {
    int x;
};

int main() {
    struct Point p;
    return (p.x = 5);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns through pointer member") {
  auto result = analyze(R"(
struct Point {
    int x;
};

int main() {
    struct Point p;
    struct Point *ptr = &p;
    return (ptr->x = 5);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  auto *expr = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns through array subscript") {
  auto result = analyze(R"(
int main() {
    int arr[5];
    return (arr[2] = 10);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns pointer value through dereference") {
  auto result = analyze(R"(
int *main() {
    int value;
    int *a;
    int *b = &value;
    return (*(&a) = b);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 3);

  auto *expr = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns struct through assignment expression") {
  auto result = analyze(R"(
struct Point {
    int x;
    int y;
};

struct Point main() {
    struct Point a;
    struct Point b;
    return (a = b);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::ASSIGNMENT);

  auto *expr = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<StructType>(expr);

  REQUIRE(type->name == "Point");

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns union through assignment expression") {
  auto result = analyze(R"(
union Data {
    int i;
    float f;
};

union Data main() {
    union Data a;
    union Data b;
    return (a = b);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  auto *expr = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<UnionType>(expr);

  REQUIRE(type->name == "Data");

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to chained assignment") {
  auto result = analyze(R"(
int main() {
    int a;
    int b;
    return (a = b = 5);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  auto *outer = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<BuiltinType>(outer);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(outer->value_category == ValueCategory::RVALUE);

  REQUIRE(outer->rhs->expr_type() == ExprType::ASSIGNMENT);
}

TEST_CASE("Semantic assigns correct pointer type to chained assignment") {
  auto result = analyze(R"(
int *main() {
    int value;
    int *a;
    int *b;
    return (a = b = &value);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 3);

  auto *outer = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<PointerType>(outer);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(outer->rhs->expr_type() == ExprType::ASSIGNMENT);

  REQUIRE(outer->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assignment expression preserves float type") {
  auto result = analyze(R"(
float main() {
    float x;
    return (x = 10);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
}

TEST_CASE("Semantic assignment expression preserves pointer type") {
  auto result = analyze(R"(
int *main() {
    int value;
    int *ptr;
    return (ptr = &value);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  auto *expr = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic handles compound addition assignment") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    return (x += 3);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<AssignmentExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic handles compound subtraction assignment") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    return (x -= 3);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<AssignmentExpr>(
      get_stmt<ReturnStmt>(get_function_decl(result), 1));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic handles compound multiplication assignment") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    return (x *= 3);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<AssignmentExpr>(
      get_stmt<ReturnStmt>(get_function_decl(result), 1));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic handles compound division assignment") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    return (x /= 3);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<AssignmentExpr>(
      get_stmt<ReturnStmt>(get_function_decl(result), 1));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic handles compound modulo assignment") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    return (x %= 3);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<AssignmentExpr>(
      get_stmt<ReturnStmt>(get_function_decl(result), 1));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic handles compound left shift assignment") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    return (x <<= 2);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<AssignmentExpr>(
      get_stmt<ReturnStmt>(get_function_decl(result), 1));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic handles compound right shift assignment") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    return (x >>= 2);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<AssignmentExpr>(
      get_stmt<ReturnStmt>(get_function_decl(result), 1));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic handles compound bitwise AND assignment") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    return (x &= 3);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<AssignmentExpr>(
      get_stmt<ReturnStmt>(get_function_decl(result), 1));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic handles compound bitwise OR assignment") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    return (x |= 3);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<AssignmentExpr>(
      get_stmt<ReturnStmt>(get_function_decl(result), 1));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic handles compound bitwise XOR assignment") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    return (x ^= 3);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *expr = get_expr<AssignmentExpr>(
      get_stmt<ReturnStmt>(get_function_decl(result), 1));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}