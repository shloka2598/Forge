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

TEST_CASE("Semantic analyzes continue inside while loop") {
  auto result = analyze(R"(
int main() {

    while (1) {
        continue;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<WhileStmt>(fn, 0);

  auto *stmt =
      get_stmt<ContinueStmt>(loop->body.get(), 0);

  REQUIRE(stmt != nullptr);
}

TEST_CASE("Semantic analyzes continue inside do while loop") {
  auto result = analyze(R"(
int main() {

    do {
        continue;
    } while (1);

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<DoWhileStmt>(fn, 0);

  auto *stmt =
      get_stmt<ContinueStmt>(loop->body.get(), 0);

  REQUIRE(stmt != nullptr);
}

TEST_CASE("Semantic analyzes continue inside for loop") {
  auto result = analyze(R"(
int main() {

    for (;;) {
        continue;
    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 0);

  auto *stmt =
      get_stmt<ContinueStmt>(loop->body.get(), 0);

  REQUIRE(stmt != nullptr);
}

TEST_CASE("Semantic analyzes continue inside nested if") {
  auto result = analyze(R"(
int main() {

    while (1) {

        if (1) {
            continue;
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
      get_stmt<ContinueStmt>(if_stmt->then_body.get(), 0);

  REQUIRE(stmt != nullptr);
}

TEST_CASE("Semantic analyzes continue inside nested for loop") {
  auto result = analyze(R"(
int main() {

    for (;;) {

        for (;;) {
            continue;
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

  auto *stmt =
      get_stmt<ContinueStmt>(inner->body.get(), 0);

  REQUIRE(stmt != nullptr);
}

TEST_CASE("Semantic analyzes continue inside nested while loop") {
  auto result = analyze(R"(
int main() {

    while (1) {

        while (1) {
            continue;
        }

    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *outer = get_stmt<WhileStmt>(fn, 0);

  auto *inner =
      get_stmt<WhileStmt>(outer->body.get(), 0);

  auto *stmt =
      get_stmt<ContinueStmt>(inner->body.get(), 0);

  REQUIRE(stmt != nullptr);
}

TEST_CASE("Semantic analyzes break in switch inside while") {
  auto result = analyze(R"(
int main() {

    while (1) {

        switch (1) {
            case 1: {
                break;
            }
        }

    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<WhileStmt>(fn, 0);

  auto *sw =
      get_stmt<SwitchStmt>(loop->body.get(), 0);

  auto *block =
      get_stmt<BlockStmt>(sw->cases[0].body.get(), 0);

  auto *br =
      get_stmt<BreakStmt>(block, 0);

  REQUIRE(br != nullptr);
}

TEST_CASE("Semantic analyzes break in switch inside for") {
  auto result = analyze(R"(
int main() {

    for (;;) {

        switch (1) {
            case 1: {
                break;
            }
        }

    }

    return 0;
}
)");

  REQUIRE_FALSE(result.semantic_error);

  auto *fn = get_function_decl(result);

  auto *loop = get_stmt<ForStmt>(fn, 0);

  auto *sw =
      get_stmt<SwitchStmt>(loop->body.get(), 0);

  auto *block =
      get_stmt<BlockStmt>(sw->cases[0].body.get(), 0);

  auto *br =
      get_stmt<BreakStmt>(block, 0);

  REQUIRE(br != nullptr);
}

TEST_CASE("Semantic analyzes continue in while inside switch") {
  auto result = analyze(R"(
int main() {

    switch (1) {
        case 1: {

            while (1) {
                continue;
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

  auto *case_block =
      get_stmt<BlockStmt>(sw->cases[0].body.get(), 0);

  auto *loop =
      get_stmt<WhileStmt>(case_block, 0);

  auto *cont =
      get_stmt<ContinueStmt>(loop->body.get(), 0);

  REQUIRE(cont != nullptr);
}

TEST_CASE("Semantic analyzes break in nested switch") {
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

  auto *outer =
      get_stmt<SwitchStmt>(fn, 0);

  auto *outer_block =
      get_stmt<BlockStmt>(outer->cases[0].body.get(), 0);

  auto *inner =
      get_stmt<SwitchStmt>(outer_block, 0);

  auto *inner_block =
      get_stmt<BlockStmt>(inner->cases[0].body.get(), 0);

  auto *br =
      get_stmt<BreakStmt>(inner_block, 0);

  REQUIRE(br != nullptr);
}

TEST_CASE("Semantic analyzes continue in nested loops inside switch") {
  auto result = analyze(R"(
int main() {

    switch (1) {
        case 1: {

            while (1) {

                for (;;) {
                    continue;
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

  auto *sw =
      get_stmt<SwitchStmt>(fn, 0);

  auto *case_block =
      get_stmt<BlockStmt>(sw->cases[0].body.get(), 0);

  auto *while_loop =
      get_stmt<WhileStmt>(case_block, 0);

  auto *for_loop =
      get_stmt<ForStmt>(while_loop->body.get(), 0);

  auto *cont =
      get_stmt<ContinueStmt>(for_loop->body.get(), 0);

  REQUIRE(cont != nullptr);
}