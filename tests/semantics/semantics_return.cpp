#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

#include "types/Types.h"

TEST_CASE("Semantic analyzes integer return") {
  auto result = analyze(R"(
int main() {
    return 42;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  auto *expr = get_expr<IntLetExpr>(ret);

  REQUIRE(expr->value == 42);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes float return") {
  auto result = analyze(R"(
float main() {
    return 3.5f;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  auto *expr = get_expr<FloatLetExpr>(ret);

  REQUIRE(expr->value == Catch::Approx(3.5f));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
}

TEST_CASE("Semantic analyzes double return") {
  auto result = analyze(R"(
double main() {
    return 3.14159;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  auto *expr = get_expr<DoubleLetExpr>(ret);

  REQUIRE(expr->value == Catch::Approx(3.14159));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);
}

TEST_CASE("Semantic analyzes character return") {
  auto result = analyze(R"(
char main() {
    return 'A';
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  auto *expr = get_expr<CharLetExpr>(ret);

  REQUIRE(expr->value == 'A');

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::CHAR);
}

TEST_CASE("Semantic analyzes identifier return") {
  auto result = analyze(R"(
int main() {
    int value = 123;
    return value;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "value");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes binary expression return") {
  auto result = analyze(R"(
int main() {
    return 5 + 10;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::PLUS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes unary expression return") {
  auto result = analyze(R"(
int main() {
    int x = 5;
    return -x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<UnaryExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes function call return") {
  auto result = analyze(R"(
int foo() {
    return 42;
}

int main() {
    return foo();
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *call = get_expr<FunctionCallExpr>(ret);

  REQUIRE(call->function_name == "foo");

  auto *type = get_type<BuiltinType>(call);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes array access return") {
  auto result = analyze(R"(
int main() {
    int arr[3];
    return arr[1];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic analyzes member access return") {
  auto result = analyze(R"(
struct Point {
    int x;
};

int main() {
    struct Point p;
    return p.x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *expr = get_expr<MemberAccessExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic analyzes pointer return") {
  auto result = analyze(R"(
int *main() {
    int value;
    return &value;
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
}

TEST_CASE("Semantic analyzes struct return") {
  auto result = analyze(R"(
struct Point {
    int x;
};

struct Point foo() {
    struct Point p;
    return p;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result, "foo");

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<IdentifierExpr>(ret);

  auto *type = get_type<StructType>(expr);

  REQUIRE(type->name == "Point");
}

TEST_CASE("Semantic analyzes enum return") {
  auto result = analyze(R"(
enum Color {
    RED,
    GREEN
};

enum Color foo() {
    return GREEN;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result, "foo");

  auto *ret = get_stmt<ReturnStmt>(fn, 0);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "GREEN");

  auto *type = get_type<EnumType>(expr);

  REQUIRE(type->name == "Color");
}

TEST_CASE("Semantic analyzes array decay in return") {
  auto result = analyze(R"(
int *foo() {
    int arr[5];
    return arr;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result, "foo");

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<IdentifierExpr>(ret);

  auto *type = get_type<ArrayType>(expr);

  REQUIRE(type->size == 5);

  auto *base =
      dynamic_cast<BuiltinType *>(type->element_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes pointer arithmetic return") {
  auto result = analyze(R"(
int *foo() {
    int arr[5];
    int *p = arr;

    return p + 2;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result, "foo");

  auto *ret = get_stmt<ReturnStmt>(fn, 2);

  auto *expr = get_expr<BinaryExpr>(ret);

  REQUIRE(expr->op == TokenType::PLUS);

  auto *type = get_type<PointerType>(expr);

  auto *base =
      dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes return inside if statement") {
  auto result = analyze(R"(
int foo() {

    if (1) {
        return 5;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result, "foo");

  auto *if_stmt = get_stmt<IfStmt>(fn, 0);

  auto *ret =
      get_stmt<ReturnStmt>(if_stmt->then_body.get(), 0);

  auto *expr = get_expr<IntLetExpr>(ret);

  REQUIRE(expr->value == 5);
}

TEST_CASE("Semantic analyzes return inside while loop") {
  auto result = analyze(R"(
int foo() {

    while (1) {
        return 7;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result, "foo");

  auto *loop = get_stmt<WhileStmt>(fn, 0);

  auto *ret =
      get_stmt<ReturnStmt>(loop->body.get(), 0);

  auto *expr = get_expr<IntLetExpr>(ret);

  REQUIRE(expr->value == 7);
}

TEST_CASE("Semantic analyzes return inside for loop") {
  auto result = analyze(R"(
int foo() {

    for (;;) {
        return 9;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result, "foo");

  auto *loop = get_stmt<ForStmt>(fn, 0);

  auto *ret =
      get_stmt<ReturnStmt>(loop->body.get(), 0);

  auto *expr = get_expr<IntLetExpr>(ret);

  REQUIRE(expr->value == 9);
}

TEST_CASE("Semantic analyzes return inside switch case") {
  auto result = analyze(R"(
int foo() {

    switch (1) {
        case 1: {
            return 11;
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result, "foo");

  auto *sw = get_stmt<SwitchStmt>(fn, 0);

  auto *block =
      get_stmt<BlockStmt>(sw->cases[0].body.get(), 0);

  auto *ret =
      get_stmt<ReturnStmt>(block, 0);

  auto *expr = get_expr<IntLetExpr>(ret);

  REQUIRE(expr->value == 11);
}
