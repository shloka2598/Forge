#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

#include "types/Types.h"

TEST_CASE("Semantic resolves struct member access 2") {
  auto result = analyze(R"(
struct Point {
    int x;
    int y;
};

int main() {
    struct Point p;
    return p.x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::MEMBER_ACCESS);

  auto *expr = get_expr<MemberAccessExpr>(ret);

  REQUIRE(expr->op == TokenType::DOT);
  REQUIRE(expr->member_name == "x");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves pointer member access 2") {
  auto result = analyze(R"(
struct Point {
    int x;
};

int main() {
    struct Point p;
    struct Point *ptr = &p;
    return ptr->x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  auto *expr = get_expr<MemberAccessExpr>(ret);

  REQUIRE(expr->op == TokenType::ARROW);
  REQUIRE(expr->member_name == "x");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves nested struct member access") {
  auto result = analyze(R"(
struct Vec {
    int x;
};

struct Object {
    struct Vec pos;
};

int main() {
    struct Object obj;
    return obj.pos.x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<MemberAccessExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves chained pointer member access") {
  auto result = analyze(R"(
struct Vec {
    int x;
};

struct Object {
    struct Vec pos;
};

int main() {
    struct Object obj;
    struct Object *ptr = &obj;
    return ptr->pos.x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  auto *expr = get_expr<MemberAccessExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves union member access") {
  auto result = analyze(R"(
union Data {
    int x;
    float y;
};

int main() {
    union Data d;
    return d.x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<MemberAccessExpr>(ret);

  REQUIRE(expr->op == TokenType::DOT);
  REQUIRE(expr->member_name == "x");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}