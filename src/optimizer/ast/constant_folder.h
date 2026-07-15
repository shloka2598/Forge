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
  bool fold_sizeof(std::unique_ptr<Expr> &);
  bool fold_cast(std::unique_ptr<Expr> &);

  bool is_constant_literal(const Expr *);
  bool is_int_literal(const Expr *, int64_t);
  bool is_zero_literal(const Expr *);
  bool is_one_literal(const Expr *);

  int64_t get_int_value(const Expr *);
  double get_double_value(const Expr *);
  float get_float_value(const Expr *);

  void replace_with_expr(std::unique_ptr<Expr> &dst, std::unique_ptr<Expr> src);
  std::unique_ptr<Expr> make_constant_expr(const Token &, Type *, long double);

public:
  explicit ConstantFolder(Program &_program) : program(_program) {};

  bool optimize();
};