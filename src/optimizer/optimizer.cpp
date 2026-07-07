#include "./optimizer.h"

#include "constant_folder.h"
#include "dead_code_eliminator.h"

// TOD add constant propogation and copy propogation

Optimizer::Optimizer(Program &program)
    : program(program) {
}

void Optimizer::optimize() {
  ConstantFolder(program).optimize();

  DeadCodeEliminator(program).optimize();
}