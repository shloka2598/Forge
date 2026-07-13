#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

#include "types/Types.h"

TEST_CASE("Semantic resolves local int identifier") {
  auto result = analyze(R"(
int main() {
    int x = 42;
    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 2);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::IDENTIFIER);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "x");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(type->is_integer_type());
  REQUIRE(type->is_signed_integer());
  REQUIRE_FALSE(type->is_unsigned());
  REQUIRE_FALSE(type->is_floating_type());
  REQUIRE(type->bit_width == 32);
  REQUIRE(type->rank == IntegerRank::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves local float identifier") {
  auto result = analyze(R"(
float main() {
    float x = 3.5f;
    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 2);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::IDENTIFIER);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "x");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
  REQUIRE(type->is_floating_type());
  REQUIRE_FALSE(type->is_integer_type());
  REQUIRE(type->bit_width == 32);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves local double identifier") {
  auto result = analyze(R"(
double main() {
    double x = 3.14159;
    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 2);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::IDENTIFIER);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "x");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);
  REQUIRE(type->is_floating_type());
  REQUIRE_FALSE(type->is_integer_type());
  REQUIRE(type->bit_width == 64);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves local char identifier") {
  auto result = analyze(R"(
char main() {
    char c = 'A';
    return c;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 2);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::IDENTIFIER);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "c");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::CHAR);
  REQUIRE(type->is_integer_type());
  REQUIRE(type->is_signed_integer());
  REQUIRE_FALSE(type->is_unsigned());
  REQUIRE_FALSE(type->is_floating_type());
  REQUIRE(type->bit_width == 8);
  REQUIRE(type->rank == IntegerRank::CHAR);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves local pointer identifier") {
  auto result = analyze(R"(
char *main() {
    char *ptr = "hello";
    return ptr;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 2);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::IDENTIFIER);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "ptr");

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::CHAR);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves global variable identifier") {
  auto result = analyze(R"(
int x = 42;

int main() {
    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 1);

  auto *ret = get_stmt<ReturnStmt>(fn, 0);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::IDENTIFIER);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "x");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves function parameter identifier") {
  auto result = analyze(R"(
int foo(int x) {
    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 1);

  auto *ret = get_stmt<ReturnStmt>(fn, 0);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::IDENTIFIER);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "x");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves identifier from enclosing scope") {
  auto result = analyze(R"(
int main() {
    int x = 42;

    {
        return x;
    }
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 2);

  auto *block = get_stmt<BlockStmt>(fn, 1);

  REQUIRE(block->statements.size() == 1);

  auto *ret = dynamic_cast<ReturnStmt *>(block->statements[0].get());

  REQUIRE(ret != nullptr);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::IDENTIFIER);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "x");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves shadowed identifier to innermost declaration") {
  auto result = analyze(R"(
int main() {
    int x = 1;

    {
        int x = 2;
        return x;
    }
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  auto *block = get_stmt<BlockStmt>(fn, 1);

  REQUIRE(block->statements.size() == 2);

  auto *ret = dynamic_cast<ReturnStmt *>(block->statements[1].get());

  REQUIRE(ret != nullptr);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "x");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves global identifier when local declaration does not exist") {
  auto result = analyze(R"(
int global = 42;

int main() {
    return global;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  auto *ret = get_stmt<ReturnStmt>(fn, 0);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "global");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves identifier through multiple nested scopes") {
  auto result = analyze(R"(
int main() {
    int x = 42;

    {
        {
            {
                return x;
            }
        }
    }
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);
  REQUIRE(fn != nullptr);

  auto *block1 = get_stmt<BlockStmt>(fn, 1);
  auto *block2 = get_stmt<BlockStmt>(block1, 0);
  auto *block3 = get_stmt<BlockStmt>(block2, 0);

  auto *ret = get_stmt<ReturnStmt>(block3, 0);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::IDENTIFIER);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "x");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic local variable shadows global variable") {
  auto result = analyze(R"(
int x = 1;

int main() {
    int x = 2;
    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "x");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic local variable shadows function parameter") {
  auto result = analyze(R"(
int foo(int x) {
    int x = 5;
    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "x");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic nested block variable shadows function parameter") {
  auto result = analyze(R"(
int foo(int x) {
    {
        int x = 10;
        return x;
    }
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  auto *block = get_stmt<BlockStmt>(fn, 0);

  auto *ret = get_stmt<ReturnStmt>(block, 1);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "x");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves global identifier inside nested block") {
  auto result = analyze(R"(
int global = 123;

int main() {
    {
        {
            return global;
        }
    }
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  auto *block1 = get_stmt<BlockStmt>(fn, 0);
  auto *block2 = get_stmt<BlockStmt>(block1, 0);

  auto *ret = get_stmt<ReturnStmt>(block2, 0);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "global");

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves array identifier") {
  auto result = analyze(R"(
int main() {
    int arr[10];
    return arr[0];
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::ARRAY_ACCESS);

  auto *expr = get_expr<ArrayAccessExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves pointer identifier") {
  auto result = analyze(R"(
int main() {
    int value = 5;
    int *ptr = &value;
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

TEST_CASE("Semantic resolves struct variable identifier") {
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

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 1);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::MEMBER_ACCESS);

  auto *expr = get_expr<MemberAccessExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves pointer member access") {
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

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::MEMBER_ACCESS);

  auto *expr = get_expr<MemberAccessExpr>(ret);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic resolves enum constant identifier") {
  auto result = analyze(R"(
enum Color {
    RED,
    GREEN
};

int main() {
    return RED;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *ret = get_stmt<ReturnStmt>(fn, 0);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::IDENTIFIER);

  auto *expr = get_expr<IdentifierExpr>(ret);

  REQUIRE(expr->identifier_name == "RED");

  auto *type = get_type<EnumType>(expr);

  REQUIRE(type->name == "Color");
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns array type to array identifier") {
  auto result = analyze(R"(
int main() {
    int arr[10];
    arr;
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *expr_stmt = get_stmt<ExpressionStmt>(fn, 1);

  REQUIRE(expr_stmt->expr->expr_type() == ExprType::IDENTIFIER);

  auto *expr = dynamic_cast<IdentifierExpr *>(expr_stmt->expr.get());

  REQUIRE(expr != nullptr);
  REQUIRE(expr->identifier_name == "arr");

  auto *type = get_type<ArrayType>(expr);

  REQUIRE(type->size == 10);

  auto *base = dynamic_cast<BuiltinType *>(type->element_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic assigns pointer type to pointer identifier") {
  auto result = analyze(R"(
int main() {
    int *ptr;
    ptr;
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *expr_stmt = get_stmt<ExpressionStmt>(fn, 1);

  auto *expr = get_expr<IdentifierExpr>(expr_stmt);

  auto *type = get_type<PointerType>(expr);

  auto *base = dynamic_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base != nullptr);
  REQUIRE(base->builtin_kind == BuiltinKind::INT);

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic assigns struct type to struct identifier") {
  auto result = analyze(R"(
struct Point {
    int x;
};

int main() {
    struct Point p;
    p;
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *expr_stmt = get_stmt<ExpressionStmt>(fn, 1);

  auto *expr = get_expr<IdentifierExpr>(expr_stmt);

  auto *type = get_type<StructType>(expr);

  REQUIRE(type->name == "Point");

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic assigns union type to union identifier") {
  auto result = analyze(R"(
union Data {
    int x;
    float y;
};

int main() {
    union Data d;
    d;
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *expr_stmt = get_stmt<ExpressionStmt>(fn, 1);

  auto *expr = get_expr<IdentifierExpr>(expr_stmt);

  auto *type = get_type<UnionType>(expr);

  REQUIRE(type->name == "Data");

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}

TEST_CASE("Semantic assigns enum type to enum variable identifier") {
  auto result = analyze(R"(
enum Color {
    RED,
    GREEN
};

int main() {
    enum Color c = RED;
    c;
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *expr_stmt = get_stmt<ExpressionStmt>(fn, 1);

  auto *expr = get_expr<IdentifierExpr>(expr_stmt);

  auto *type = get_type<EnumType>(expr);

  REQUIRE(type->name == "Color");

  REQUIRE(expr->value_category == ValueCategory::LVALUE);
}