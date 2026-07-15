#include "./optimizer.h"

#include "constant_folder.h"
#include "dead_code_eliminator.h"

Optimizer::Optimizer(Program &program)
    : program(program) {
}

void Optimizer::optimize() {
  bool changed;

  do {
    changed = false;
    changed |= ConstantFolder(program).optimize();
    changed |= DeadCodeEliminator(program).optimize();
  } while (changed);
}