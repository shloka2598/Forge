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
};

class Lexer {
private:
  const std::string _str;
  size_t _index = 0;

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