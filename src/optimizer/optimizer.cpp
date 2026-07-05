#include "./optimizer.h"

#include "./constant_folder.h"
#include "./dead_code_eliminator.h"
// #include "./constant_propagation.h"
// #include "./copy_propagation.h"

Optimizer::Optimizer(Program &program)
    : program(program) {
}

void Optimizer::optimize() {
  ConstantFolder(program).optimize();

  DeadCodeEliminator(program).optimize();
  // ConstantPropagation(program).optimize();
  // CopyPropagation(program).optimize();
}