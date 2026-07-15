#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "Diagnostics.h"

class DiagnosticEngine {
private:
  std::vector<Diagnostic> diagnostics;
  std::string source;
  std::vector<size_t> line_offsets;

public:
  explicit DiagnosticEngine(std::string source);

  void add(const Diagnostic &diagnostic);

  void error(std::string message, size_t line, size_t column, size_t length);
  void warning(std::string message, size_t line, size_t column, size_t length);
  void note(std::string message, size_t line, size_t column, size_t length);

  bool has_diagnostic_level(DiagnosticLevel) const;

  std::string_view get_line(size_t line) const;

  void print(std::ostream &os = std::cerr) const;

  const std::vector<Diagnostic> &get_diagnostics() const {
    return diagnostics;
  }
};