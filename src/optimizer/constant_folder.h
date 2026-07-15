#pragma once

#include "ast/program.h"

class ConstantFolder {
private:
  Program &program;

  bool optimize_stmt(Stmt *);
  bool optimize_expr(std::unique_ptr<Expr> &);
  bool optimize_array_initializer(ArrayInitializer &);

  bool fold_binary_constants(std::unique_ptr<Expr> &);
  bool fold_algebraic(std::unique_ptr<Expr> &);
  bool fold_unary(std::unique_ptr<Expr> &);
  bool fold_conditional(std::unique_ptr<Expr> &);

  bool is_int_literal(const Expr *, int64_t);

  void replace_with_int(std::unique_ptr<Expr> &, int64_t);

public:
  explicit ConstantFolder(Program &_program) : program(_program) {};

  bool optimize();
};