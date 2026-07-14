#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

#include "types/Types.h"

TEST_CASE("Semantic analyzes integer global variable") {
  auto result = analyze(R"(
int value;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result);

  REQUIRE(decl->declaration->name == "value");

  auto *type = static_cast<BuiltinType *>(decl->declaration->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(decl->declaration->initializer == nullptr);
}

TEST_CASE("Semantic analyzes initialized integer global") {
  auto result = analyze(R"(
int value = 42;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result);

  REQUIRE(decl->declaration->initializer != nullptr);

  auto *expr =
      dynamic_cast<IntLetExpr *>(decl->declaration->initializer.get());

  REQUIRE(expr != nullptr);

  REQUIRE(expr->value == 42);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes float global variable") {
  auto result = analyze(R"(
float value = 1.5f;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result);

  auto *expr =
      dynamic_cast<FloatLetExpr *>(decl->declaration->initializer.get());

  REQUIRE(expr != nullptr);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
}

TEST_CASE("Semantic analyzes double global variable") {
  auto result = analyze(R"(
double value = 3.14;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result);

  auto *expr =
      dynamic_cast<DoubleLetExpr *>(decl->declaration->initializer.get());

  REQUIRE(expr != nullptr);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);
}

TEST_CASE("Semantic analyzes character global variable") {
  auto result = analyze(R"(
char value = 'a';
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result);

  auto *expr =
      dynamic_cast<CharLetExpr *>(decl->declaration->initializer.get());

  REQUIRE(expr != nullptr);

  REQUIRE(expr->value == 'a');

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::CHAR);
}

TEST_CASE("Semantic analyzes boolean expression global initializer") {
  auto result = analyze(R"(
int value = 1 < 2;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result);

  auto *expr =
      dynamic_cast<BinaryExpr *>(decl->declaration->initializer.get());

  REQUIRE(expr != nullptr);

  REQUIRE(expr->op == TokenType::SMALLER_THAN);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes global integer array") {
  auto result = analyze(R"(
int arr[5];
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result);

  REQUIRE(decl->declaration->array_initializer == std::nullopt);

  auto *type =
      static_cast<ArrayType *>(decl->declaration->resolved_type);

  REQUIRE(type->size == 5);

  auto *base =
      dynamic_cast<BuiltinType *>(type->element_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes initialized global array") {
  auto result = analyze(R"(
int arr[3] = {1, 2, 3};
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result);

  REQUIRE(decl->declaration->array_initializer.has_value());

  const auto &init = *decl->declaration->array_initializer;

  REQUIRE(init.children.size() == 3);

  for (const auto &child : init.children) {
    auto *expr =
        dynamic_cast<IntLetExpr *>(child.expr.get());

    REQUIRE(expr != nullptr);

    auto *type = get_type<BuiltinType>(expr);

    REQUIRE(type->builtin_kind == BuiltinKind::INT);
  }
}

TEST_CASE("Semantic analyzes global pointer") {
  auto result = analyze(R"(
int *ptr;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result);

  auto *type =
      static_cast<PointerType *>(decl->declaration->resolved_type);

  auto *base =
      dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);

  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes initialized global pointer") {
  auto result = analyze(R"(
int value;
int *ptr = &value;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result, 1);

  auto *expr =
      dynamic_cast<UnaryExpr *>(decl->declaration->initializer.get());

  REQUIRE(expr != nullptr);

  auto *type = get_type<PointerType>(expr);

  auto *base =
      dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);

  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes global array of pointers") {
  auto result = analyze(R"(
int *ptrs[4];
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result);

  auto *array =
      static_cast<ArrayType *>(decl->declaration->resolved_type);

  REQUIRE(array->size == 4);

  auto *ptr =
      dynamic_cast<PointerType *>(array->element_type);

  REQUIRE(ptr != nullptr);

  auto *base =
      dynamic_cast<BuiltinType *>(ptr->pointee_type);

  REQUIRE(base != nullptr);

  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes multidimensional global array") {
  auto result = analyze(R"(
int matrix[2][3];
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result);

  auto *outer =
      static_cast<ArrayType *>(decl->declaration->resolved_type);

  REQUIRE(outer->size == 2);

  auto *inner =
      dynamic_cast<ArrayType *>(outer->element_type);

  REQUIRE(inner != nullptr);

  REQUIRE(inner->size == 3);

  auto *base =
      dynamic_cast<BuiltinType *>(inner->element_type);

  REQUIRE(base != nullptr);

  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes global struct variable") {
  auto result = analyze(R"(
struct Point {
    int x;
    int y;
};

struct Point p;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result, 1);

  auto *type =
      static_cast<StructType *>(decl->declaration->resolved_type);

  REQUIRE(type->name == "Point");
}

/*
TODO: Add support for aggregate initialization for struct and union
TEST_CASE("Semantic analyzes initialized global struct") {
  auto result = analyze(R"(
struct Point {
    int x;
};

struct Point p = {1};
)");

  REQUIRE_FALSE(result.semantic_error);

  result.program.statements[0]->show_statement();

  auto *decl = get_global_variable_decl(result, 1);

  REQUIRE(decl->declaration->array_initializer.has_value());

  REQUIRE(decl->declaration->resolved_type->kind == TypeKind::STRUCT);
}
  */

TEST_CASE("Semantic analyzes global enum variable") {
  auto result = analyze(R"(
enum Color {
    RED,
    GREEN
};

enum Color c;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result, 1);

  auto *type =
      static_cast<EnumType *>(decl->declaration->resolved_type);

  REQUIRE(type->name == "Color");
}

TEST_CASE("Semantic analyzes initialized global enum") {
  auto result = analyze(R"(
enum Color {
    RED,
    GREEN
};

enum Color c = GREEN;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result, 1);

  auto *expr =
      dynamic_cast<IdentifierExpr *>(decl->declaration->initializer.get());

  REQUIRE(expr != nullptr);

  REQUIRE(expr->identifier_name == "GREEN");

  auto *type = get_type<EnumType>(expr);

  REQUIRE(type->name == "Color");
}

TEST_CASE("Semantic analyzes multiple global variables") {
  auto result = analyze(R"(
int a;
float b;
double c;
char d;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *a = get_global_variable_decl(result, 0);
  auto *b = get_global_variable_decl(result, 1);
  auto *c = get_global_variable_decl(result, 2);
  auto *d = get_global_variable_decl(result, 3);

  REQUIRE(a->declaration->name == "a");
  REQUIRE(b->declaration->name == "b");
  REQUIRE(c->declaration->name == "c");
  REQUIRE(d->declaration->name == "d");
}

TEST_CASE("Semantic analyzes globals before function") {
  auto result = analyze(R"(
int global = 42;

int main() {
    return global;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *decl = get_global_variable_decl(result);

  REQUIRE(decl->declaration->name == "global");

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 0);

  auto *id = get_expr<IdentifierExpr>(ret);

  REQUIRE(id->identifier_name == "global");

  auto *type = get_type<BuiltinType>(id);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}
