#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

TEST_CASE("Semantic analyzes break inside while loop") {
  auto result = analyze(R"(
int main() {
    while (1) {
        break;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<WhileStmt>(fn, 0);

  auto *stmt =
      get_stmt<BreakStmt>(loop->body.get(), 0);

  REQUIRE(stmt != nullptr);
}

TEST_CASE("Semantic analyzes break inside do while loop") {
  auto result = analyze(R"(
int main() {
    do {
        break;
    } while (1);

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<DoWhileStmt>(fn, 0);

  auto *stmt =
      get_stmt<BreakStmt>(loop->body.get(), 0);

  REQUIRE(stmt != nullptr);
}

TEST_CASE("Semantic analyzes break inside for loop") {
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

  auto *stmt =
      get_stmt<BreakStmt>(loop->body.get(), 0);

  REQUIRE(stmt != nullptr);
}

TEST_CASE("Semantic analyzes break inside switch") {
  auto result = analyze(R"(
int main() {

    switch (1) {
        case 1: {
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

  auto *stmt =
      get_stmt<BreakStmt>(block, 0);

  REQUIRE(stmt != nullptr);
}

TEST_CASE("Semantic analyzes break inside default") {
  auto result = analyze(R"(
int main() {

    switch (1) {
        default: {
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

  auto *stmt =
      get_stmt<BreakStmt>(block, 0);

  REQUIRE(stmt != nullptr);
}

TEST_CASE("Semantic analyzes nested break") {
  auto result = analyze(R"(
int main() {

    while (1) {

        if (1) {
            break;
        }

    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<WhileStmt>(fn, 0);

  auto *if_stmt =
      get_stmt<IfStmt>(loop->body.get(), 0);

  auto *stmt =
      get_stmt<BreakStmt>(if_stmt->then_body.get(), 0);

  REQUIRE(stmt != nullptr);
}
