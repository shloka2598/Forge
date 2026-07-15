#pragma once

#include "ast/program.h"

class DeadCodeEliminator {
private:
  Program &program;

  bool optimize_stmt(std::unique_ptr<Stmt> &);
  bool optimize_block_stmt(BlockStmt *);

  std::unique_ptr<Stmt> make_expression_stmt(std::unique_ptr<Expr>);

public:
  explicit DeadCodeEliminator(Program &program) : program(program) {};

  bool optimize();
};