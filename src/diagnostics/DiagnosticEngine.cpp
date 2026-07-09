#include "./DiagnosticEngine.h"

#include <iostream>

void DiagnosticEngine::add(const Diagnostic &diagnostic) {
  diagnostics.push_back(diagnostic);
}

bool DiagnosticEngine::has_diagnostic_level(DiagnosticLevel level) const {
  for (const Diagnostic &diagnostic : diagnostics) {
    if (diagnostic.level == level) {
      return true;
    }
  }
  return false;
}

void DiagnosticEngine::error(std::string msg, size_t line, size_t column, size_t length) {
  add({DiagnosticLevel::Error, std::move(msg), line, column, length});
}

void DiagnosticEngine::warning(std::string msg, size_t line, size_t column, size_t length) {
  add({DiagnosticLevel::Warning, std::move(msg), line, column, length});
}

void DiagnosticEngine::note(std::string msg, size_t line, size_t column, size_t length) {
  add({DiagnosticLevel::Note, std::move(msg), line, column, length});
}

void DiagnosticEngine::print(std::ostream &os) const {
  for (const Diagnostic &diagnostic : diagnostics) {
    switch (diagnostic.level) {
    case DiagnosticLevel::Error:
      os << "error";
      break;

    case DiagnosticLevel::Warning:
      os << "warning";
      break;

    case DiagnosticLevel::Note:
      os << "note";
      break;
    }

    os << ": " << diagnostic.message << '\n';
    os << " --> line " << diagnostic.line << ", column " << diagnostic.column << '\n';

    if (diagnostic.length > 0) {
      os << "     length: " << diagnostic.length << '\n';
    }

    os << '\n';
  }
}