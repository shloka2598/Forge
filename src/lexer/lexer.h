#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "./token.h"

std::ostream &operator<<(std::ostream &os, TokenType type);

struct Token {
  TokenType tokentype;
  std::optional<std::string> value;
  size_t line;
  size_t column;
  size_t length;
};

class Lexer {
private:
  const std::string _str;
  size_t _index = 0;
  size_t current_line = 1;
  size_t current_column = 1;

  bool has_error = false;

  [[nodiscard]] std::optional<char> peek(int ahead = 0) const; // done
  char consume();                                              // done

public:
  Lexer(std::string src) : _str{std::move(src)} {
  }

  std::vector<Token> tokenize();

  bool had_error() const {
    return has_error;
  }
};