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