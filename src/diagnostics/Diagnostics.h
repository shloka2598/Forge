#pragma once

#include <cstddef>
#include <string>

enum class DiagnosticLevel {
  Error,
  Warning,
  Note
};

struct Diagnostic {
  DiagnosticLevel level;

  std::string message;

  size_t line;
  size_t column;
  size_t length;
};