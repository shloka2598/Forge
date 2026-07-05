#pragma once

#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include <vector>

inline void show_tokens(bool debug, const std::vector<Token> &vec) {
  if (!debug) {
    return;
  }

  for (size_t i = 0; i < vec.size(); i++) {
    std::cout << "Token " << i + 1 << " -> " << vec[i].tokentype << ", " << (vec[i].value.has_value() ? (vec[i].value.value()) : ("NULL")) << "\n";
  }
}

inline void show_ast(bool debug, const Program &program) {
  static int count = 1;

  if (!debug) {
    return;
  }

  if (count == 1) {
    std::cout << "\n===== AST =====\n\n";
  } else if (count == 2) {
    std::cout << "\n===== TYPED AST =====\n\n";
  } else if (count == 3) {
    std::cout << "\n===== OPTIMIZED TYPED AST =====\n\n";
  }

  for (const auto &stmt : program.statements) {
    stmt->show_statement();
    std::cout << '\n';
  }

  count++;
}