#include "./DiagnosticEngine.h"

#include <utility>

DiagnosticEngine::DiagnosticEngine(std::string source_) : source(std::move(source_)) {
  line_offsets.push_back(0);

  for (size_t i = 0; i < source.size(); ++i) {
    if (source[i] == '\n') {
      line_offsets.push_back(i + 1);
    }
  }
}

std::string_view DiagnosticEngine::get_line(size_t line) const {
  if (line == 0 || line > line_offsets.size()) {
    return {};
  }

  size_t begin = line_offsets[line - 1];

  size_t end;

  if (line == line_offsets.size()) {
    end = source.size();
  } else {
    end = line_offsets[line] - 1; // skip new line
  }

  if (end > begin && source[end - 1] == '\r') {
    --end;
  }

  return std::string_view(source.data() + begin, end - begin);
}

static const char *level_to_string(DiagnosticLevel level) {
  switch (level) {
  case DiagnosticLevel::Error:
    return "error";

  case DiagnosticLevel::Warning:
    return "warning";

  case DiagnosticLevel::Note:
    return "note";
  }

  return "unknown";
}

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
    os << level_to_string(diagnostic.level) << ": " << diagnostic.message << '\n';
    os << " --> " << diagnostic.line << ':' << diagnostic.column << '\n';

    std::string_view line = get_line(diagnostic.line);

    os << diagnostic.line << " | " << line << '\n';
    os << std::string(std::to_string(diagnostic.line).size(), ' ') << " | ";

    for (size_t i = 1; i < diagnostic.column; ++i) {
      os << ' ';
    }

    if (diagnostic.length == 0) {
      os << '^';
    } else {
      os << '^';

      for (size_t i = 1; i < diagnostic.length; ++i) {
        os << '~';
      }
    }

    os << "\n\n";
  }
}