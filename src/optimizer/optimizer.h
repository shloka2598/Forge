#pragma once

#include "../ast/program.h"

class Optimizer {
private:
  Program &program;

public:
  explicit Optimizer(Program &program);

  void optimize();
};