#include <fstream>
#include <iostream>
#include <sstream>

#include "./common/helper.h"
#include "./lexer/lexer.h"
#include "./optimizer/optimizer.h"
#include "./parser/parser.h"
#include "./semantics/semantics.h"

bool debug = false;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage ./com <filename.qq>" << std::endl;
    return EXIT_FAILURE;
  }

  std::stringstream file_contents;

  {
    std::fstream file{argv[1], std::ios::in};
    file_contents << file.rdbuf();
  }

  std::string contents = file_contents.str();

  Lexer lexer{contents};
  std::vector<Token> tokens = lexer.tokenize();

  show_tokens(debug, tokens);

  Program program;

  if (lexer.had_error()) {
    std::cerr << "Compiler halted due to lexer failure" << std::endl;
    return -1;
  }

  Parser parser{tokens, program};
  parser.parse();

  show_ast(debug, program);

  if (parser.had_error()) {
    std::cerr << "Compiler halted due to parser failure" << std::endl;
    return -2;
  }

  Semantics analyzer(program);

  analyzer.analyze();

  show_ast(debug, program);

  if (analyzer.error_occured()) {
    std::cerr << "Compiler halted due to Semantic failure" << std::endl;
    return -3;
  }
  Optimizer optimizer(program);
  optimizer.optimize();

  show_ast(debug, program);

  return 0;
}