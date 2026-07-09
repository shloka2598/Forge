#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "common/test_utils.h"

TEST_CASE("Lexer reports invalid numeric literals") {
  auto [num, error_msg] = GENERATE(
      std::pair<std::string_view, std::string_view>{"123abc", "Invalid numeric literal"},
      std::pair<std::string_view, std::string_view>{"123_", "Invalid numeric literal"},
      std::pair<std::string_view, std::string_view>{"0x", "Invalid numeric literal"},
      std::pair<std::string_view, std::string_view>{"0X", "Invalid numeric literal"},
      std::pair<std::string_view, std::string_view>{"089", "Invalid octal literal"},
      std::pair<std::string_view, std::string_view>{"078", "Invalid octal literal"});

  DYNAMIC_SECTION("Number = " << num) {
    auto result = tokenize(num);

    REQUIRE(result.diagnostics.get_diagnostics().size() == 1);
    REQUIRE(result.diagnostics.has_diagnostic_level(DiagnosticLevel::Error));

    const auto &diag = result.diagnostics.get_diagnostics()[0];
    REQUIRE(diag.level == DiagnosticLevel::Error);
    REQUIRE(diag.message == "Invalid numeric literal");
    REQUIRE(diag.line == 1);
    REQUIRE(diag.column == 1);
    REQUIRE(diag.length == 3);

    auto &tokens = result.tokens;
    REQUIRE(tokens.empty());
  }
}