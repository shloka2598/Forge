#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

#include "types/Types.h"

TEST_CASE("Semantic assigns int type to initialized variable") {
  auto result = analyze(R"(
int main() {
    int x = 42;
    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->var_name == "x");

  auto *type = static_cast<BuiltinType *>(decl->resolved_type);

  REQUIRE(type != nullptr);
  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(decl->expr_ptr != nullptr);

  auto *init = dynamic_cast<IntLetExpr *>(decl->expr_ptr.get());

  REQUIRE(init != nullptr);
  REQUIRE(init->value == 42);
}

TEST_CASE("Semantic assigns float type to initialized variable") {
  auto result = analyze(R"(
float main() {
    float x = 3.5f;
    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->var_name == "x");

  auto *type = static_cast<BuiltinType *>(decl->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);

  auto *init = dynamic_cast<FloatLetExpr *>(decl->expr_ptr.get());

  REQUIRE(init != nullptr);

  REQUIRE(init->value == Catch::Approx(3.5f));
}

TEST_CASE("Semantic assigns double type to initialized variable") {
  auto result = analyze(R"(
double main() {
    double x = 8.25;
    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  auto *type = static_cast<BuiltinType *>(decl->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);

  auto *init = dynamic_cast<DoubleLetExpr *>(decl->expr_ptr.get());

  REQUIRE(init != nullptr);

  REQUIRE(init->value == Catch::Approx(8.25));
}

TEST_CASE("Semantic assigns char type to initialized variable") {
  auto result = analyze(R"(
char main() {
    char c = 'A';
    return c;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  auto *type = static_cast<BuiltinType *>(decl->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::CHAR);

  auto *init = dynamic_cast<CharLetExpr *>(decl->expr_ptr.get());

  REQUIRE(init != nullptr);

  REQUIRE(init->value == 'A');
}

TEST_CASE("Semantic supports declaration without initializer") {
  auto result = analyze(R"(
int main() {
    int x;
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->var_name == "x");

  auto *type = static_cast<BuiltinType *>(decl->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(decl->expr_ptr == nullptr);
  REQUIRE_FALSE(decl->init.has_value());
}

TEST_CASE("Semantic assigns pointer type to initialized pointer variable") {
  auto result = analyze(R"(
int main() {
    int value;
    int *ptr = &value;
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 1);

  REQUIRE(decl->var_name == "ptr");

  auto *type = static_cast<PointerType *>(decl->resolved_type);

  REQUIRE(type != nullptr);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(decl->expr_ptr != nullptr);
  REQUIRE(decl->expr_ptr->expr_type() == ExprType::UNARY);
}

TEST_CASE("Semantic assigns array type to array declaration") {
  auto result = analyze(R"(
int main() {
    int arr[10];
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->var_name == "arr");

  auto *type = static_cast<ArrayType *>(decl->resolved_type);

  REQUIRE(type != nullptr);

  REQUIRE(type->size == 10);

  auto *base = dynamic_cast<BuiltinType *>(type->element_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(decl->expr_ptr == nullptr);
}

TEST_CASE("Semantic assigns multidimensional array type") {
  auto result = analyze(R"(
int main() {
    int matrix[3][4];
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  auto *outer = static_cast<ArrayType *>(decl->resolved_type);

  REQUIRE(outer != nullptr);
  REQUIRE(outer->size == 3);

  auto *inner = dynamic_cast<ArrayType *>(outer->element_type);

  REQUIRE(inner != nullptr);
  REQUIRE(inner->size == 4);

  auto *base = dynamic_cast<BuiltinType *>(inner->element_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic assigns struct type to variable declaration") {
  auto result = analyze(R"(
struct Point {
    int x;
};

int main() {
    struct Point p;
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->var_name == "p");

  auto *type = static_cast<StructType *>(decl->resolved_type);

  REQUIRE(type != nullptr);
  REQUIRE(type->name == "Point");

  REQUIRE(decl->expr_ptr == nullptr);
}

TEST_CASE("Semantic assigns enum type to variable declaration") {
  auto result = analyze(R"(
enum Color {
    RED,
    GREEN
};

int main() {
    enum Color c = RED;
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->var_name == "c");

  auto *type = static_cast<EnumType *>(decl->resolved_type);

  REQUIRE(type != nullptr);
  REQUIRE(type->name == "Color");

  REQUIRE(decl->expr_ptr != nullptr);
  REQUIRE(decl->expr_ptr->expr_type() == ExprType::IDENTIFIER);
}

TEST_CASE("Semantic analyzes binary expression initializer") {
  auto result = analyze(R"(
int main() {
    int x = 2 + 3;
    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->expr_ptr->expr_type() == ExprType::BINARY);

  auto *expr = dynamic_cast<BinaryExpr *>(decl->expr_ptr.get());

  REQUIRE(expr != nullptr);
  REQUIRE(expr->op == TokenType::PLUS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes unary expression initializer") {
  auto result = analyze(R"(
int main() {
    int x = -5;
    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->expr_ptr->expr_type() == ExprType::UNARY);

  auto *expr = dynamic_cast<UnaryExpr *>(decl->expr_ptr.get());

  REQUIRE(expr != nullptr);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes function call initializer") {
  auto result = analyze(R"(
int foo() {
    return 42;
}

int main() {
    int x = foo();
    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *decl = get_stmt<VariableDeclarationStmt>(main_fn, 0);

  REQUIRE(decl->expr_ptr->expr_type() == ExprType::FUNCTION_CALL);

  auto *expr = dynamic_cast<FunctionCallExpr *>(decl->expr_ptr.get());

  REQUIRE(expr != nullptr);
  REQUIRE(expr->function_name == "foo");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes member access initializer") {
  auto result = analyze(R"(
struct Point {
    int x;
};

int main() {
    struct Point p;
    int value = p.x;
    return value;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 1);

  REQUIRE(decl->expr_ptr->expr_type() == ExprType::MEMBER_ACCESS);

  auto *expr = dynamic_cast<MemberAccessExpr *>(decl->expr_ptr.get());

  REQUIRE(expr != nullptr);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic analyzes array access initializer") {
  auto result = analyze(R"(
int main() {
    int arr[5];
    int x = arr[2];
    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 1);

  REQUIRE(decl->expr_ptr->expr_type() == ExprType::ARRAY_ACCESS);

  auto *expr = dynamic_cast<ArrayAccessExpr *>(decl->expr_ptr.get());

  REQUIRE(expr != nullptr);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic analyzes identifier initializer") {
  auto result = analyze(R"(
int main() {
    int value = 10;
    int x = value;
    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 1);

  REQUIRE(decl->expr_ptr->expr_type() == ExprType::IDENTIFIER);

  auto *expr = dynamic_cast<IdentifierExpr *>(decl->expr_ptr.get());

  REQUIRE(expr != nullptr);
  REQUIRE(expr->identifier_name == "value");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic initializes pointer from address expression") {
  auto result = analyze(R"(
int main() {
    int value;
    int *ptr = &value;
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 1);

  auto *type = get_type<PointerType>(decl->expr_ptr.get());

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic initializes pointer from array decay") {
  auto result = analyze(R"(
int main() {
    int arr[5];
    int *ptr = arr;
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 1);

  REQUIRE(decl->expr_ptr->expr_type() == ExprType::IDENTIFIER);

  auto *expr = dynamic_cast<IdentifierExpr *>(decl->expr_ptr.get());

  REQUIRE(expr != nullptr);

  // The expression itself should still be an array.
  auto *type = get_type<ArrayType>(expr);

  REQUIRE(type->size == 5);
}

TEST_CASE("Semantic initializes struct from identifier") {
  auto result = analyze(R"(
struct Point {
    int x;
};

int main() {
    struct Point a;
    struct Point b = a;
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 1);

  REQUIRE(decl->expr_ptr->expr_type() == ExprType::IDENTIFIER);

  auto *expr = dynamic_cast<IdentifierExpr *>(decl->expr_ptr.get());

  REQUIRE(expr != nullptr);

  auto *type = get_type<StructType>(expr);

  REQUIRE(type->name == "Point");
}

TEST_CASE("Semantic initializes enum from enumerator") {
  auto result = analyze(R"(
enum Color {
    RED,
    GREEN
};

int main() {
    enum Color c = GREEN;
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  auto *expr = dynamic_cast<IdentifierExpr *>(decl->expr_ptr.get());

  REQUIRE(expr != nullptr);

  REQUIRE(expr->identifier_name == "GREEN");

  auto *type = get_type<EnumType>(expr);

  REQUIRE(type->name == "Color");
}

TEST_CASE("Semantic initializes variable from nested function call") {
  auto result = analyze(R"(
int foo() {
    return 10;
}

int bar() {
    return foo();
}

int main() {
    int value = bar();
    return value;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *decl = get_stmt<VariableDeclarationStmt>(main_fn, 0);

  auto *call = dynamic_cast<FunctionCallExpr *>(decl->expr_ptr.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->function_name == "bar");

  auto *type = get_type<BuiltinType>(call);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic initializes variable from parenthesized expression") {
  auto result = analyze(R"(
int main() {
    int value = (2 + 3);
    return value;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  auto *expr = dynamic_cast<BinaryExpr *>(decl->expr_ptr.get());

  REQUIRE(expr != nullptr);

  REQUIRE(expr->op == TokenType::PLUS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}