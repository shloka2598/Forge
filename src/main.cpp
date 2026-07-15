#include <fstream>
#include <iostream>
#include <sstream>

#include "common/helper.h"
#include "diagnostics/DiagnosticEngine.h"
#include "lexer/lexer.h"
#include "optimizer/optimizer.h"
#include "parser/parser.h"
#include "semantics/semantics.h"

bool debug = false;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage ./com <filename.qq>" << std::endl;
    return EXIT_FAILURE;
  }

  std::stringstream file_contents;

  {
    std::ifstream file{argv[1]};
    if (!file) {
      std::cerr << "error: could not open '" << argv[1] << "'\n";
      return EXIT_FAILURE;
    }
    file_contents << file.rdbuf();
  }

  std::string contents = file_contents.str();

  DiagnosticEngine diagnostics{contents};

  Lexer lexer{contents, diagnostics};
  std::vector<Token> tokens = lexer.tokenize();

  show_tokens(debug, tokens);

  Program program;

  Parser parser{tokens, program, diagnostics};
  parser.parse();

  show_parsed_ast(debug, program);

  if (diagnostics.has_diagnostic_level(DiagnosticLevel::Error)) {
    diagnostics.print();
    return EXIT_FAILURE;
  }

  Semantics analyzer(program, diagnostics);
  analyzer.analyze();

  if (!diagnostics.get_diagnostics().empty()) {
    diagnostics.print();
  }

  show_typed_ast(debug, program);

  if (diagnostics.has_diagnostic_level(DiagnosticLevel::Error)) {
    return EXIT_FAILURE;
  }

  Optimizer optimizer(program);
  optimizer.optimize();

  show_optimized_ast(debug, program);

  return 0;
}