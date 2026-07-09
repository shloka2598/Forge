#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "common/test_utils.h"

TEST_CASE("Lexer reports invalid numeric literals") {
  auto [num, error_msg, length] = GENERATE(
      std::tuple<std::string_view, std::string_view, size_t>{"123abc", "Invalid numeric literal", 3},
      std::tuple<std::string_view, std::string_view, size_t>{"123_", "Invalid numeric literal", 3},
      std::tuple<std::string_view, std::string_view, size_t>{"0x", "Invalid numeric literal", 2},
      std::tuple<std::string_view, std::string_view, size_t>{"0X", "Invalid numeric literal", 2},
      std::tuple<std::string_view, std::string_view, size_t>{"089", "Invalid octal literal", 3},
      std::tuple<std::string_view, std::string_view, size_t>{"078", "Invalid octal literal", 3});

  DYNAMIC_SECTION("Number = " << num) {
    auto result = tokenize(num);

    REQUIRE(result.diagnostics.get_diagnostics().size() == 1);
    REQUIRE(result.diagnostics.has_diagnostic_level(DiagnosticLevel::Error));

    const auto &diag = result.diagnostics.get_diagnostics()[0];

    REQUIRE(diag.level == DiagnosticLevel::Error);
    REQUIRE(diag.message == error_msg);
    REQUIRE(diag.line == 1);
    REQUIRE(diag.column == 1);
    REQUIRE(diag.length == length);

    auto &tokens = result.tokens;
    REQUIRE(tokens.empty());
  }
}