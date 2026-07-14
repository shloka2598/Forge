#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

#include "types/Types.h"

TEST_CASE("Semantic analyzes simple if statement") {
  auto result = analyze(R"(
int main() {
    int x = 1;

    if (x) {
        x = 2;
    }

    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *if_stmt = get_stmt<IfStmt>(fn, 1);

  REQUIRE(if_stmt->condition != nullptr);

  auto *cond = dynamic_cast<IdentifierExpr *>(if_stmt->condition.get());

  REQUIRE(cond != nullptr);
  REQUIRE(cond->identifier_name == "x");

  auto *type = get_type<BuiltinType>(cond);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(if_stmt->then_body != nullptr);
  REQUIRE(if_stmt->else_body == nullptr);
}

TEST_CASE("Semantic analyzes if else statement") {
  auto result = analyze(R"(
int main() {
    int x = 0;

    if (x) {
        x = 1;
    } else {
        x = 2;
    }

    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *if_stmt = get_stmt<IfStmt>(fn, 1);

  REQUIRE(if_stmt->then_body != nullptr);
  REQUIRE(if_stmt->else_body != nullptr);

  auto *cond = dynamic_cast<IdentifierExpr *>(if_stmt->condition.get());

  REQUIRE(cond != nullptr);

  auto *type = get_type<BuiltinType>(cond);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes binary condition in if") {
  auto result = analyze(R"(
int main() {
    int a = 1;
    int b = 2;

    if (a < b) {
        return 1;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *if_stmt = get_stmt<IfStmt>(fn, 2);

  auto *cond =
      dynamic_cast<BinaryExpr *>(if_stmt->condition.get());

  REQUIRE(cond != nullptr);

  REQUIRE(cond->op == TokenType::SMALLER_THAN);

  auto *type = get_type<BuiltinType>(cond);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes function call condition") {
  auto result = analyze(R"(
int foo() {
    return 1;
}

int main() {
    if (foo()) {
        return 1;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *if_stmt = get_stmt<IfStmt>(main_fn, 0);

  auto *call =
      dynamic_cast<FunctionCallExpr *>(if_stmt->condition.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->function_name == "foo");

  auto *type = get_type<BuiltinType>(call);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic creates scope for if body") {
  auto result = analyze(R"(
int main() {
    if (1) {
        int x = 42;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *if_stmt = get_stmt<IfStmt>(fn, 0);

  REQUIRE(if_stmt->then_body != nullptr);

  auto *decl = get_stmt<VariableDeclarationStmt>(if_stmt->then_body.get(), 0);

  REQUIRE(decl->var_name == "x");

  auto *type = static_cast<BuiltinType *>(decl->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes nested if statements") {
  auto result = analyze(R"(
int main() {
    int x = 1;

    if (x) {
        if (x) {
            x = 2;
        }
    }

    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *outer = get_stmt<IfStmt>(fn, 1);

  REQUIRE(outer->then_body != nullptr);

  auto *inner = get_stmt<IfStmt>(outer->then_body.get(), 0);

  REQUIRE(inner->condition != nullptr);

  auto *cond = dynamic_cast<IdentifierExpr *>(inner->condition.get());

  REQUIRE(cond != nullptr);
  REQUIRE(cond->identifier_name == "x");

  auto *type = get_type<BuiltinType>(cond);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes if statement with empty body") {
  auto result = analyze(R"(
int main() {
    if (1) {
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *stmt = get_stmt<IfStmt>(fn, 0);

  REQUIRE(stmt->then_body != nullptr);

  REQUIRE(stmt->then_body->statements.empty());
}

TEST_CASE("Semantic analyzes empty else body") {
  auto result = analyze(R"(
int main() {
    if (1) {
        int x = 0;
    } else {
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *stmt = get_stmt<IfStmt>(fn, 0);

  REQUIRE(stmt->else_body != nullptr);

  REQUIRE(stmt->else_body->statements.empty());
}

TEST_CASE("Semantic analyzes declaration inside else body") {
  auto result = analyze(R"(
int main() {
    if (0) {
    } else {
        int value = 10;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *stmt = get_stmt<IfStmt>(fn, 0);

  auto *decl =
      get_stmt<VariableDeclarationStmt>(stmt->else_body.get(), 0);

  REQUIRE(decl->var_name == "value");

  auto *type =
      static_cast<BuiltinType *>(decl->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes assignment inside if body") {
  auto result = analyze(R"(
int main() {
    int x = 0;

    if (x) {
        x = x + 1;
    }

    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *stmt = get_stmt<IfStmt>(fn, 1);

  auto *expr_stmt =
      get_stmt<ExpressionStmt>(stmt->then_body.get(), 0);

  auto *assign = get_expr<AssignmentExpr>(expr_stmt);

  auto *type = get_type<BuiltinType>(assign);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes return inside if body") {
  auto result = analyze(R"(
int main() {
    if (1) {
        return 5;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *stmt = get_stmt<IfStmt>(fn, 0);

  auto *ret =
      get_stmt<ReturnStmt>(stmt->then_body.get(), 0);

  auto *expr = get_expr<IntLetExpr>(ret);

  REQUIRE(expr->value == 5);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes simple while loop") {
  auto result = analyze(R"(
int main() {
    int x = 5;

    while (x) {
        x = x - 1;
    }

    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<WhileStmt>(fn, 1);

  auto *cond = dynamic_cast<IdentifierExpr *>(loop->condition.get());

  REQUIRE(cond != nullptr);
  REQUIRE(cond->identifier_name == "x");

  auto *type = get_type<BuiltinType>(cond);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(loop->body != nullptr);
}

TEST_CASE("Semantic analyzes binary condition in while loop") {
  auto result = analyze(R"(
int main() {
    int x = 5;

    while (x > 0) {
        x = x - 1;
    }

    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<WhileStmt>(fn, 1);

  auto *cond = dynamic_cast<BinaryExpr *>(loop->condition.get());

  REQUIRE(cond != nullptr);
  REQUIRE(cond->op == TokenType::GREATER_THAN);

  auto *type = get_type<BuiltinType>(cond);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes function call condition in while loop") {
  auto result = analyze(R"(
int foo() {
    return 1;
}

int main() {
    while (foo()) {
        return 1;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *loop = get_stmt<WhileStmt>(main_fn, 0);

  auto *call = dynamic_cast<FunctionCallExpr *>(loop->condition.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->function_name == "foo");

  auto *type = get_type<BuiltinType>(call);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes variable declaration inside while body") {
  auto result = analyze(R"(
int main() {
    while (1) {
        int value = 10;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<WhileStmt>(fn, 0);

  auto *decl =
      get_stmt<VariableDeclarationStmt>(loop->body.get(), 0);

  REQUIRE(decl->var_name == "value");

  auto *type =
      static_cast<BuiltinType *>(decl->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes nested while loops") {
  auto result = analyze(R"(
int main() {
    int x = 2;

    while (x) {
        while (x) {
            x = x - 1;
        }
    }

    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *outer = get_stmt<WhileStmt>(fn, 1);

  auto *inner =
      get_stmt<WhileStmt>(outer->body.get(), 0);

  auto *cond =
      dynamic_cast<IdentifierExpr *>(inner->condition.get());

  REQUIRE(cond != nullptr);
  REQUIRE(cond->identifier_name == "x");

  auto *type = get_type<BuiltinType>(cond);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes empty while body") {
  auto result = analyze(R"(
int main() {
    while (1) {
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<WhileStmt>(fn, 0);

  REQUIRE(loop->body != nullptr);

  REQUIRE(loop->body->statements.empty());
}

TEST_CASE("Semantic analyzes simple do while loop") {
  auto result = analyze(R"(
int main() {
    int x = 5;

    do {
        x = x - 1;
    } while (x);

    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<DoWhileStmt>(fn, 1);

  auto *cond = dynamic_cast<IdentifierExpr *>(loop->condition.get());

  REQUIRE(cond != nullptr);
  REQUIRE(cond->identifier_name == "x");

  auto *type = get_type<BuiltinType>(cond);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(loop->body != nullptr);
}

TEST_CASE("Semantic analyzes binary condition in do while") {
  auto result = analyze(R"(
int main() {
    int x = 5;

    do {
        x = x - 1;
    } while (x > 0);

    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<DoWhileStmt>(fn, 1);

  auto *cond = dynamic_cast<BinaryExpr *>(loop->condition.get());

  REQUIRE(cond != nullptr);

  REQUIRE(cond->op == TokenType::GREATER_THAN);

  auto *type = get_type<BuiltinType>(cond);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes function call condition in do while") {
  auto result = analyze(R"(
int foo() {
    return 1;
}

int main() {
    do {
    } while (foo());

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *loop = get_stmt<DoWhileStmt>(main_fn, 0);

  auto *call =
      dynamic_cast<FunctionCallExpr *>(loop->condition.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->function_name == "foo");

  auto *type = get_type<BuiltinType>(call);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes declaration inside do while body") {
  auto result = analyze(R"(
int main() {
    do {
        int value = 10;
    } while (0);

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<DoWhileStmt>(fn, 0);

  auto *decl =
      get_stmt<VariableDeclarationStmt>(loop->body.get(), 0);

  REQUIRE(decl->var_name == "value");

  auto *type =
      static_cast<BuiltinType *>(decl->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes nested do while loops") {
  auto result = analyze(R"(
int main() {
    int x = 2;

    do {
        do {
            x = x - 1;
        } while (x);
    } while (0);

    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *outer = get_stmt<DoWhileStmt>(fn, 1);

  auto *inner =
      get_stmt<DoWhileStmt>(outer->body.get(), 0);

  REQUIRE(inner != nullptr);

  auto *cond =
      dynamic_cast<IdentifierExpr *>(inner->condition.get());

  REQUIRE(cond != nullptr);

  REQUIRE(cond->identifier_name == "x");

  auto *type = get_type<BuiltinType>(cond);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes empty do while body") {
  auto result = analyze(R"(
int main() {
    do {
    } while (0);

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<DoWhileStmt>(fn, 0);

  REQUIRE(loop->body != nullptr);

  REQUIRE(loop->body->statements.empty());
}

TEST_CASE("Semantic analyzes simple for loop") {
  auto result = analyze(R"(
int main() {
    int i;

    for (i = 0; i < 10; i = i + 1) {
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 1);

  REQUIRE(loop->init_stmt != nullptr);
  REQUIRE(loop->condition != nullptr);
  REQUIRE(loop->update_stmt != nullptr);
  REQUIRE(loop->body != nullptr);

  auto *cond = dynamic_cast<BinaryExpr *>(loop->condition.get());

  REQUIRE(cond != nullptr);
  REQUIRE(cond->op == TokenType::SMALLER_THAN);

  auto *type = get_type<BuiltinType>(cond);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes declaration initializer in for loop") {
  auto result = analyze(R"(
int main() {

    for (int i = 0; i < 10; i = i + 1) {
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 0);

  auto *decl =
      dynamic_cast<VariableDeclarationStmt *>(loop->init_stmt.get());

  REQUIRE(decl != nullptr);

  REQUIRE(decl->var_name == "i");

  auto *type = static_cast<BuiltinType *>(decl->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes expression initializer in for loop") {
  auto result = analyze(R"(
int main() {
    int i;

    for (i = 0; i < 10; i = i + 1) {
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 1);

  auto *expr_stmt =
      dynamic_cast<ExpressionStmt *>(loop->init_stmt.get());

  REQUIRE(expr_stmt != nullptr);

  auto *assign = get_expr<AssignmentExpr>(expr_stmt);

  auto *type = get_type<BuiltinType>(assign);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes assignment update in for loop") {
  auto result = analyze(R"(
int main() {
    int i;

    for (i = 0; i < 10; i = i + 1) {
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 1);

  auto *expr_stmt =
      dynamic_cast<ExpressionStmt *>(loop->update_stmt.get());

  REQUIRE(expr_stmt != nullptr);

  auto *assign = get_expr<AssignmentExpr>(expr_stmt);

  auto *type = get_type<BuiltinType>(assign);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes prefix increment update in for loop") {
  auto result = analyze(R"(
int main() {
    int i;

    for (i = 0; i < 10; ++i) {
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 1);

  auto *expr_stmt =
      dynamic_cast<ExpressionStmt *>(loop->update_stmt.get());

  REQUIRE(expr_stmt != nullptr);

  auto *inc = get_expr<IncrementExpr>(expr_stmt);

  REQUIRE(inc->is_prefix);
  REQUIRE(inc->is_increment);

  auto *type = get_type<BuiltinType>(inc);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes postfix increment update in for loop") {
  auto result = analyze(R"(
int main() {
    int i;

    for (i = 0; i < 10; i++) {
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 1);

  auto *expr_stmt =
      dynamic_cast<ExpressionStmt *>(loop->update_stmt.get());

  REQUIRE(expr_stmt != nullptr);

  auto *inc = get_expr<IncrementExpr>(expr_stmt);

  REQUIRE_FALSE(inc->is_prefix);
  REQUIRE(inc->is_increment);

  auto *type = get_type<BuiltinType>(inc);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes for loop without initializer") {
  auto result = analyze(R"(
int main() {
    int i = 0;

    for (; i < 10; i++) {
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 1);

  REQUIRE(loop->init_stmt == nullptr);
  REQUIRE(loop->condition != nullptr);
  REQUIRE(loop->update_stmt != nullptr);

  auto *cond = dynamic_cast<BinaryExpr *>(loop->condition.get());

  REQUIRE(cond != nullptr);
  REQUIRE(cond->op == TokenType::SMALLER_THAN);
}

TEST_CASE("Semantic analyzes for loop without condition") {
  auto result = analyze(R"(
int main() {
    int i;

    for (i = 0;; i++) {
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 1);

  REQUIRE(loop->init_stmt != nullptr);
  REQUIRE(loop->condition == nullptr);
  REQUIRE(loop->update_stmt != nullptr);

  auto *expr_stmt =
      dynamic_cast<ExpressionStmt *>(loop->update_stmt.get());

  REQUIRE(expr_stmt != nullptr);

  auto *inc = get_expr<IncrementExpr>(expr_stmt);

  REQUIRE(inc->is_increment);
}

TEST_CASE("Semantic analyzes for loop without update") {
  auto result = analyze(R"(
int main() {
    int i;

    for (i = 0; i < 10;) {
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 1);

  REQUIRE(loop->init_stmt != nullptr);
  REQUIRE(loop->condition != nullptr);
  REQUIRE(loop->update_stmt == nullptr);

  auto *cond = dynamic_cast<BinaryExpr *>(loop->condition.get());

  REQUIRE(cond != nullptr);

  REQUIRE(cond->op == TokenType::SMALLER_THAN);
}

TEST_CASE("Semantic analyzes infinite for loop") {
  auto result = analyze(R"(
int main() {

    for (;;) {
        break;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 0);

  REQUIRE(loop->init_stmt == nullptr);
  REQUIRE(loop->condition == nullptr);
  REQUIRE(loop->update_stmt == nullptr);

  REQUIRE(loop->body != nullptr);

  REQUIRE(loop->body->statements.size() == 1);
}

TEST_CASE("Semantic analyzes for loop with declaration and no update") {
  auto result = analyze(R"(
int main() {

    for (int i = 0; i < 5;) {
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 0);

  auto *decl =
      dynamic_cast<VariableDeclarationStmt *>(loop->init_stmt.get());

  REQUIRE(decl != nullptr);

  REQUIRE(decl->var_name == "i");

  REQUIRE(loop->update_stmt == nullptr);
}

TEST_CASE("Semantic analyzes for loop with declaration only") {
  auto result = analyze(R"(
int main() {

    for (int i = 0;;) {
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 0);

  REQUIRE(loop->condition == nullptr);
  REQUIRE(loop->update_stmt == nullptr);

  auto *decl =
      dynamic_cast<VariableDeclarationStmt *>(loop->init_stmt.get());

  REQUIRE(decl != nullptr);

  REQUIRE(decl->var_name == "i");
}

TEST_CASE("Semantic analyzes declaration inside for body") {
  auto result = analyze(R"(
int main() {

    for (;;) {
        int x = 42;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 0);

  auto *decl =
      get_stmt<VariableDeclarationStmt>(loop->body.get(), 0);

  REQUIRE(decl->var_name == "x");

  auto *type =
      static_cast<BuiltinType *>(decl->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes nested for loops") {
  auto result = analyze(R"(
int main() {

    for (int i = 0; i < 2; i++) {

        for (int j = 0; j < 2; j++) {
        }

    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *outer = get_stmt<ForStmt>(fn, 0);

  auto *inner =
      get_stmt<ForStmt>(outer->body.get(), 0);

  REQUIRE(inner != nullptr);

  auto *decl =
      dynamic_cast<VariableDeclarationStmt *>(inner->init_stmt.get());

  REQUIRE(decl != nullptr);

  REQUIRE(decl->var_name == "j");
}

TEST_CASE("Semantic analyzes if statement inside for body") {
  auto result = analyze(R"(
int main() {

    for (;;) {

        if (1) {
            int x = 5;
        }

    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 0);

  auto *if_stmt =
      get_stmt<IfStmt>(loop->body.get(), 0);

  REQUIRE(if_stmt->then_body != nullptr);

  auto *decl =
      get_stmt<VariableDeclarationStmt>(
          if_stmt->then_body.get(), 0);

  REQUIRE(decl->var_name == "x");
}

TEST_CASE("Semantic analyzes while loop inside for body") {
  auto result = analyze(R"(
int main() {

    for (;;) {

        while (0) {
            int value = 1;
        }

    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 0);

  auto *while_stmt =
      get_stmt<WhileStmt>(loop->body.get(), 0);

  auto *decl =
      get_stmt<VariableDeclarationStmt>(
          while_stmt->body.get(), 0);

  REQUIRE(decl->var_name == "value");
}

TEST_CASE("Semantic analyzes do while inside for body") {
  auto result = analyze(R"(
int main() {

    for (;;) {

        do {
            int value = 10;
        } while (0);

    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 0);

  auto *do_stmt =
      get_stmt<DoWhileStmt>(loop->body.get(), 0);

  auto *decl =
      get_stmt<VariableDeclarationStmt>(
          do_stmt->body.get(), 0);

  REQUIRE(decl->var_name == "value");
}

TEST_CASE("Semantic analyzes return inside for body") {
  auto result = analyze(R"(
int main() {

    for (;;) {
        return 42;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 0);

  auto *ret =
      get_stmt<ReturnStmt>(loop->body.get(), 0);

  auto *expr = get_expr<IntLetExpr>(ret);

  REQUIRE(expr->value == 42);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes simple switch statement") {
  auto result = analyze(R"(
int main() {
    int x = 1;

    switch (x) {
        case 1: {
            break;
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *sw = get_stmt<SwitchStmt>(fn, 1);

  auto *cond = dynamic_cast<IdentifierExpr *>(sw->condition.get());

  REQUIRE(cond != nullptr);
  REQUIRE(cond->identifier_name == "x");

  auto *type = get_type<BuiltinType>(cond);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);

  REQUIRE(sw->cases.size() == 1);
  REQUIRE(sw->default_body == nullptr);
}

TEST_CASE("Semantic analyzes switch with multiple cases") {
  auto result = analyze(R"(
int main() {
    int x = 2;

    switch (x) {
        case 0: {
            break;
        }

        case 1: {
            break;
        }

        case 2: {
            break;
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *sw = get_stmt<SwitchStmt>(fn, 1);

  REQUIRE(sw->cases.size() == 3);

  for (const auto &c : sw->cases) {
    REQUIRE(c.value != nullptr);
    REQUIRE(c.body != nullptr);
  }
}

TEST_CASE("Semantic analyzes switch with default") {
  auto result = analyze(R"(
int main() {
    int x = 0;

    switch (x) {
        case 0: {
            break;
        }

        default: {
            break;
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *sw = get_stmt<SwitchStmt>(fn, 1);

  REQUIRE(sw->cases.size() == 1);

  REQUIRE(sw->default_body != nullptr);
}

TEST_CASE("Semantic analyzes enum switch") {
  auto result = analyze(R"(
enum Color {
    RED,
    GREEN
};

int main() {
    enum Color c = GREEN;

    switch (c) {
        case RED: {
            break;
        }

        case GREEN: {
            break;
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *sw = get_stmt<SwitchStmt>(main_fn, 1);

  auto *cond =
      dynamic_cast<IdentifierExpr *>(sw->condition.get());

  REQUIRE(cond != nullptr);

  auto *type = get_type<EnumType>(cond);

  REQUIRE(type->name == "Color");

  REQUIRE(sw->cases.size() == 2);
}

TEST_CASE("Semantic analyzes binary expression switch condition") {
  auto result = analyze(R"(
int main() {

    switch (1 + 2) {
        case 3: {
            break;
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *sw = get_stmt<SwitchStmt>(fn, 0);

  auto *cond =
      dynamic_cast<BinaryExpr *>(sw->condition.get());

  REQUIRE(cond != nullptr);

  REQUIRE(cond->op == TokenType::PLUS);

  auto *type = get_type<BuiltinType>(cond);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes function call switch condition") {
  auto result = analyze(R"(
int foo() {
    return 1;
}

int main() {

    switch (foo()) {
        case 1: {
            break;
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *main_fn = get_function_decl(result, "main");

  auto *sw = get_stmt<SwitchStmt>(main_fn, 0);

  auto *call =
      dynamic_cast<FunctionCallExpr *>(sw->condition.get());

  REQUIRE(call != nullptr);

  REQUIRE(call->function_name == "foo");

  auto *type = get_type<BuiltinType>(call);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes declaration inside case body") {
  auto result = analyze(R"(
int main() {

    switch (1) {
        case 1: {
            int x = 42;
            break;
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *sw = get_stmt<SwitchStmt>(fn, 0);

  REQUIRE(sw->cases.size() == 1);

  auto *block =
      get_stmt<BlockStmt>(sw->cases[0].body.get(), 0);

  auto *decl =
      get_stmt<VariableDeclarationStmt>(block, 0);

  REQUIRE(decl->var_name == "x");

  auto *type = static_cast<BuiltinType *>(decl->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes assignment inside case body") {
  auto result = analyze(R"(
int main() {
    int x = 0;

    switch (1) {
        case 1: {
            x = 5;
            break;
        }
    }

    return x;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *sw = get_stmt<SwitchStmt>(fn, 1);

  auto *block =
      get_stmt<BlockStmt>(sw->cases[0].body.get(), 0);

  auto *expr_stmt =
      get_stmt<ExpressionStmt>(block, 0);

  auto *assign = get_expr<AssignmentExpr>(expr_stmt);

  auto *type = get_type<BuiltinType>(assign);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes return inside case body") {
  auto result = analyze(R"(
int main() {

    switch (1) {
        case 1: {
            return 42;
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *sw = get_stmt<SwitchStmt>(fn, 0);

  auto *block =
      get_stmt<BlockStmt>(sw->cases[0].body.get(), 0);

  auto *ret =
      get_stmt<ReturnStmt>(block, 0);

  auto *expr = get_expr<IntLetExpr>(ret);

  REQUIRE(expr->value == 42);

  auto *type = get_type<BuiltinType>(expr);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes declaration inside default body") {
  auto result = analyze(R"(
int main() {

    switch (1) {
        default: {
            int value = 7;
            break;
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *sw = get_stmt<SwitchStmt>(fn, 0);

  REQUIRE(sw->default_body != nullptr);

  auto *block =
      get_stmt<BlockStmt>(sw->default_body.get(), 0);

  auto *decl =
      get_stmt<VariableDeclarationStmt>(block, 0);

  REQUIRE(decl->var_name == "value");

  auto *type = static_cast<BuiltinType *>(decl->resolved_type);

  REQUIRE(type->builtin_kind == BuiltinKind::INT);
}

TEST_CASE("Semantic analyzes nested if inside case body") {
  auto result = analyze(R"(
int main() {

    switch (1) {
        case 1: {
            if (1) {
                int x = 1;
            }
            break;
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *sw = get_stmt<SwitchStmt>(fn, 0);

  auto *block =
      get_stmt<BlockStmt>(sw->cases[0].body.get(), 0);

  auto *if_stmt =
      get_stmt<IfStmt>(block, 0);

  auto *decl =
      get_stmt<VariableDeclarationStmt>(if_stmt->then_body.get(), 0);

  REQUIRE(decl->var_name == "x");
}

TEST_CASE("Semantic analyzes nested while inside case body") {
  auto result = analyze(R"(
int main() {

    switch (1) {
        case 1: {
            while (0) {
                int x = 1;
            }
            break;
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *sw = get_stmt<SwitchStmt>(fn, 0);

  auto *block =
      get_stmt<BlockStmt>(sw->cases[0].body.get(), 0);

  auto *loop =
      get_stmt<WhileStmt>(block, 0);

  auto *decl =
      get_stmt<VariableDeclarationStmt>(loop->body.get(), 0);

  REQUIRE(decl->var_name == "x");
}

TEST_CASE("Semantic analyzes nested switch statement") {
  auto result = analyze(R"(
int main() {

    switch (1) {
        case 1: {

            switch (2) {
                case 2: {
                    break;
                }
            }

            break;
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *outer = get_stmt<SwitchStmt>(fn, 0);

  auto *block =
      get_stmt<BlockStmt>(outer->cases[0].body.get(), 0);

  auto *inner =
      get_stmt<SwitchStmt>(block, 0);

  REQUIRE(inner->cases.size() == 1);

  auto *cond =
      dynamic_cast<IntLetExpr *>(inner->condition.get());

  REQUIRE(cond != nullptr);

  REQUIRE(cond->value == 2);
}

TEST_CASE("Semantic analyzes empty switch") {
  auto result = analyze(R"(
int main() {

    switch (1) {
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *sw = get_stmt<SwitchStmt>(fn, 0);

  REQUIRE(sw->cases.empty());

  REQUIRE(sw->default_body == nullptr);
}

TEST_CASE("Semantic analyzes switch with empty case body") {
  auto result = analyze(R"(
int main() {

    switch (1) {
        case 1: {
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *sw = get_stmt<SwitchStmt>(fn, 0);

  REQUIRE(sw->cases.size() == 1);

  auto *block =
      get_stmt<BlockStmt>(sw->cases[0].body.get(), 0);

  REQUIRE(block->statements.empty());
}

TEST_CASE("Semantic analyzes switch with empty default body") {
  auto result = analyze(R"(
int main() {

    switch (1) {
        default: {
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *sw = get_stmt<SwitchStmt>(fn, 0);

  REQUIRE(sw->default_body != nullptr);

  auto *block =
      get_stmt<BlockStmt>(sw->default_body.get(), 0);

  REQUIRE(block->statements.empty());
}

TEST_CASE("Semantic analyzes while inside default body") {
  auto result = analyze(R"(
int main() {

    switch (1) {
        default: {

            while (0) {
                int x = 5;
            }

            break;
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *sw = get_stmt<SwitchStmt>(fn, 0);

  auto *block =
      get_stmt<BlockStmt>(sw->default_body.get(), 0);

  auto *loop =
      get_stmt<WhileStmt>(block, 0);

  auto *decl =
      get_stmt<VariableDeclarationStmt>(loop->body.get(), 0);

  REQUIRE(decl->var_name == "x");
}

TEST_CASE("Semantic analyzes for loop inside case body") {
  auto result = analyze(R"(
int main() {

    switch (1) {
        case 1: {

            for (;;) {
            }

            break;
        }
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *sw = get_stmt<SwitchStmt>(fn, 0);

  auto *block =
      get_stmt<BlockStmt>(sw->cases[0].body.get(), 0);

  auto *loop =
      get_stmt<ForStmt>(block, 0);

  REQUIRE(loop->body != nullptr);

  REQUIRE(loop->body->statements.empty());
}