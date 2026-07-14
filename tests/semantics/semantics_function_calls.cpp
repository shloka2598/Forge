#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

#include "types/Types.h"

TEST_CASE("Semantic assigns int type to function call") {
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

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::FUNCTION_CALL);

  auto *expr = get_expr<FunctionCallExpr>(ret);

  REQUIRE(expr->function_name == "foo");
  REQUIRE(expr->arguments.empty());

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns float type to function call") {
  auto result = analyze(R"(
float foo() {
    return 1.5f;
}

float main() {
    return foo();
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *expr = get_expr<FunctionCallExpr>(ret);

  REQUIRE(expr->function_name == "foo");
  REQUIRE(expr->arguments.empty());

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns double type to function call") {
  auto result = analyze(R"(
double foo() {
    return 3.14;
}

double main() {
    return foo();
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *expr = get_expr<FunctionCallExpr>(ret);

  REQUIRE(expr->function_name == "foo");
  REQUIRE(expr->arguments.empty());

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns pointer type to function call") {
  auto result = analyze(R"(
int *foo(int *p) {
    return p;
}

int *main() {
    int x;
    return foo(&x);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *expr = get_expr<FunctionCallExpr>(ret);

  REQUIRE(expr->function_name == "foo");
  REQUIRE(expr->arguments.size() == 1);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns struct type to function call") {
  auto result = analyze(R"(
struct Point {
    int x;
};

struct Point foo() {
    struct Point p;
    return p;
}

struct Point main() {
    return foo();
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *expr = get_expr<FunctionCallExpr>(ret);

  REQUIRE(expr->function_name == "foo");
  REQUIRE(expr->arguments.empty());

  auto *type = get_type<StructType>(expr);

  REQUIRE(type->name == "Point");

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic analyzes single integer argument") {
  auto result = analyze(R"(
int foo(int x) {
    return x;
}

int main() {
    return foo(42);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");
  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *call = get_expr<FunctionCallExpr>(ret);

  REQUIRE(call->function_name == "foo");
  REQUIRE(call->arguments.size() == 1);

  auto *arg = get_argument<IntLetExpr>(call, 0);

  REQUIRE(arg->value == 42);

  auto *type = get_type<BuiltinType>(arg);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes multiple arguments") {
  auto result = analyze(R"(
int foo(int a, float b, double c) {
    return a;
}

int main() {
    return foo(1, 2.5f, 3.14);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");
  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *call = get_expr<FunctionCallExpr>(ret);

  REQUIRE(call->function_name == "foo");
  REQUIRE(call->arguments.size() == 3);

  auto *arg0 = get_argument<IntLetExpr>(call, 0);
  auto *arg1 = get_argument<FloatLetExpr>(call, 1);
  auto *arg2 = get_argument<DoubleLetExpr>(call, 2);

  REQUIRE(arg0->value == 1);
  REQUIRE(arg1->value == Catch::Approx(2.5f));
  REQUIRE(arg2->value == Catch::Approx(3.14));

  REQUIRE(get_type<BuiltinType>(arg0)->builtin_kind == BuiltinKind::INT);
  REQUIRE(get_type<BuiltinType>(arg1)->builtin_kind == BuiltinKind::FLOAT);
  REQUIRE(get_type<BuiltinType>(arg2)->builtin_kind == BuiltinKind::DOUBLE);
}

TEST_CASE("Semantic analyzes identifier argument") {
  auto result = analyze(R"(
int foo(int x) {
    return x;
}

int main() {
    int value = 10;
    return foo(value);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");
  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *call = get_expr<FunctionCallExpr>(ret);

  REQUIRE(call->arguments.size() == 1);

  auto *arg = get_argument<IdentifierExpr>(call, 0);

  REQUIRE(arg->identifier_name == "value");

  auto *type = get_type<BuiltinType>(arg);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(arg->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic analyzes pointer argument") {
  auto result = analyze(R"(
int foo(int *p) {
    return 0;
}

int main() {
    int x;
    return foo(&x);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");
  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *call = get_expr<FunctionCallExpr>(ret);

  REQUIRE(call->arguments.size() == 1);

  auto *arg = get_argument<UnaryExpr>(call, 0);

  auto *type = get_type<PointerType>(arg);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes struct argument") {
  auto result = analyze(R"(
struct Point {
    int x;
};

int foo(struct Point p) {
    return p.x;
}

int main() {
    struct Point p;
    return foo(p);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");
  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *call = get_expr<FunctionCallExpr>(ret);

  REQUIRE(call->arguments.size() == 1);

  auto *arg = get_argument<IdentifierExpr>(call, 0);

  auto *type = get_type<StructType>(arg);

  REQUIRE(type->name == "Point");
}

TEST_CASE("Semantic decays array argument to pointer") {
  auto result = analyze(R"(
int foo(int *p) {
    return 0;
}

int main() {
    int arr[10];
    return foo(arr);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *call = get_expr<FunctionCallExpr>(ret);

  REQUIRE(call->arguments.size() == 1);

  auto *arg = get_argument<IdentifierExpr>(call, 0);

  auto *type = get_type<ArrayType>(arg);

  auto *base = dynamic_cast<BuiltinType *>(type->element_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes nested function call") {
  auto result = analyze(R"(
int foo() {
    return 10;
}

int bar(int x) {
    return x;
}

int main() {
    return bar(foo());
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *outer = get_expr<FunctionCallExpr>(ret);

  REQUIRE(outer->function_name == "bar");
  REQUIRE(outer->arguments.size() == 1);

  auto *inner = get_argument<FunctionCallExpr>(outer, 0);

  REQUIRE(inner->function_name == "foo");

  auto *type = get_type<BuiltinType>(inner);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes function call in binary expression") {
  auto result = analyze(R"(
int foo() {
    return 5;
}

int main() {
    return foo() + 2;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *binary = get_expr<BinaryExpr>(ret);

  REQUIRE(binary->op == TokenType::PLUS);

  auto *call =
      dynamic_cast<FunctionCallExpr *>(binary->left_expr.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->function_name == "foo");

  auto *type = get_type<BuiltinType>(call);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes function call in unary expression") {
  auto result = analyze(R"(
int foo() {
    return 10;
}

int main() {
    return -foo();
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *unary = get_expr<UnaryExpr>(ret);

  auto *call =
      dynamic_cast<FunctionCallExpr *>(unary->right_expr.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->function_name == "foo");

  auto *type = get_type<BuiltinType>(call);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes function call passed as argument") {
  auto result = analyze(R"(
int foo() {
    return 1;
}

int bar(int x) {
    return x;
}

int baz(int x) {
    return x;
}

int main() {
    return baz(bar(foo()));
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *baz = get_expr<FunctionCallExpr>(ret);

  REQUIRE(baz->function_name == "baz");

  auto *bar = get_argument<FunctionCallExpr>(baz, 0);

  REQUIRE(bar->function_name == "bar");

  auto *foo = get_argument<FunctionCallExpr>(bar, 0);

  REQUIRE(foo->function_name == "foo");

  auto *type = get_type<BuiltinType>(foo);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes binary expression argument") {
  auto result = analyze(R"(
int foo(int x) {
    return x;
}

int main() {
    return foo(2 + 3);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *call = get_expr<FunctionCallExpr>(ret);

  REQUIRE(call->arguments.size() == 1);

  auto *arg = get_argument<BinaryExpr>(call, 0);

  REQUIRE(arg->op == TokenType::PLUS);

  auto *type = get_type<BuiltinType>(arg);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes unary expression argument") {
  auto result = analyze(R"(
int foo(int x) {
    return x;
}

int main() {
    return foo(-5);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *call = get_expr<FunctionCallExpr>(ret);

  auto *arg = get_argument<UnaryExpr>(call, 0);

  auto *type = get_type<BuiltinType>(arg);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes assignment expression argument") {
  auto result = analyze(R"(
int foo(int x) {
    return x;
}

int main() {
    int value = 0;
    return foo(value = 42);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *call = get_expr<FunctionCallExpr>(ret);

  auto *arg = get_argument<AssignmentExpr>(call, 0);

  auto *type = get_type<BuiltinType>(arg);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes member access argument") {
  auto result = analyze(R"(
struct Point {
    int x;
};

int foo(int x) {
    return x;
}

int main() {
    struct Point p;
    return foo(p.x);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *call = get_expr<FunctionCallExpr>(ret);

  auto *arg = get_argument<MemberAccessExpr>(call, 0);

  auto *type = get_type<BuiltinType>(arg);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(arg->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic analyzes array subscript argument") {
  auto result = analyze(R"(
int foo(int x) {
    return x;
}

int main() {
    int arr[5];
    return foo(arr[2]);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *call = get_expr<FunctionCallExpr>(ret);

  auto *arg = get_argument<ArrayAccessExpr>(call, 0);

  auto *type = get_type<BuiltinType>(arg);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(arg->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic analyzes recursive function call") {
  auto result = analyze(R"(
int fact(int n) {
    if (n == 0)
        return 1;

    return fact(n - 1);
}

int main() {
    return fact(5);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *call = get_expr<FunctionCallExpr>(ret);

  REQUIRE(call->function_name == "fact");

  auto *type = get_type<BuiltinType>(call);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes function call with many arguments") {
  auto result = analyze(R"(
int foo(int a, int b, int c, int d, int e, int f) {
    return a;
}

int main() {
    return foo(1, 2, 3, 4, 5, 6);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *call = get_expr<FunctionCallExpr>(ret);

  REQUIRE(call->arguments.size() == 6);

  for (size_t i = 0; i < 6; ++i) {
    auto *arg = get_argument<IntLetExpr>(call, i);

    auto *type = get_type<BuiltinType>(arg);

    REQUIRE(type->builtin_kind == BuiltinKind::INT);
  }
}

TEST_CASE("Semantic analyzes function returning pointer used in dereference") {
  auto result = analyze(R"(
int *foo(int *p) {
    return p;
}

int main() {
    int value = 10;
    return *foo(&value);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *unary = get_expr<UnaryExpr>(ret);

  auto *call = dynamic_cast<FunctionCallExpr *>(unary->right_expr.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->function_name == "foo");

  auto *type = get_type<PointerType>(call);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes function returning struct used in member access") {
  auto result = analyze(R"(
struct Point {
    int x;
};

struct Point foo() {
    struct Point p;
    return p;
}

int main() {
    return foo().x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *member = get_expr<MemberAccessExpr>(ret);

  auto *call = dynamic_cast<FunctionCallExpr *>(member->base_expr.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->function_name == "foo");

  auto *type = get_type<BuiltinType>(member);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes nested function calls with multiple arguments") {
  auto result = analyze(R"(
int foo(int x) {
    return x;
}

int bar(int a, int b) {
    return a + b;
}

int main() {
    return bar(foo(1), foo(2));
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 0);

  auto *bar = get_expr<FunctionCallExpr>(ret);

  REQUIRE(bar->function_name == "bar");
  REQUIRE(bar->arguments.size() == 2);

  auto *left = get_argument<FunctionCallExpr>(bar, 0);
  auto *right = get_argument<FunctionCallExpr>(bar, 1);

  REQUIRE(left->function_name == "foo");
  REQUIRE(right->function_name == "foo");
}