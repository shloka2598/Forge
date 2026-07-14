#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

#include "types/Types.h"

TEST_CASE("Semantic assigns int type to integer array access") {
  auto result = analyze(R"(
int main() {
    int arr[10];
    return arr[3];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::ARRAY_ACCESS);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic assigns float type to float array access") {
  auto result = analyze(R"(
float main() {
    float arr[8];
    return arr[2];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic assigns double type to double array access") {
  auto result = analyze(R"(
double main() {
    double arr[5];
    return arr[1];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic assigns char type to character array access") {
  auto result = analyze(R"(
char main() {
    char arr[32];
    return arr[7];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::CHAR);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic analyzes base and index expressions of array access") {
  auto result = analyze(R"(
int main() {
    int arr[10];
    return arr[5];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  auto *base = dynamic_cast<IdentifierExpr *>(expr->base_expr.get());

  REQUIRE(base != nullptr);
  REQUIRE(base->identifier_name == "arr");

  auto *index = dynamic_cast<IntLetExpr *>(expr->index_expr.get());

  REQUIRE(index != nullptr);
  REQUIRE(index->value == 5);
}

TEST_CASE("Semantic analyzes identifier index expression") {
  auto result = analyze(R"(
int main() {
    int arr[10];
    int i = 4;
    return arr[i];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 2);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  auto *index = dynamic_cast<IdentifierExpr *>(expr->index_expr.get());

  REQUIRE(index != nullptr);
  REQUIRE(index->identifier_name == "i");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic analyzes binary index expression") {
  auto result = analyze(R"(
int main() {
    int arr[10];
    return arr[2 + 3];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  auto *index = dynamic_cast<BinaryExpr *>(expr->index_expr.get());

  REQUIRE(index != nullptr);
  REQUIRE(index->op == TokenType::PLUS);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes function call index expression") {
  auto result = analyze(R"(
int index() {
    return 3;
}

int main() {
    int arr[10];
    return arr[index()];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  auto *index = dynamic_cast<FunctionCallExpr *>(expr->index_expr.get());

  REQUIRE(index != nullptr);
  REQUIRE(index->function_name == "index");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes pointer indexing") {
  auto result = analyze(R"(
int main() {
    int arr[10];
    int *ptr = arr;
    return ptr[6];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 2);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  auto *base = dynamic_cast<IdentifierExpr *>(expr->base_expr.get());

  REQUIRE(base != nullptr);
  REQUIRE(base->identifier_name == "ptr");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic analyzes unary expression as array base") {
  auto result = analyze(R"(
int main() {
    int arr[10];
    int *ptr = arr;
    return (*(&ptr))[2];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 2);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  REQUIRE(expr->base_expr->expr_type() == ExprType::UNARY);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic assigns array type to first dimension access") {
  auto result = analyze(R"(
int main() {
    int matrix[3][4];
    matrix[1];
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *expr_stmt = get_stmt<ExpressionStmt>(main_fn, 1);

  auto *expr = get_expr<ArrayAccessExpr>(expr_stmt);

  auto *type = get_type<ArrayType>(expr);

  REQUIRE(type->size == 4);

  auto *base = dynamic_cast<BuiltinType *>(type->element_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic assigns element type to multidimensional array access") {
  auto result = analyze(R"(
int main() {
    int matrix[3][4];
    return matrix[1][2];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *outer = get_expr<ArrayAccessExpr>(ret);

  auto *type = get_type<BuiltinType>(outer);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(outer->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic builds nested array access expressions") {
  auto result = analyze(R"(
int main() {
    int matrix[3][4];
    return matrix[1][2];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *outer = get_expr<ArrayAccessExpr>(ret);

  auto *inner =
      dynamic_cast<ArrayAccessExpr *>(outer->base_expr.get());

  REQUIRE(inner != nullptr);

  auto *base =
      dynamic_cast<IdentifierExpr *>(inner->base_expr.get());

  REQUIRE(base != nullptr);
  REQUIRE(base->identifier_name == "matrix");
}

TEST_CASE("Semantic assigns pointer type to array of pointers access") {
  auto result = analyze(R"(
int *main() {
    int value;
    int *ptrs[5];
    ptrs[0] = &value;
    return ptrs[0];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 3);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic analyzes dereference of array of pointers") {
  auto result = analyze(R"(
int main() {
    int value = 42;
    int *ptrs[5];
    ptrs[0] = &value;
    return *ptrs[0];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 3);

  auto *expr = get_expr<UnaryExpr>(ret);

  auto *access = dynamic_cast<ArrayAccessExpr *>(expr->right_expr.get());

  REQUIRE(access != nullptr);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes multidimensional array through nested access") {
  auto result = analyze(R"(
int main() {
    int matrix[3][4];
    return matrix[2][1];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *outer = get_expr<ArrayAccessExpr>(ret);

  REQUIRE(outer->base_expr->expr_type() == ExprType::ARRAY_ACCESS);

  auto *type = get_type<BuiltinType>(outer);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(outer->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic analyzes array access through function call") {
  auto result = analyze(R"(
int *foo(int *p) {
    return p;
}

int main() {
    int arr[5];
    return foo(arr)[2];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  REQUIRE(expr->base_expr->expr_type() == ExprType::FUNCTION_CALL);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic analyzes nested array access base") {
  auto result = analyze(R"(
int main() {
    int matrix[2][3];
    return matrix[0][1];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *outer = get_expr<ArrayAccessExpr>(ret);

  REQUIRE(outer->base_expr->expr_type() == ExprType::ARRAY_ACCESS);

  auto *inner =
      dynamic_cast<ArrayAccessExpr *>(outer->base_expr.get());

  REQUIRE(inner != nullptr);

  auto *type = get_type<BuiltinType>(outer);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes array of structs access") {
  auto result = analyze(R"(
struct Point {
    int x;
};

int main() {
    struct Point points[4];
    return points[2].x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *member = get_expr<MemberAccessExpr>(ret);

  REQUIRE(member->base_expr->expr_type() == ExprType::ARRAY_ACCESS);

  auto *access = dynamic_cast<ArrayAccessExpr *>(member->base_expr.get());

  REQUIRE(access != nullptr);

  auto *type = get_type<BuiltinType>(member);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes array access used in binary expression") {
  auto result = analyze(R"(
int main() {
    int arr[5];
    return arr[1] + arr[2];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *binary = get_expr<BinaryExpr>(ret);

  REQUIRE(binary->left_expr->expr_type() == ExprType::ARRAY_ACCESS);
  REQUIRE(binary->right_expr->expr_type() == ExprType::ARRAY_ACCESS);

  auto *type = get_type<BuiltinType>(binary);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

/*
TODO: Add Semantics support for 2[arr] kind of array access

TEST_CASE("Semantic supports commutative array subscript syntax") {
  auto result = analyze(R"(
int main() {
    int arr[5];
    return 2[arr];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::ARRAY_ACCESS);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);

  auto *base = dynamic_cast<IdentifierExpr *>(expr->base_expr.get());
  REQUIRE(base != nullptr);
  REQUIRE(base->identifier_name == "arr");

  auto *index = dynamic_cast<IntLetExpr *>(expr->index_expr.get());
  REQUIRE(index != nullptr);
  REQUIRE(index->value == 2);
}
*/

TEST_CASE("Semantic assigns struct type to array of structs access") {
  auto result = analyze(R"(
struct Point {
    int x;
    int y;
};

struct Point main() {
    struct Point points[4];
    return points[1];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");
  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::ARRAY_ACCESS);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  auto *type = get_type<StructType>(expr);

  REQUIRE(type->name == "Point");

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic assigns enum type to enum array access") {
  auto result = analyze(R"(
enum Color {
    RED,
    GREEN,
    BLUE
};

enum Color main() {
    enum Color colors[3];
    return colors[2];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");
  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  auto *type = get_type<EnumType>(expr);

  REQUIRE(type->name == "Color");

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic analyzes assignment to array element") {
  auto result = analyze(R"(
int main() {
    int arr[5];
    return (arr[3] = 42);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");
  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *assign = get_expr<AssignmentExpr>(ret);

  auto *lhs = dynamic_cast<ArrayAccessExpr *>(assign->lhs.get());

  REQUIRE(lhs != nullptr);

  auto *type = get_type<BuiltinType>(lhs);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes address of array element") {
  auto result = analyze(R"(
int *main() {
    int arr[5];
    return &arr[2];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");
  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *unary = get_expr<UnaryExpr>(ret);

  auto *access =
      dynamic_cast<ArrayAccessExpr *>(unary->right_expr.get());

  REQUIRE(access != nullptr);

  auto *type = get_type<PointerType>(unary);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes array access inside function call") {
  auto result = analyze(R"(
int foo(int x) {
    return x;
}

int main() {
    int arr[5];
    return foo(arr[4]);
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");
  auto *ret = get_stmt<ReturnStmt>(main_fn, 1);

  auto *call = get_expr<FunctionCallExpr>(ret);

  auto *arg = get_argument<ArrayAccessExpr>(call, 0);

  auto *type = get_type<BuiltinType>(arg);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}