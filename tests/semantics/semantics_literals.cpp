#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

#include "types/Types.h"

TEST_CASE("Semantic assigns correct type to integer literal") {
  auto result = analyze(R"(
int main() {
    return 42;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);
  REQUIRE(ret->expr_ptr->expr_type() == ExprType::INT_LITERAL);
  auto *expr = get_expr<IntLetExpr>(ret);

  REQUIRE(expr->value == 42);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(type->is_integer_type());
  REQUIRE(type->is_signed_integer());
  REQUIRE_FALSE(type->is_unsigned());
  REQUIRE_FALSE(type->is_floating_type());
  REQUIRE(type->bit_width == 32);
  REQUIRE(type->rank == IntegerRank::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to zero integer literal") {
  auto result = analyze(R"(
int main() {
    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::INT_LITERAL);

  auto *expr = get_expr<IntLetExpr>(ret);

  REQUIRE(expr->value == 0);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(type->is_integer_type());
  REQUIRE(type->is_signed_integer());
  REQUIRE_FALSE(type->is_unsigned());
  REQUIRE_FALSE(type->is_floating_type());
  REQUIRE(type->bit_width == 32);
  REQUIRE(type->rank == IntegerRank::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to hexadecimal literal") {
  auto result = analyze(R"(
int main() {
    return 0x1234;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::INT_LITERAL);

  auto *expr = get_expr<IntLetExpr>(ret);

  REQUIRE(expr->value == 0x1234);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(type->is_integer_type());
  REQUIRE(type->is_signed_integer());
  REQUIRE_FALSE(type->is_unsigned());
  REQUIRE_FALSE(type->is_floating_type());
  REQUIRE(type->bit_width == 32);
  REQUIRE(type->rank == IntegerRank::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to octal literal") {
  auto result = analyze(R"(
int main() {
    return 077;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);

  REQUIRE(ret->expr_ptr->expr_type() == ExprType::INT_LITERAL);

  auto *expr = get_expr<IntLetExpr>(ret);

  REQUIRE(expr->value == 077);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(type->is_integer_type());
  REQUIRE(type->is_signed_integer());
  REQUIRE_FALSE(type->is_unsigned());
  REQUIRE_FALSE(type->is_floating_type());
  REQUIRE(type->bit_width == 32);
  REQUIRE(type->rank == IntegerRank::INT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to large decimal literal") {
  auto result = analyze(R"(
int main() {
    return 123456789;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);
  REQUIRE(ret->expr_ptr->expr_type() == ExprType::INT_LITERAL);
  auto *expr = get_expr<IntLetExpr>(ret);

  REQUIRE(expr->value == 123456789);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
  REQUIRE(type->is_integer_type());
  REQUIRE(type->is_signed_integer());
  REQUIRE_FALSE(type->is_unsigned());
  REQUIRE_FALSE(type->is_floating_type());
  REQUIRE(type->bit_width == 32);
  REQUIRE(type->rank == IntegerRank::INT);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to float literal") {
  auto result = analyze(R"(
float main() {
    return 3.5f;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);
  REQUIRE(ret->expr_ptr->expr_type() == ExprType::FLOAT_LITERAL);

  auto *expr = get_expr<FloatLetExpr>(ret);

  REQUIRE(expr->value == Catch::Approx(3.5f));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
  REQUIRE_FALSE(type->is_integer_type());
  REQUIRE_FALSE(type->is_signed_integer());
  REQUIRE_FALSE(type->is_unsigned());
  REQUIRE(type->is_floating_type());
  REQUIRE(type->bit_width == 32);
  REQUIRE_FALSE(type->rank.has_value());

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to zero float literal") {
  auto result = analyze(R"(
float main() {
    return 0.0f;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);
  REQUIRE(ret->expr_ptr->expr_type() == ExprType::FLOAT_LITERAL);

  auto *expr = get_expr<FloatLetExpr>(ret);

  REQUIRE(expr->value == Catch::Approx(0.0f));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
  REQUIRE_FALSE(type->is_integer_type());
  REQUIRE_FALSE(type->is_signed_integer());
  REQUIRE_FALSE(type->is_unsigned());
  REQUIRE(type->is_floating_type());
  REQUIRE(type->bit_width == 32);
  REQUIRE_FALSE(type->rank.has_value());

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to double literal") {
  auto result = analyze(R"(
double main() {
    return 3.1415926535;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);
  REQUIRE(ret->expr_ptr->expr_type() == ExprType::DOUBLE_LITERAL);

  auto *expr = get_expr<DoubleLetExpr>(ret);

  REQUIRE(expr->value == Catch::Approx(3.1415926535));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);
  REQUIRE_FALSE(type->is_integer_type());
  REQUIRE_FALSE(type->is_signed_integer());
  REQUIRE_FALSE(type->is_unsigned());
  REQUIRE(type->is_floating_type());
  REQUIRE(type->bit_width == 64);
  REQUIRE_FALSE(type->rank.has_value());

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to zero double literal") {
  auto result = analyze(R"(
double main() {
    return 0.0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);
  REQUIRE(ret->expr_ptr->expr_type() == ExprType::DOUBLE_LITERAL);

  auto *expr = get_expr<DoubleLetExpr>(ret);

  REQUIRE(expr->value == Catch::Approx(0.0));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::DOUBLE);
  REQUIRE_FALSE(type->is_integer_type());
  REQUIRE_FALSE(type->is_signed_integer());
  REQUIRE_FALSE(type->is_unsigned());
  REQUIRE(type->is_floating_type());
  REQUIRE(type->bit_width == 64);
  REQUIRE_FALSE(type->rank.has_value());

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to character literal") {
  auto result = analyze(R"(
char main() {
    return 'A';
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);
  REQUIRE(ret->expr_ptr->expr_type() == ExprType::CHAR_LITERAL);

  auto *expr = get_expr<CharLetExpr>(ret);

  REQUIRE(expr->value == 'A');

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::CHAR);
  REQUIRE(type->is_integer_type());
  REQUIRE(type->is_signed_integer());
  REQUIRE_FALSE(type->is_unsigned());
  REQUIRE_FALSE(type->is_floating_type());
  REQUIRE(type->bit_width == 8);
  REQUIRE(type->rank == IntegerRank::CHAR);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to numeric character literal") {
  auto result = analyze(R"(
char main() {
    return '0';
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);
  REQUIRE(ret->expr_ptr->expr_type() == ExprType::CHAR_LITERAL);

  auto *expr = get_expr<CharLetExpr>(ret);

  REQUIRE(expr->value == '0');

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::CHAR);
  REQUIRE(type->is_integer_type());
  REQUIRE(type->is_signed_integer());
  REQUIRE_FALSE(type->is_unsigned());
  REQUIRE_FALSE(type->is_floating_type());
  REQUIRE(type->bit_width == 8);
  REQUIRE(type->rank == IntegerRank::CHAR);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to string literal") {
  auto result = analyze(R"(
char *main() {
    return "hello";
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);
  REQUIRE(ret->expr_ptr->expr_type() == ExprType::STRING_LITERAL);

  auto *expr = get_expr<StringLiteralExpr>(ret);

  REQUIRE(expr->value == "hello");

  auto *type = get_type<PointerType>(expr);

  REQUIRE(type->pointee_type != nullptr);
  REQUIRE(type->pointee_type->kind == TypeKind::BUILTIN);

  auto *base = static_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base->builtin_kind == BuiltinKind::CHAR);
  REQUIRE(base->bit_width == 8);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to empty string literal") {
  auto result = analyze(R"(
char *main() {
    return "";
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);
  REQUIRE(ret->expr_ptr->expr_type() == ExprType::STRING_LITERAL);

  auto *expr = get_expr<StringLiteralExpr>(ret);

  REQUIRE(expr->value.empty());

  auto *type = get_type<PointerType>(expr);

  REQUIRE(type->pointee_type != nullptr);
  REQUIRE(type->pointee_type->kind == TypeKind::BUILTIN);

  auto *base = static_cast<BuiltinType *>(type->pointee_type);

  REQUIRE(base->builtin_kind == BuiltinKind::CHAR);

  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}

TEST_CASE("Semantic assigns correct type to float literal with uppercase suffix") {
  auto result = analyze(R"(
float main() {
    return 3.5F;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *ret = get_return_stmt(result);
  REQUIRE(ret->expr_ptr->expr_type() == ExprType::FLOAT_LITERAL);

  auto *expr = get_expr<FloatLetExpr>(ret);

  REQUIRE(expr->value == Catch::Approx(3.5f));

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::FLOAT);
  REQUIRE(expr->value_category == ValueCategory::RVALUE);
}