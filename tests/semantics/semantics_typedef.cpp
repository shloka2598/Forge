#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

#include "types/Types.h"

TEST_CASE("Semantic analyzes integer typedef") {
  auto result = analyze(R"(
typedef int Int;

Int value;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 1);

  REQUIRE(global->declaration->name == "value");

  auto *type =
      static_cast<BuiltinType *>(global->declaration->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes initialized integer typedef") {
  auto result = analyze(R"(
typedef int Int;

Int value = 42;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 1);

  auto *expr = dynamic_cast<IntLetExpr *>(global->declaration->initializer.get());

  REQUIRE(expr != nullptr);

  REQUIRE(expr->value == 42);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes local typedef variable") {
  auto result = analyze(R"(
typedef int Int;

int main() {
    Int value = 7;
    return value;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl =
      get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->var_name == "value");

  auto *type =
      static_cast<BuiltinType *>(decl->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes typedef as function return type") {
  auto result = analyze(R"(
typedef int Int;

Int foo() {
    return 42;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result, "foo");

  auto *ret =
      get_stmt<ReturnStmt>(fn, 0);

  auto *expr = get_expr<IntLetExpr>(ret);

  REQUIRE(expr->value == 42);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes typedef as function parameter") {
  auto result = analyze(R"(
typedef int Int;

Int foo(Int value) {
    return value;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result, "foo");

  REQUIRE(fn->declaration->parameters.size() == 1);

  auto &param = fn->declaration->parameters[0];

  REQUIRE(param.name.has_value());
  REQUIRE(*param.name == "value");

  auto *type =
      static_cast<BuiltinType *>(param.resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes multiple variables using same typedef") {
  auto result = analyze(R"(
typedef int Int;

Int a;
Int b;
Int c;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *a = get_global_variable_decl(result, 1);
  auto *b = get_global_variable_decl(result, 2);
  auto *c = get_global_variable_decl(result, 3);

  REQUIRE(a->declaration->name == "a");
  REQUIRE(b->declaration->name == "b");
  REQUIRE(c->declaration->name == "c");

  auto *ta =
      static_cast<BuiltinType *>(a->declaration->resolved_type);

  auto *tb =
      static_cast<BuiltinType *>(b->declaration->resolved_type);

  auto *tc =
      static_cast<BuiltinType *>(c->declaration->resolved_type);

  REQUIRE(ta->builtin_kind == BuiltinKind::INT);
  REQUIRE(tb->builtin_kind == BuiltinKind::INT);
  REQUIRE(tc->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes pointer typedef") {
  auto result = analyze(R"(
typedef int *IntPtr;

IntPtr ptr;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 1);

  auto *type =
      static_cast<PointerType *>(global->declaration->resolved_type);

  auto *base =
      dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes initialized pointer typedef") {
  auto result = analyze(R"(
typedef int *IntPtr;

int value;

IntPtr ptr = &value;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 2);

  auto *expr =
      dynamic_cast<UnaryExpr *>(global->declaration->initializer.get());

  REQUIRE(expr != nullptr);

  auto *type = get_type<PointerType>(expr);

  auto *base =
      dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes local pointer typedef") {
  auto result = analyze(R"(
typedef int *IntPtr;

int main() {
    IntPtr ptr;
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl =
      get_stmt<VariableDeclarationStmt>(fn, 0);

  auto *type =
      static_cast<PointerType *>(decl->resolved_type);

  auto *base =
      dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes array typedef") {
  auto result = analyze(R"(
typedef int IntArray[5];

IntArray arr;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 1);

  auto *type =
      static_cast<ArrayType *>(global->declaration->resolved_type);

  REQUIRE(type->size == 5);

  auto *base =
      dynamic_cast<BuiltinType *>(type->element_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes multidimensional array typedef") {
  auto result = analyze(R"(
typedef int Matrix[2][3];

Matrix mat;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 1);

  auto *outer =
      static_cast<ArrayType *>(global->declaration->resolved_type);

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

TEST_CASE("Semantic analyzes typedef used as function parameter") {
  auto result = analyze(R"(
typedef int Matrix[5];

int foo(Matrix arr) {
    return arr[0];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result, "foo");

  REQUIRE(fn->declaration->parameters.size() == 1);

  auto &param = fn->declaration->parameters[0];

  auto *type =
      static_cast<PointerType *>(param.resolved_type);

  auto *base =
      dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes typedef of named struct") {
  auto result = analyze(R"(
struct Point {
    int x;
    int y;
};

typedef struct Point Point;

Point p;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 2);

  auto *type =
      static_cast<StructType *>(global->declaration->resolved_type);

  REQUIRE(type->name == "Point");
}

TEST_CASE("Semantic analyzes typedef of named enum") {
  auto result = analyze(R"(
enum Color {
    RED,
    GREEN
};

typedef enum Color Color;

Color value;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 2);

  auto *type =
      static_cast<EnumType *>(global->declaration->resolved_type);

  REQUIRE(type->name == "Color");
}

TEST_CASE("Semantic analyzes typedef of anonymous struct") {
  auto result = analyze(R"(
typedef struct {
    int x;
    int y;
} Point;

Point p;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 1);

  auto *type =
      static_cast<StructType *>(global->declaration->resolved_type);

  REQUIRE(type != nullptr);

  REQUIRE(type->members.size() == 2);

  auto *field = dynamic_cast<BuiltinType *>(type->members[0].type);

  REQUIRE(field != nullptr);
  REQUIRE(field->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes typedef of anonymous enum") {
  auto result = analyze(R"(
typedef enum {
    RED,
    GREEN
} Color;

Color value;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 1);

  auto *type =
      static_cast<EnumType *>(global->declaration->resolved_type);

  REQUIRE(type != nullptr);

  REQUIRE(type->constants.size() == 2);
}

TEST_CASE("Semantic analyzes chained typedef") {
  auto result = analyze(R"(
typedef int Int;
typedef Int Number;

Number value;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 2);

  auto *type =
      static_cast<BuiltinType *>(global->declaration->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes chained pointer typedef") {
  auto result = analyze(R"(
typedef int *IntPtr;
typedef IntPtr Pointer;

Pointer ptr;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 2);

  auto *type =
      static_cast<PointerType *>(global->declaration->resolved_type);

  auto *base =
      dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);

  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes array of typedef pointers") {
  auto result = analyze(R"(
typedef int *IntPtr;

IntPtr ptrs[4];
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 1);

  auto *array =
      static_cast<ArrayType *>(global->declaration->resolved_type);

  REQUIRE(array->size == 4);

  auto *ptr =
      dynamic_cast<PointerType *>(array->element_type);

  REQUIRE(ptr != nullptr);

  auto *base =
      dynamic_cast<BuiltinType *>(ptr->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes pointer to typedef array") {
  auto result = analyze(R"(
typedef int Array[5];

Array *ptr;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 1);

  auto *ptr =
      static_cast<PointerType *>(global->declaration->resolved_type);

  auto *array =
      dynamic_cast<ArrayType *>(ptr->pointee_type);

  REQUIRE(array != nullptr);

  REQUIRE(array->size == 5);

  auto *base =
      dynamic_cast<BuiltinType *>(array->element_type);

  REQUIRE(base != nullptr);

  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes typedef inside local declaration") {
  auto result = analyze(R"(
typedef int Int;

int main() {
    Int a;
    Int b;

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *a =
      get_stmt<VariableDeclarationStmt>(fn, 0);

  auto *b =
      get_stmt<VariableDeclarationStmt>(fn, 1);

  REQUIRE(a->var_name == "a");
  REQUIRE(b->var_name == "b");

  auto *ta =
      static_cast<BuiltinType *>(a->resolved_type);

  auto *tb =
      static_cast<BuiltinType *>(b->resolved_type);

  REQUIRE(ta->builtin_kind == BuiltinKind::INT);
  REQUIRE(tb->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes typedef as function return and parameter") {
  auto result = analyze(R"(
typedef int Int;

Int identity(Int value) {
    return value;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result, "identity");

  auto &param = fn->declaration->parameters[0];

  auto *ptype =
      static_cast<BuiltinType *>(param.resolved_type);

  REQUIRE(ptype->builtin_kind == BuiltinKind::INT);

  auto *ret =
      get_stmt<ReturnStmt>(fn, 0);

  auto *expr =
      get_expr<IdentifierExpr>(ret);

  auto *rtype =
      get_type<BuiltinType>(expr);

  REQUIRE(rtype->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes typedef of typedef array") {
  auto result = analyze(R"(
typedef int Array[3];
typedef Array Matrix;

Matrix values;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 2);

  auto *array =
      static_cast<ArrayType *>(global->declaration->resolved_type);

  REQUIRE(array->size == 3);

  auto *base =
      dynamic_cast<BuiltinType *>(array->element_type);

  REQUIRE(base != nullptr);

  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes typedef of typedef struct") {
  auto result = analyze(R"(
struct Point {
    int x;
};

typedef struct Point Point;
typedef Point Coordinate;

Coordinate p;
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *global = get_global_variable_decl(result, 3);

  auto *type =
      static_cast<StructType *>(global->declaration->resolved_type);

  REQUIRE(type->name == "Point");
}