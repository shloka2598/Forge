#pragma once

#include "ast/program.h"

class DeadCodeEliminator {
private:
  Program &program;

  bool optimize_stmt(std::unique_ptr<Stmt> &);
  bool optimize_block_stmt(BlockStmt *);

public:
  explicit DeadCodeEliminator(Program &program) : program(program) {};

  bool optimize();
};