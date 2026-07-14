#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

#include "types/Types.h"

TEST_CASE("Semantic initializes integer array") {
  auto result = analyze(R"(
int main() {
    int arr[3] = {1, 2, 3};
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->init.has_value());

  auto *type = static_cast<ArrayType *>(decl->resolved_type);

  REQUIRE(type != nullptr);
  REQUIRE(type->size == 3);

  auto *base = dynamic_cast<BuiltinType *>(type->element_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  const auto &init = *decl->init;

  REQUIRE_FALSE(init.is_leaf);
  REQUIRE(init.children.size() == 3);

  for (const auto &child : init.children) {
    REQUIRE(child.is_leaf);

    auto *expr = dynamic_cast<IntLetExpr *>(child.expr.get());

    REQUIRE(expr != nullptr);

    auto *expr_type = get_type<BuiltinType>(expr);

    REQUIRE(expr_type->builtin_kind == BuiltinKind::INT);
  }
}

TEST_CASE("Semantic initializes single element array") {
  auto result = analyze(R"(
int main() {
    int arr[1] = {42};
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->init.has_value());

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 1);

  auto *expr =
      dynamic_cast<IntLetExpr *>(init.children[0].expr.get());

  REQUIRE(expr != nullptr);
  REQUIRE(expr->value == 42);
}

TEST_CASE("Semantic initializes float array") {
  auto result = analyze(R"(
float main() {
    float arr[2] = {1.5f, 2.5f};
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  auto *type = static_cast<ArrayType *>(decl->resolved_type);

  auto *base =
      dynamic_cast<BuiltinType *>(type->element_type);

  REQUIRE(base->builtin_kind == BuiltinKind::FLOAT);

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 2);

  for (const auto &child : init.children) {
    auto *expr =
        dynamic_cast<FloatLetExpr *>(child.expr.get());

    REQUIRE(expr != nullptr);

    auto *expr_type = get_type<BuiltinType>(expr);

    REQUIRE(expr_type->builtin_kind == BuiltinKind::FLOAT);
  }
}

TEST_CASE("Semantic initializes double array") {
  auto result = analyze(R"(
double main() {
    double arr[2] = {1.25, 2.75};
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  auto *type = static_cast<ArrayType *>(decl->resolved_type);

  auto *base =
      dynamic_cast<BuiltinType *>(type->element_type);

  REQUIRE(base->builtin_kind == BuiltinKind::DOUBLE);

  REQUIRE(decl->init->children.size() == 2);
}

TEST_CASE("Semantic initializes character array") {
  auto result = analyze(R"(
char main() {
    char arr[3] = {'a', 'b', 'c'};
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  auto *type = static_cast<ArrayType *>(decl->resolved_type);

  auto *base =
      dynamic_cast<BuiltinType *>(type->element_type);

  REQUIRE(base->builtin_kind == BuiltinKind::CHAR);

  REQUIRE(decl->init->children.size() == 3);
}

TEST_CASE("Semantic initializes two dimensional integer array") {
  auto result = analyze(R"(
int main() {
    int matrix[2][3] = {
        {1, 2, 3},
        {4, 5, 6}
    };
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->init.has_value());

  const auto &init = *decl->init;

  REQUIRE_FALSE(init.is_leaf);
  REQUIRE(init.children.size() == 2);

  for (const auto &row : init.children) {
    REQUIRE_FALSE(row.is_leaf);
    REQUIRE(row.children.size() == 3);

    for (const auto &elem : row.children) {
      REQUIRE(elem.is_leaf);

      auto *expr = dynamic_cast<IntLetExpr *>(elem.expr.get());

      REQUIRE(expr != nullptr);

      auto *type = get_type<BuiltinType>(expr);

      REQUIRE(type->builtin_kind == BuiltinKind::INT);
    }
  }
}

TEST_CASE("Semantic initializes multidimensional float array") {
  auto result = analyze(R"(
float main() {
    float matrix[2][2] = {
        {1.5f, 2.5f},
        {3.5f, 4.5f}
    };
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->init.has_value());

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 2);

  for (const auto &row : init.children) {
    REQUIRE(row.children.size() == 2);

    for (const auto &elem : row.children) {
      auto *expr =
          dynamic_cast<FloatLetExpr *>(elem.expr.get());

      REQUIRE(expr != nullptr);

      auto *type = get_type<BuiltinType>(expr);

      REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
    }
  }
}

TEST_CASE("Semantic initializes multidimensional character array") {
  auto result = analyze(R"(
char main() {
    char matrix[2][2] = {
        {'a', 'b'},
        {'c', 'd'}
    };
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->init.has_value());

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 2);

  for (const auto &row : init.children) {
    REQUIRE(row.children.size() == 2);

    for (const auto &elem : row.children) {
      auto *expr =
          dynamic_cast<CharLetExpr *>(elem.expr.get());

      REQUIRE(expr != nullptr);

      auto *type = get_type<BuiltinType>(expr);

      REQUIRE(type->builtin_kind == BuiltinKind::CHAR);
    }
  }
}

TEST_CASE("Semantic initializes array using constant expressions") {
  auto result = analyze(R"(
int main() {
    int arr[3] = {
        1 + 2,
        3 * 4,
        10 - 5
    };
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 3);

  for (const auto &child : init.children) {
    auto *expr =
        dynamic_cast<BinaryExpr *>(child.expr.get());

    REQUIRE(expr != nullptr);

    auto *type = get_type<BuiltinType>(expr);

    REQUIRE(type->builtin_kind == BuiltinKind::INT);
  }
}

TEST_CASE("Semantic initializes array using function calls") {
  auto result = analyze(R"(
int foo() {
    return 1;
}

int main() {
    int arr[2] = {
        foo(),
        foo()
    };

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *decl = get_stmt<VariableDeclarationStmt>(main_fn, 0);

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 2);

  for (const auto &child : init.children) {
    auto *expr =
        dynamic_cast<FunctionCallExpr *>(child.expr.get());

    REQUIRE(expr != nullptr);

    REQUIRE(expr->function_name == "foo");

    auto *type = get_type<BuiltinType>(expr);

    REQUIRE(type->builtin_kind == BuiltinKind::INT);
  }
}

TEST_CASE("Semantic partially initializes integer array") {
  auto result = analyze(R"(
int main() {
    int arr[5] = {1, 2};
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->init.has_value());

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 2);

  auto *first = dynamic_cast<IntLetExpr *>(init.children[0].expr.get());
  auto *second = dynamic_cast<IntLetExpr *>(init.children[1].expr.get());

  REQUIRE(first != nullptr);
  REQUIRE(second != nullptr);

  REQUIRE(first->value == 1);
  REQUIRE(second->value == 2);
}

TEST_CASE("Semantic partially initializes multidimensional array") {
  auto result = analyze(R"(
int main() {
    int matrix[2][3] = {
        {1},
        {2, 3}
    };

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 2);

  REQUIRE(init.children[0].children.size() == 1);
  REQUIRE(init.children[1].children.size() == 2);
}

/*
TODO: Add semantics for struct initializers
TEST_CASE("Semantic initializes array of structs") {
  auto result = analyze(R"(
struct Point {
    int x;
};

int main() {
    struct Point pts[2] = {
        {1},
        {2}
    };

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  REQUIRE(decl->init.has_value());

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 2);

  for (const auto &elem : init.children) {
    REQUIRE_FALSE(elem.is_leaf);
    REQUIRE(elem.children.size() == 1);

    auto *expr =
        dynamic_cast<IntLetExpr *>(elem.children[0].expr.get());

    REQUIRE(expr != nullptr);
  }
}
*/

TEST_CASE("Semantic initializes array of enums") {
  auto result = analyze(R"(
enum Color {
    RED,
    GREEN
};

int main() {
    enum Color colors[2] = {
        RED,
        GREEN
    };

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 2);

  for (const auto &elem : init.children) {
    auto *expr =
        dynamic_cast<IdentifierExpr *>(elem.expr.get());

    REQUIRE(expr != nullptr);

    auto *type = get_type<EnumType>(expr);

    REQUIRE(type->name == "Color");
  }
}

TEST_CASE("Semantic initializes array of pointers") {
  auto result = analyze(R"(
int main() {
    int a;
    int b;

    int *ptrs[2] = {
        &a,
        &b
    };

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 2);

  REQUIRE(decl->init.has_value());

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 2);

  for (const auto &elem : init.children) {
    auto *expr =
        dynamic_cast<UnaryExpr *>(elem.expr.get());

    REQUIRE(expr != nullptr);

    auto *type = get_type<PointerType>(expr);

    auto *base =
        dynamic_cast<BuiltinType *>(type->pointee_type);

    REQUIRE(base != nullptr);
    REQUIRE(base->builtin_kind == BuiltinKind::INT);
  }
}

TEST_CASE("Semantic initializes array from identifiers") {
  auto result = analyze(R"(
int main() {
    int a = 1;
    int b = 2;

    int arr[2] = {a, b};

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 2);

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 2);

  for (const auto &child : init.children) {
    auto *expr = dynamic_cast<IdentifierExpr *>(child.expr.get());

    REQUIRE(expr != nullptr);

    auto *type = get_type<BuiltinType>(expr);

    REQUIRE(type->builtin_kind == BuiltinKind::INT);
  }
}

TEST_CASE("Semantic initializes array from unary expressions") {
  auto result = analyze(R"(
int main() {
    int arr[2] = {-1, +2};

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 0);

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 2);

  for (const auto &child : init.children) {
    auto *expr = dynamic_cast<UnaryExpr *>(child.expr.get());

    REQUIRE(expr != nullptr);

    auto *type = get_type<BuiltinType>(expr);

    REQUIRE(type->builtin_kind == BuiltinKind::INT);
  }
}

TEST_CASE("Semantic initializes array from array accesses") {
  auto result = analyze(R"(
int main() {
    int src[2];

    int dst[2] = {
        src[0],
        src[1]
    };

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 1);

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 2);

  for (const auto &child : init.children) {
    auto *expr = dynamic_cast<ArrayAccessExpr *>(child.expr.get());

    REQUIRE(expr != nullptr);

    auto *type = get_type<BuiltinType>(expr);

    REQUIRE(type->builtin_kind == BuiltinKind::INT);
  }
}

TEST_CASE("Semantic initializes array from member accesses") {
  auto result = analyze(R"(
struct Point {
    int x;
};

int main() {
    struct Point p;

    int arr[1] = {
        p.x
    };

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 1);

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 1);

  auto *expr =
      dynamic_cast<MemberAccessExpr *>(init.children[0].expr.get());

  REQUIRE(expr != nullptr);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic initializes multidimensional array from expressions") {
  auto result = analyze(R"(
int main() {
    int a = 1;

    int matrix[2][2] = {
        {a, a + 1},
        {2, 3}
    };

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *decl = get_stmt<VariableDeclarationStmt>(fn, 1);

  const auto &init = *decl->init;

  REQUIRE(init.children.size() == 2);

  auto *id =
      dynamic_cast<IdentifierExpr *>(init.children[0].children[0].expr.get());

  REQUIRE(id != nullptr);

  auto *binary =
      dynamic_cast<BinaryExpr *>(init.children[0].children[1].expr.get());

  REQUIRE(binary != nullptr);

  auto *type = get_type<BuiltinType>(binary);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}