#pragma once

#include "./exprs.h"

#include <iostream>
#include <memory>
#include <vector>

struct ArrayInitializer {
  bool is_leaf = false;
  std::unique_ptr<Expr> expr;
  std::vector<ArrayInitializer> children;

  void show_initializer(int indent = 0) const {
    for (int i = 0; i < indent; i++) {
      std::cout << "  ";
    }
    if (is_leaf) {
      std::cout << "Value\n";
      if (expr) {
        expr->show_expr(indent + 1);
      }
      return;
    }
    std::cout << "InitializerList\n";
    for (const auto &child : children) {
      child.show_initializer(indent + 1);
    }
  }
};