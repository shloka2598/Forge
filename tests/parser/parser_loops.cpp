#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "common/test_utils.h"

TEST_CASE("Parser parses empty while loop") {
  auto result = parse(R"(
    int main() {
      while (condition) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 1);

  auto *while_stmt = dynamic_cast<WhileStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(while_stmt != nullptr);

  REQUIRE(while_stmt->condition->expr_type() == ExprType::IDENTIFIER);

  auto *condition = dynamic_cast<IdentifierExpr *>(while_stmt->condition.get());

  REQUIRE(condition != nullptr);
  REQUIRE(condition->identifier_name == "condition");

  REQUIRE(while_stmt->body != nullptr);
  REQUIRE(while_stmt->body->empty());
}

TEST_CASE("Parser parses while loop with one statement") {
  auto result = parse(R"(
    int main() {
      while (condition) {
        return 10;
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *while_stmt = dynamic_cast<WhileStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(while_stmt != nullptr);

  REQUIRE(while_stmt->body->size() == 1);

  REQUIRE(while_stmt->body->statements[0]->stmt_type() == StmtType::RETURN_STMT);
}

TEST_CASE("Parser parses while loop with multiple statements") {
  auto result = parse(R"(
    int main() {
      while (condition) {
        return 10;
        return 20;
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *while_stmt = dynamic_cast<WhileStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(while_stmt != nullptr);
  REQUIRE(while_stmt->body->size() == 2);
  REQUIRE(while_stmt->body->statements[0]->stmt_type() == StmtType::RETURN_STMT);
  REQUIRE(while_stmt->body->statements[1]->stmt_type() == StmtType::RETURN_STMT);
}

TEST_CASE("Parser parses binary condition in while") {
  auto result = parse(R"(
    int main() {
      while (a + b) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *while_stmt = dynamic_cast<WhileStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(while_stmt != nullptr);
  REQUIRE(while_stmt->condition->expr_type() == ExprType::BINARY);
  auto *binary = dynamic_cast<BinaryExpr *>(while_stmt->condition.get());
  REQUIRE(binary != nullptr);
  REQUIRE(binary->op == TokenType::PLUS);
}

TEST_CASE("Parser parses function call condition in while") {
  auto result = parse(R"(
    int main() {
      while (foo()) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *while_stmt = dynamic_cast<WhileStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(while_stmt != nullptr);
  REQUIRE(while_stmt->condition->expr_type() == ExprType::FUNCTION_CALL);
}

TEST_CASE("Parser parses nested while loops") {
  auto result = parse(R"(
    int main() {
      while (a) {
        while (b) {
        }
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *outer = dynamic_cast<WhileStmt *>(fn->declaration->body->statements[0].get());
  REQUIRE(outer != nullptr);
  REQUIRE(outer->body->size() == 1);

  auto *inner = dynamic_cast<WhileStmt *>(outer->body->statements[0].get());
  REQUIRE(inner != nullptr);

  auto *condition = dynamic_cast<IdentifierExpr *>(inner->condition.get());
  REQUIRE(condition != nullptr);
  REQUIRE(condition->identifier_name == "b");
}

TEST_CASE("Parser parses nested blocks inside while") {
  auto result = parse(R"(
    int main() {
      while (a) {
        {
          return 10;
        }
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *while_stmt = dynamic_cast<WhileStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(while_stmt != nullptr);
  REQUIRE(while_stmt->body->size() == 1);
  REQUIRE(while_stmt->body->statements[0]->stmt_type() == StmtType::BLOCK_STMT);
}

TEST_CASE("Parser parses empty do while loop") {
  auto result = parse(R"(
    int main() {
      do {
      } while (condition);
    }
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 1);

  auto *do_while = dynamic_cast<DoWhileStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(do_while != nullptr);
  REQUIRE(do_while->body != nullptr);
  REQUIRE(do_while->body->empty());
  REQUIRE(do_while->condition->expr_type() == ExprType::IDENTIFIER);

  auto *condition = dynamic_cast<IdentifierExpr *>(do_while->condition.get());

  REQUIRE(condition != nullptr);
  REQUIRE(condition->identifier_name == "condition");
}

TEST_CASE("Parser parses do while loop with one statement") {
  auto result = parse(R"(
    int main() {
      do {
        return 10;
      } while (condition);
    }
  )");

  auto *fn = get_function_decl(result);

  auto *do_while = dynamic_cast<DoWhileStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(do_while != nullptr);
  REQUIRE(do_while->body->size() == 1);
  REQUIRE(do_while->body->statements[0]->stmt_type() == StmtType::RETURN_STMT);
}

TEST_CASE("Parser parses do while loop with multiple statements") {
  auto result = parse(R"(
    int main() {
      do {
        return 10;
        return 20;
      } while (condition);
    }
  )");

  auto *fn = get_function_decl(result);
  auto *do_while = dynamic_cast<DoWhileStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(do_while != nullptr);
  REQUIRE(do_while->body->size() == 2);
}

TEST_CASE("Parser parses binary condition in do while") {
  auto result = parse(R"(
    int main() {
      do {
      } while (a + b);
    }
  )");

  auto *fn = get_function_decl(result);

  auto *do_while = dynamic_cast<DoWhileStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(do_while != nullptr);
  REQUIRE(do_while->condition->expr_type() == ExprType::BINARY);

  auto *binary = dynamic_cast<BinaryExpr *>(do_while->condition.get());

  REQUIRE(binary != nullptr);
  REQUIRE(binary->op == TokenType::PLUS);
}

TEST_CASE("Parser parses function call condition in do while") {
  auto result = parse(R"(
    int main() {
      do {
      } while (foo());
    }
  )");

  auto *fn = get_function_decl(result);

  auto *do_while = dynamic_cast<DoWhileStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(do_while != nullptr);
  REQUIRE(do_while->condition->expr_type() == ExprType::FUNCTION_CALL);
}

TEST_CASE("Parser parses nested do while loops") {
  auto result = parse(R"(
    int main() {
      do {
        do {
        } while (b);
      } while (a);
    }
  )");

  auto *fn = get_function_decl(result);

  auto *outer = dynamic_cast<DoWhileStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(outer != nullptr);

  REQUIRE(outer->body->size() == 1);

  auto *inner = dynamic_cast<DoWhileStmt *>(outer->body->statements[0].get());

  REQUIRE(inner != nullptr);

  auto *condition = dynamic_cast<IdentifierExpr *>(inner->condition.get());

  REQUIRE(condition != nullptr);
  REQUIRE(condition->identifier_name == "b");
}

TEST_CASE("Parser parses nested block inside do while") {
  auto result = parse(R"(
    int main() {
      do {
        {
          return 10;
        }
      } while (condition);
    }
  )");

  auto *fn = get_function_decl(result);

  auto *do_while = dynamic_cast<DoWhileStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(do_while != nullptr);

  REQUIRE(do_while->body->size() == 1);

  REQUIRE(do_while->body->statements[0]->stmt_type() == StmtType::BLOCK_STMT);
}

TEST_CASE("Parser parses empty for loop") {
  auto result = parse(R"(
    int main() {
      for (;;) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *for_stmt = dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);

  REQUIRE(for_stmt->init_stmt == nullptr);
  REQUIRE(for_stmt->condition == nullptr);
  REQUIRE(for_stmt->update_stmt == nullptr);

  REQUIRE(for_stmt->body != nullptr);
  REQUIRE(for_stmt->body->empty());
}

TEST_CASE("Parser parses for loop with init only") {
  auto result = parse(R"(
    int main() {
      for (i = 0;;) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *for_stmt = dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);

  REQUIRE(for_stmt->init_stmt != nullptr);
  REQUIRE(for_stmt->condition == nullptr);
  REQUIRE(for_stmt->update_stmt == nullptr);
}

TEST_CASE("Parser parses for loop with condition only") {
  auto result = parse(R"(
    int main() {
      for (; i < 10;) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *for_stmt = dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);

  REQUIRE(for_stmt->init_stmt == nullptr);
  REQUIRE(for_stmt->condition != nullptr);
  REQUIRE(for_stmt->update_stmt == nullptr);

  REQUIRE(for_stmt->condition->expr_type() == ExprType::BINARY);
}

TEST_CASE("Parser parses for loop with update only") {
  auto result = parse(R"(
    int main() {
      for (;; i++) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *for_stmt = dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);

  REQUIRE(for_stmt->init_stmt == nullptr);
  REQUIRE(for_stmt->condition == nullptr);
  REQUIRE(for_stmt->update_stmt != nullptr);
}

TEST_CASE("Parser parses complete for loop") {
  auto result = parse(R"(
    int main() {
      for (i = 0; i < n; i++) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *for_stmt = dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);

  REQUIRE(for_stmt->init_stmt != nullptr);
  REQUIRE(for_stmt->condition != nullptr);
  REQUIRE(for_stmt->update_stmt != nullptr);

  REQUIRE(for_stmt->body->empty());
}

TEST_CASE("Parser parses for loop with one statement") {
  auto result = parse(R"(
    int main() {
      for (;;) {
        return 10;
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *for_stmt = dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);

  REQUIRE(for_stmt->body->size() == 1);
  REQUIRE(for_stmt->body->statements[0]->stmt_type() == StmtType::RETURN_STMT);
}

TEST_CASE("Parser parses for loop with multiple statements") {
  auto result = parse(R"(
    int main() {
      for (;;) {
        return 10;
        return 20;
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *for_stmt = dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);

  REQUIRE(for_stmt->body->size() == 2);
}

TEST_CASE("Parser parses nested for loops") {
  auto result = parse(R"(
    int main() {
      for (;;) {
        for (;;) {
        }
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *outer = dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(outer != nullptr);

  REQUIRE(outer->body->size() == 1);

  auto *inner = dynamic_cast<ForStmt *>(outer->body->statements[0].get());

  REQUIRE(inner != nullptr);
}

TEST_CASE("Parser parses nested block inside for loop") {
  auto result = parse(R"(
    int main() {
      for (;;) {
        {
          return 10;
        }
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *for_stmt = dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);
  REQUIRE(for_stmt->body->size() == 1);
  REQUIRE(for_stmt->body->statements[0]->stmt_type() == StmtType::BLOCK_STMT);
}

TEST_CASE("Parser parses function call condition in for loop") {
  auto result = parse(R"(
    int main() {
      for (;; foo()) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *for_stmt = dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);

  REQUIRE(for_stmt->update_stmt != nullptr);
}

TEST_CASE("Parser parses variable declaration in for initializer") {
  auto result = parse(R"(
    int main() {
      for (int i = 0;;) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *for_stmt =
      dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);

  REQUIRE(for_stmt->init_stmt != nullptr);

  auto *decl = dynamic_cast<VariableDeclarationStmt *>(for_stmt->init_stmt.get());

  REQUIRE(decl != nullptr);

  REQUIRE(decl->var_name == "i");
  REQUIRE(decl->type.datatype == DataType::INT);

  REQUIRE(decl->expr_ptr != nullptr);
  REQUIRE(decl->expr_ptr->expr_type() == ExprType::INT_LITERAL);
}

TEST_CASE("Parser parses expression initializer in for loop") {
  auto result = parse(R"(
    int main() {
      for (i = 0;;) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *for_stmt = dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);

  auto *expr_stmt = dynamic_cast<ExpressionStmt *>(for_stmt->init_stmt.get());

  REQUIRE(expr_stmt != nullptr);
  REQUIRE(expr_stmt->expr->expr_type() == ExprType::ASSIGNMENT);
}

TEST_CASE("Parser parses assignment update in for loop") {
  auto result = parse(R"(
    int main() {
      for (;; i = i + 1) {
      }
    }
  )");

  auto *fn = get_function_decl(result);
  auto *for_stmt = dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);

  auto *expr_stmt = dynamic_cast<ExpressionStmt *>(for_stmt->update_stmt.get());

  REQUIRE(expr_stmt != nullptr);
  REQUIRE(expr_stmt->expr->expr_type() == ExprType::ASSIGNMENT);
}

TEST_CASE("Parser parses post increment update in for loop") {
  auto result = parse(R"(
    int main() {
      for (;; i++) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *for_stmt = dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);

  auto *expr_stmt = dynamic_cast<ExpressionStmt *>(for_stmt->update_stmt.get());

  REQUIRE(expr_stmt != nullptr);
  REQUIRE(expr_stmt->expr->expr_type() == ExprType::POST_INCREMENT);
}

TEST_CASE("Parser parses function call update in for loop") {
  auto result = parse(R"(
    int main() {
      for (;; foo()) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *for_stmt = dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);

  auto *expr_stmt = dynamic_cast<ExpressionStmt *>(for_stmt->update_stmt.get());

  REQUIRE(expr_stmt != nullptr);
  REQUIRE(expr_stmt->expr->expr_type() == ExprType::FUNCTION_CALL);
}

TEST_CASE("Parser parses binary condition in for loop") {
  auto result = parse(R"(
    int main() {
      for (; i < n;) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *for_stmt =
      dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);

  auto *binary = dynamic_cast<BinaryExpr *>(for_stmt->condition.get());

  REQUIRE(binary != nullptr);
  REQUIRE(binary->op == TokenType::SMALLER_THAN);
}

TEST_CASE("Parser parses simple if statement") {
  auto result = parse(R"(
    int main() {
      if (condition) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *if_stmt = dynamic_cast<IfStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(if_stmt != nullptr);
  REQUIRE(if_stmt->condition->expr_type() == ExprType::IDENTIFIER);

  auto *condition = dynamic_cast<IdentifierExpr *>(if_stmt->condition.get());

  REQUIRE(condition != nullptr);
  REQUIRE(condition->identifier_name == "condition");
  REQUIRE(if_stmt->then_body != nullptr);
  REQUIRE(if_stmt->then_body->empty());
  REQUIRE(if_stmt->else_body == nullptr);
}

TEST_CASE("Parser parses if statement with one statement") {
  auto result = parse(R"(
    int main() {
      if (condition) {
        return 10;
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *if_stmt = dynamic_cast<IfStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(if_stmt != nullptr);

  REQUIRE(if_stmt->then_body->size() == 1);

  REQUIRE(if_stmt->then_body->statements[0]->stmt_type() == StmtType::RETURN_STMT);
}

TEST_CASE("Parser parses if statement with multiple statements") {
  auto result = parse(R"(
    int main() {
      if (condition) {
        return 10;
        return 20;
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *if_stmt = dynamic_cast<IfStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(if_stmt != nullptr);
  REQUIRE(if_stmt->then_body->size() == 2);
}

TEST_CASE("Parser parses binary condition in if") {
  auto result = parse(R"(
    int main() {
      if (a + b) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *if_stmt = dynamic_cast<IfStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(if_stmt != nullptr);

  auto *binary = dynamic_cast<BinaryExpr *>(if_stmt->condition.get());

  REQUIRE(binary != nullptr);
  REQUIRE(binary->op == TokenType::PLUS);
}

TEST_CASE("Parser parses function call condition in if") {
  auto result = parse(R"(
    int main() {
      if (foo()) {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *if_stmt = dynamic_cast<IfStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(if_stmt != nullptr);

  REQUIRE(if_stmt->condition->expr_type() == ExprType::FUNCTION_CALL);
}

TEST_CASE("Parser parses if else statement") {
  auto result = parse(R"(
    int main() {
      if (condition) {
      } else {
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *if_stmt = dynamic_cast<IfStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(if_stmt != nullptr);

  REQUIRE(if_stmt->then_body != nullptr);
  REQUIRE(if_stmt->else_body != nullptr);

  REQUIRE(if_stmt->then_body->empty());
  REQUIRE(if_stmt->else_body->empty());
}

TEST_CASE("Parser parses else body") {
  auto result = parse(R"(
    int main() {
      if (condition) {
      } else {
        return 10;
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *if_stmt = dynamic_cast<IfStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(if_stmt != nullptr);

  REQUIRE(if_stmt->else_body->size() == 1);

  REQUIRE(if_stmt->else_body->statements[0]->stmt_type() == StmtType::RETURN_STMT);
}

TEST_CASE("Parser parses nested if statements") {
  auto result = parse(R"(
    int main() {
      if (a) {
        if (b) {
        }
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *outer = dynamic_cast<IfStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(outer != nullptr);

  REQUIRE(outer->then_body->size() == 1);

  auto *inner = dynamic_cast<IfStmt *>(outer->then_body->statements[0].get());

  REQUIRE(inner != nullptr);

  auto *condition = dynamic_cast<IdentifierExpr *>(inner->condition.get());

  REQUIRE(condition != nullptr);
  REQUIRE(condition->identifier_name == "b");
}

TEST_CASE("Parser parses nested block inside if") {
  auto result = parse(R"(
    int main() {
      if (condition) {
        {
          return 10;
        }
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *if_stmt = dynamic_cast<IfStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(if_stmt != nullptr);

  REQUIRE(if_stmt->then_body->size() == 1);

  REQUIRE(if_stmt->then_body->statements[0]->stmt_type() == StmtType::BLOCK_STMT);
}

TEST_CASE("Parser associates else with nearest if") {
  auto result = parse(R"(
    int main() {
      if (a) {
        if (b) {
        } else {
        }
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *outer = dynamic_cast<IfStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(outer != nullptr);

  auto *inner = dynamic_cast<IfStmt *>(outer->then_body->statements[0].get());

  REQUIRE(inner != nullptr);

  REQUIRE(inner->else_body != nullptr);
  REQUIRE(outer->else_body == nullptr);
}

TEST_CASE("Parser parses break statement inside while") {
  auto result = parse(R"(
    int main() {
      while (1) {
        break;
      }
    }
  )");

  auto *fn = get_function_decl(result);
  auto *while_stmt = dynamic_cast<WhileStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(while_stmt != nullptr);
  REQUIRE(while_stmt->body->size() == 1);
  REQUIRE(while_stmt->body->statements[0]->stmt_type() == StmtType::BREAK_STMT);
}

TEST_CASE("Parser parses break inside switch case") {
  auto result = parse(R"(
    int main() {
      switch (x) {
        case 1:
          break;
      }
    }
  )");

  auto *fn = get_function_decl(result);
  auto *sw = dynamic_cast<SwitchStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(sw != nullptr);
  REQUIRE(sw->cases.size() == 1);
  REQUIRE(sw->cases[0].body->size() == 1);
  REQUIRE(sw->cases[0].body->statements[0]->stmt_type() == StmtType::BREAK_STMT);
}

TEST_CASE("Parser parses continue statement inside while") {
  auto result = parse(R"(
    int main() {
      while (1) {
        continue;
      }
    }
  )");

  auto *fn = get_function_decl(result);
  auto *while_stmt = dynamic_cast<WhileStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(while_stmt != nullptr);
  REQUIRE(while_stmt->body->size() == 1);
  REQUIRE(while_stmt->body->statements[0]->stmt_type() == StmtType::CONTINUE_STMT);
}

TEST_CASE("Parser parses continue inside for") {
  auto result = parse(R"(
    int main() {
      for (;;) {
        continue;
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *for_stmt = dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(for_stmt != nullptr);
  REQUIRE(for_stmt->body->size() == 1);
  REQUIRE(for_stmt->body->statements[0]->stmt_type() == StmtType::CONTINUE_STMT);
}

TEST_CASE("Parser parses empty switch") {
  auto result = parse(R"(
    int main() {
      switch (x) {
      }
    }
  )");

  auto *fn = get_function_decl(result);
  auto *sw = dynamic_cast<SwitchStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(sw != nullptr);
  REQUIRE(sw->condition->expr_type() == ExprType::IDENTIFIER);
  REQUIRE(sw->cases.empty());
  REQUIRE(sw->default_body == nullptr);
}

TEST_CASE("Parser parses switch with one case") {
  auto result = parse(R"(
    int main() {
      switch (x) {
        case 10:
          break;
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *sw = dynamic_cast<SwitchStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(sw != nullptr);
  REQUIRE(sw->cases.size() == 1);

  auto *value = dynamic_cast<IntLetExpr *>(sw->cases[0].value.get());

  REQUIRE(value != nullptr);
  REQUIRE(value->value == 10);
}

TEST_CASE("Parser parses switch with multiple cases") {
  auto result = parse(R"(
    int main() {
      switch (x) {
        case 1:
          break;
        case 2:
          break;
        case 3:
          break;
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *sw = dynamic_cast<SwitchStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(sw != nullptr);
  REQUIRE(sw->cases.size() == 3);
}

TEST_CASE("Parser parses switch with default") {
  auto result = parse(R"(
    int main() {
      switch (x) {
        default:
          break;
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *sw = dynamic_cast<SwitchStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(sw != nullptr);
  REQUIRE(sw->cases.empty());
  REQUIRE(sw->default_body != nullptr);
  REQUIRE(sw->default_body->size() == 1);
  REQUIRE(sw->default_body->statements[0]->stmt_type() == StmtType::BREAK_STMT);
}

TEST_CASE("Parser parses switch with cases and default") {
  auto result = parse(R"(
    int main() {
      switch (x) {
        case 1:
          break;
        case 2:
          break;
        default:
          break;
      }
    }
  )");

  auto *fn = get_function_decl(result);
  auto *sw = dynamic_cast<SwitchStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(sw != nullptr);
  REQUIRE(sw->cases.size() == 2);
  REQUIRE(sw->default_body != nullptr);
}

TEST_CASE("Parser parses binary switch condition") {
  auto result = parse(R"(
    int main() {
      switch (a + b) {
      }
    }
  )");

  auto *fn = get_function_decl(result);
  auto *sw = dynamic_cast<SwitchStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(sw != nullptr);
  REQUIRE(sw->condition->expr_type() == ExprType::BINARY);
}

TEST_CASE("Parser parses function call switch condition") {
  auto result = parse(R"(
    int main() {
      switch (foo()) {
      }
    }
  )");

  auto *fn = get_function_decl(result);
  auto *sw = dynamic_cast<SwitchStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(sw != nullptr);
  REQUIRE(sw->condition->expr_type() == ExprType::FUNCTION_CALL);
}

TEST_CASE("Parser parses multiple statements inside switch case") {
  auto result = parse(R"(
    int main() {
      switch (x) {
        case 1:
          return 1;
          break;
      }
    }
  )");

  auto *fn = get_function_decl(result);

  auto *sw = dynamic_cast<SwitchStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(sw != nullptr);
  REQUIRE(sw->cases[0].body->size() == 2);
  REQUIRE(sw->cases[0].body->statements[0]->stmt_type() == StmtType::RETURN_STMT);
  REQUIRE(sw->cases[0].body->statements[1]->stmt_type() == StmtType::BREAK_STMT);
}