#include "./lexer.h"

#include <cctype>
#include <iostream>
#include <string>

std::ostream &operator<<(std::ostream &os, TokenType type) {
  switch (type) {
  case TokenType::INVALID:
    return os << "INVALID";
  case TokenType::IDENTIFIER:
    return os << "IDENTIFIER";

  case TokenType::INT_LET:
    return os << "INT_LIT";
  case TokenType::DOUBLE_LET:
    return os << "DOUBLE_LIT";
  case TokenType::FLOAT_LET:
    return os << "FLOAT_LIT";
  case TokenType::CHAR_LET:
    return os << "CHAR_LIT";
  case TokenType::STRING_LET:
    return os << "STRING_LIT";

  case TokenType::DATATYPE_VOID:
    return os << "DATATYPE_VOID";
  case TokenType::DATATYPE_CHAR:
    return os << "DATATYPE_CHAR";
  case TokenType::DATATYPE_INT:
    return os << "DATATYPE_INT";
  case TokenType::DATATYPE_FLOAT:
    return os << "DATATYPE_FLOAT";
  case TokenType::DATATYPE_DOUBLE:
    return os << "DATATYPE_DOUBLE";
  case TokenType::SHORT:
    return os << "SHORT";
  case TokenType::LONG:
    return os << "LONG";
  case TokenType::SIGNED:
    return os << "SIGNED";
  case TokenType::UNSIGNED:
    return os << "UNSIGNED";

  case TokenType::CONST:
    return os << "CONST";
  case TokenType::VOLATILE:
    return os << "VOLATILE";
  case TokenType::RESTRICT:
    return os << "RESTRICT";

  case TokenType::STRUCT:
    return os << "STRUCT";
  case TokenType::UNION:
    return os << "UNION";
  case TokenType::ENUM:
    return os << "ENUM";
  case TokenType::TYPEDEF:
    return os << "TYPEDEF";

  case TokenType::AUTO:
    return os << "AUTO";
  case TokenType::EXTERN:
    return os << "EXTERN";
  case TokenType::REGISTER:
    return os << "REGISTER";
  case TokenType::STATIC:
    return os << "STATIC";

  case TokenType::INLINE:
    return os << "INLINE";

  case TokenType::IF:
    return os << "IF";
  case TokenType::ELSE:
    return os << "ELSE";
  case TokenType::SWITCH:
    return os << "SWITCH";
  case TokenType::CASE:
    return os << "CASE";
  case TokenType::DEFAULT:
    return os << "DEFAULT";
  case TokenType::FOR:
    return os << "FOR";
  case TokenType::WHILE:
    return os << "WHILE";
  case TokenType::DO:
    return os << "DO";
  case TokenType::BREAK:
    return os << "BREAK";
  case TokenType::CONTINUE:
    return os << "CONTINUE";
  case TokenType::GOTO:
    return os << "GOTO";
  case TokenType::RETURN:
    return os << "RETURN";

  case TokenType::SIZEOF:
    return os << "SIZEOF";

  case TokenType::PLUS:
    return os << "+";
  case TokenType::MINUS:
    return os << "-";
  case TokenType::MULTIPLY:
    return os << "*";
  case TokenType::DIVIDE:
    return os << "/";
  case TokenType::MODULO:
    return os << "%";
  case TokenType::PLUS_PLUS:
    return os << "++";
  case TokenType::MINUS_MINUS:
    return os << "--";

  case TokenType::EQUALS:
    return os << "=";
  case TokenType::PLUS_EQUALS:
    return os << "+=";
  case TokenType::MINUS_EQUALS:
    return os << "-=";
  case TokenType::MULTIPLY_EQUALS:
    return os << "*=";
  case TokenType::DIVIDE_EQUALS:
    return os << "/=";
  case TokenType::MOD_EQUALS:
    return os << "%=";
  case TokenType::LEFT_SHIFT_EQUALS:
    return os << "<<=";
  case TokenType::RIGHT_SHIFT_EQUALS:
    return os << ">>=";
  case TokenType::AMPERSAND_EQUALS:
    return os << "&=";
  case TokenType::PIPE_EQUALS:
    return os << "|=";
  case TokenType::CARET_EQUALS:
    return os << "^=";

  case TokenType::DOUBLE_EQUALS:
    return os << "==";
  case TokenType::NOT_EQUALS:
    return os << "!=";
  case TokenType::GREATER_THAN:
    return os << ">";
  case TokenType::SMALLER_THAN:
    return os << "<";
  case TokenType::GREATER_THAN_EQUAL_THAN:
    return os << ">=";
  case TokenType::SMALLER_THAN_EQUAL_THAN:
    return os << "<=";

  case TokenType::EXCLAMATION:
    return os << "!";
  case TokenType::DOUBLE_AMPERSAND:
    return os << "&&";
  case TokenType::DOUBLE_PIPE:
    return os << "||";

  case TokenType::AMPERSAND:
    return os << "&";
  case TokenType::PIPE:
    return os << "|";
  case TokenType::CARET:
    return os << "^";
  case TokenType::TILDE:
    return os << "~";
  case TokenType::LEFT_SHIFT:
    return os << "<<";
  case TokenType::RIGHT_SHIFT:
    return os << ">>";

  case TokenType::QUESTION_MARK:
    return os << "?";
  case TokenType::COLON:
    return os << ":";
  case TokenType::DOT:
    return os << ".";
  case TokenType::ARROW:
    return os << "->";

  case TokenType::COMMA:
    return os << ",";
  case TokenType::SEMI_COLON:
    return os << ";";
  case TokenType::ELLIPSIS:
    return os << "...";

  case TokenType::PARENTHESIS_OPEN:
    return os << "(";
  case TokenType::PARENTHESIS_CLOSE:
    return os << ")";
  case TokenType::SQUARE_BRACKETS_OPEN:
    return os << "[";
  case TokenType::SQUARE_BRACKETS_CLOSE:
    return os << "]";
  case TokenType::BRACES_OPEN:
    return os << "{";
  case TokenType::BRACES_CLOSE:
    return os << "}";
  }

  return os << "UNKNOWN_TOKEN";
}

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;

  std::string buffer;

  while (peek().has_value()) {
    if (std::isalpha(static_cast<unsigned char>(peek().value())) || peek().value() == '_') {
      buffer.push_back(consume());
      while (peek().has_value() && (std::isalnum(peek().value()) || peek().value() == '_')) {
        buffer.push_back(consume());
      }

      if (buffer == "return") {
        tokens.push_back({.tokentype = TokenType::RETURN, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "int") {
        tokens.push_back({.tokentype = TokenType::DATATYPE_INT, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "char") {
        tokens.push_back({.tokentype = TokenType::DATATYPE_CHAR, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "double") {
        tokens.push_back({.tokentype = TokenType::DATATYPE_DOUBLE, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "float") {
        tokens.push_back({.tokentype = TokenType::DATATYPE_FLOAT, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "void") {
        tokens.push_back({.tokentype = TokenType::DATATYPE_VOID, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "if") {
        tokens.push_back({.tokentype = TokenType::IF, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "else") {
        tokens.push_back({.tokentype = TokenType::ELSE, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "while") {
        tokens.push_back({.tokentype = TokenType::WHILE, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "do") {
        tokens.push_back({.tokentype = TokenType::DO, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "for") {
        tokens.push_back({.tokentype = TokenType::FOR, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "switch") {
        tokens.push_back({.tokentype = TokenType::SWITCH, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "case") {
        tokens.push_back({.tokentype = TokenType::CASE, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "default") {
        tokens.push_back({.tokentype = TokenType::DEFAULT, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "unsigned") {
        tokens.push_back({.tokentype = TokenType::UNSIGNED, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "signed") {
        tokens.push_back({.tokentype = TokenType::SIGNED, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "break") {
        tokens.push_back({.tokentype = TokenType::BREAK, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "continue") {
        tokens.push_back({.tokentype = TokenType::CONTINUE, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "struct") {
        tokens.push_back({.tokentype = TokenType::STRUCT, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "union") {
        tokens.push_back({.tokentype = TokenType::UNION, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "typedef") {
        tokens.push_back({.tokentype = TokenType::TYPEDEF, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "const") {
        tokens.push_back({.tokentype = TokenType::CONST, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "auto") {
        tokens.push_back({.tokentype = TokenType::AUTO, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "static") {
        tokens.push_back({.tokentype = TokenType::STATIC, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "extern") {
        tokens.push_back({.tokentype = TokenType::EXTERN, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "register") {
        tokens.push_back({.tokentype = TokenType::REGISTER, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "volatile") {
        tokens.push_back({.tokentype = TokenType::VOLATILE, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "restrict") {
        tokens.push_back({.tokentype = TokenType::RESTRICT, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "short") {
        tokens.push_back({.tokentype = TokenType::SHORT, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "long") {
        tokens.push_back({.tokentype = TokenType::LONG, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "enum") {
        tokens.push_back({.tokentype = TokenType::ENUM, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "inline") {
        tokens.push_back({.tokentype = TokenType::INLINE, .value = std::nullopt});
        buffer.clear();
      } else if (buffer == "sizeof") {
        tokens.push_back({.tokentype = TokenType::SIZEOF, .value = std::nullopt});
        buffer.clear();
      } else {
        tokens.push_back({.tokentype = TokenType::IDENTIFIER, .value = buffer});
        buffer.clear();
      }
    } else if (std::isdigit(static_cast<unsigned char>(peek().value()))) {
      buffer.clear();

      bool is_decimal = false;

      buffer.push_back(consume());

      if (buffer == "0" && peek().has_value() && (peek().value() == 'x' || peek().value() == 'X')) {
        buffer.push_back(consume()); // x / X

        if (!peek().has_value() || !std::isxdigit(static_cast<unsigned char>(peek().value()))) {
          has_error = true;
          std::cerr << "Lexer: Invalid hexadecimal literal (" << peek().value() << ")\n";
          break;
        }

        while (peek().has_value() && std::isxdigit(static_cast<unsigned char>(peek().value()))) {
          buffer.push_back(consume());
        }
      }

      else if (buffer == "0" && peek().has_value() && peek().value() >= '0' && peek().value() <= '7') {
        while (peek().has_value() && peek().value() >= '0' && peek().value() <= '7') {
          buffer.push_back(consume());
        }
        if (peek().has_value() && (peek().value() == '8' || peek().value() == '9')) {
          has_error = true;
          std::cerr << "Lexer: Invalid octal literal (" << peek().value() << ")\n";
          break;
        }
      }

      else {
        while (peek().has_value() && std::isdigit(static_cast<unsigned char>(peek().value()))) {
          buffer.push_back(consume());
        }
        if (peek().has_value() && peek().value() == '.' && peek(1).has_value() && std::isdigit(static_cast<unsigned char>(peek(1).value()))) {
          is_decimal = true;
          buffer.push_back(consume()); // .
          while (peek().has_value() && std::isdigit(static_cast<unsigned char>(peek().value()))) {
            buffer.push_back(consume());
          }
        }
      }

      if (is_decimal) {
        if (peek().has_value() && (peek().value() == 'f' || peek().value() == 'F')) {
          consume();
          tokens.push_back({.tokentype = TokenType::FLOAT_LET, .value = buffer});
        } else {
          tokens.push_back({.tokentype = TokenType::DOUBLE_LET, .value = buffer});
        }
      } else {
        tokens.push_back({.tokentype = TokenType::INT_LET, .value = buffer});
      }

      if (peek().has_value() && (std::isalnum(static_cast<unsigned char>(peek().value())) || peek().value() == '_')) {
        has_error = true;
        std::cerr << "Lexer: Invalid numeric literal\n";
        break;
      }

      buffer.clear();
    } else if (peek().value() == ';') {
      tokens.push_back({.tokentype = TokenType::SEMI_COLON, .value = std::nullopt});
      consume();
    } else if (peek().value() == ':') {
      tokens.push_back({.tokentype = TokenType::COLON, .value = std::nullopt});
      consume();
    } else if (peek().value() == ',') {
      tokens.push_back({.tokentype = TokenType::COMMA, .value = std::nullopt});
      consume();
    } else if (std::isspace(static_cast<unsigned char>(peek().value()))) {
      consume();
    } else if (peek().value() == '+') {
      consume(); // first +
      if (peek().has_value() && peek().value() == '+') {
        consume(); // second +
        tokens.push_back({.tokentype = TokenType::PLUS_PLUS, .value = std::nullopt});
      } else if (peek().has_value() && peek().value() == '=') {
        consume(); // =
        tokens.push_back({.tokentype = TokenType::PLUS_EQUALS, .value = std::nullopt});
      } else {
        tokens.push_back({.tokentype = TokenType::PLUS, .value = std::nullopt});
      }
    } else if (peek().value() == '-') {
      consume(); // first -
      if (peek().has_value() && peek().value() == '-') {
        consume(); // second -
        tokens.push_back({.tokentype = TokenType::MINUS_MINUS, .value = std::nullopt});
      } else if (peek().has_value() && peek().value() == '=') {
        consume(); // =
        tokens.push_back({.tokentype = TokenType::MINUS_EQUALS, .value = std::nullopt});
      } else if (peek().has_value() && peek().value() == '>') {
        consume(); // -
        tokens.push_back({.tokentype = TokenType::ARROW, .value = std::nullopt});
      } else {
        tokens.push_back({.tokentype = TokenType::MINUS, .value = std::nullopt});
      }
    } else if (peek().value() == '*') {
      consume(); // *
      if (peek().has_value() && peek().value() == '=') {
        consume(); // =
        tokens.push_back({.tokentype = TokenType::MULTIPLY_EQUALS, .value = std::nullopt});
      } else {
        tokens.push_back({.tokentype = TokenType::MULTIPLY, .value = std::nullopt});
      }
    } else if (peek().value() == '\'') {
      consume(); // opening single quote
      if (!peek().has_value()) {
        has_error = true;
        std::cerr << "Lexer: Expected a character literal" << std::endl;
        break;
      }

      char c = consume();

      if (c == '\'') {
        has_error = true;
        std::cerr << "Lexer: Empty character literal not supported" << std::endl;
        break;
      }

      if (!peek().has_value() || peek().value() != '\'') {
        has_error = true;
        std::cerr << "Lexer: Expected a single-quote(')" << std::endl;
        break;
      }

      consume(); // ending single quote
      tokens.push_back({.tokentype = TokenType::CHAR_LET, .value = std::string(1, c)});
    } else if (peek().value() == '/') {
      consume(); // first /
      if (peek().has_value() && peek().value() == '/') {
        consume(); // second /
        while (peek().has_value() && peek().value() != '\n') {
          consume();
        }
      } else if (peek().has_value() && peek().value() == '*') {
        consume(); // *
        bool found_end = false;
        while (peek().has_value()) {
          if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '/') {
            consume(); // *
            consume(); // /
            found_end = true;
            break;
          }
          consume();
        }
        if (!found_end) {
          has_error = true;
          std::cerr << "Lexer: Unterminated multi-line comment" << std::endl;
        }
      } else if (peek().has_value() && peek().value() == '=') {
        consume(); // =
        tokens.push_back({.tokentype = TokenType::DIVIDE_EQUALS, .value = std::nullopt});
      } else {
        tokens.push_back({.tokentype = TokenType::DIVIDE, .value = std::nullopt});
      }
    } else if (peek().value() == '%') {
      consume(); // %
      if (peek().has_value() && peek().value() == '=') {
        consume(); // =
        tokens.push_back({.tokentype = TokenType::MOD_EQUALS, .value = std::nullopt});
      } else {
        tokens.push_back({.tokentype = TokenType::MODULO, .value = std::nullopt});
      }
    } else if (peek().value() == '(') {
      tokens.push_back({.tokentype = TokenType::PARENTHESIS_OPEN, .value = std::nullopt});
      consume();
    } else if (peek().value() == ')') {
      tokens.push_back({.tokentype = TokenType::PARENTHESIS_CLOSE, .value = std::nullopt});
      consume();
    } else if (peek().value() == '{') {
      tokens.push_back({.tokentype = TokenType::BRACES_OPEN, .value = std::nullopt});
      consume();
    } else if (peek().value() == '}') {
      tokens.push_back({.tokentype = TokenType::BRACES_CLOSE, .value = std::nullopt});
      consume();
    } else if (peek().value() == '[') {
      tokens.push_back({.tokentype = TokenType::SQUARE_BRACKETS_OPEN, .value = std::nullopt});
      consume();
    } else if (peek().value() == ']') {
      tokens.push_back({.tokentype = TokenType::SQUARE_BRACKETS_CLOSE, .value = std::nullopt});
      consume();
    } else if (peek().value() == '?') {
      tokens.push_back({.tokentype = TokenType::QUESTION_MARK, .value = std::nullopt});
      consume();
    } else if (peek().value() == '~') {
      tokens.push_back({.tokentype = TokenType::TILDE, .value = std::nullopt});
      consume(); // ~
    } else if (peek().value() == '^') {
      consume(); // ^
      if (peek().has_value() && peek().value() == '=') {
        consume(); // =
        tokens.push_back({.tokentype = TokenType::CARET_EQUALS, .value = std::nullopt});
      } else {
        tokens.push_back({.tokentype = TokenType::CARET, .value = std::nullopt});
      }
    } else if (peek().value() == '.') {
      consume(); // first .
      if (peek().has_value() && peek(1).has_value() && peek().value() == '.' && peek(1).value() == '.') {
        consume();
        consume();
        tokens.push_back({.tokentype = TokenType::ELLIPSIS, .value = std::nullopt});
      } else {
        tokens.push_back({.tokentype = TokenType::DOT, .value = std::nullopt});
      }
    } else if (peek().value() == '=') {
      consume(); // first =

      if (peek().has_value() && peek().value() == '=') {
        consume(); // second =
        tokens.push_back({.tokentype = TokenType::DOUBLE_EQUALS, .value = std::nullopt});
      } else {
        tokens.push_back({.tokentype = TokenType::EQUALS, .value = std::nullopt});
      }
    } else if (peek().value() == '&') {
      consume(); // first &
      if (peek().has_value() && peek().value() == '&') {
        consume(); // second &
        tokens.push_back({.tokentype = TokenType::DOUBLE_AMPERSAND, .value = std::nullopt});
      } else if (peek().has_value() && peek().value() == '=') {
        consume(); // =
        tokens.push_back({.tokentype = TokenType::AMPERSAND_EQUALS, .value = std::nullopt});
      } else {
        tokens.push_back({.tokentype = TokenType::AMPERSAND, .value = std::nullopt});
      }
    } else if (peek().value() == '|') {
      consume(); // first |

      if (peek().has_value() && peek().value() == '|') {
        consume(); // second |
        tokens.push_back({.tokentype = TokenType::DOUBLE_PIPE, .value = std::nullopt});
      } else if (peek().has_value() && peek().value() == '=') {
        consume(); // =
        tokens.push_back({.tokentype = TokenType::PIPE_EQUALS, .value = std::nullopt});
      } else {
        tokens.push_back({.tokentype = TokenType::PIPE, .value = std::nullopt});
      }
    } else if (peek().value() == '>') {
      consume(); // first >

      if (peek().has_value() && peek().value() == '=') {
        consume(); // =
        tokens.push_back({.tokentype = TokenType::GREATER_THAN_EQUAL_THAN, .value = std::nullopt});
      } else if (peek().has_value() && peek().value() == '>') {
        consume(); // second >
        if (peek().has_value() && peek().value() == '=') {
          consume();
          tokens.push_back({.tokentype = TokenType::RIGHT_SHIFT_EQUALS, .value = std::nullopt});
        } else {
          tokens.push_back({.tokentype = TokenType::RIGHT_SHIFT, .value = std::nullopt});
        }
      } else {
        tokens.push_back({.tokentype = TokenType::GREATER_THAN, .value = std::nullopt});
      }
    } else if (peek().value() == '<') {
      consume(); // first <
      if (peek().has_value() && peek().value() == '=') {
        consume();
        tokens.push_back({.tokentype = TokenType::SMALLER_THAN_EQUAL_THAN, .value = std::nullopt});
      } else if (peek().has_value() && peek().value() == '<') {
        consume(); // second <
        if (peek().has_value() && peek().value() == '=') {
          consume();
          tokens.push_back({.tokentype = TokenType::LEFT_SHIFT_EQUALS, .value = std::nullopt});
        } else {
          tokens.push_back({.tokentype = TokenType::LEFT_SHIFT, .value = std::nullopt});
        }
      } else {
        tokens.push_back({.tokentype = TokenType::SMALLER_THAN, .value = std::nullopt});
      }
    } else if (peek().value() == '"') {
      consume(); // opening "
      std::string value;
      while (peek().has_value() && peek().value() != '"') {
        value.push_back(consume());
      }
      if (!peek().has_value()) {
        has_error = true;
        std::cerr << "Lexer: Unterminated string literal" << std::endl;
        break;
      }
      consume(); // closing "
      tokens.push_back({.tokentype = TokenType::STRING_LET, .value = value});
    } else if (peek().value() == '!') {
      consume(); // !
      if (peek().has_value() && peek().value() == '=') {
        consume(); // =
        tokens.push_back({.tokentype = TokenType::NOT_EQUALS, .value = std::nullopt});
      } else {
        tokens.push_back({.tokentype = TokenType::EXCLAMATION, .value = std::nullopt});
      }
    } else {
      has_error = true;
      std::cerr << "Lexer: Invalid char '" << peek().value() << "'\n";
    }
  }

  _index = 0;
  return tokens;
}

std::optional<char> Lexer::peek(int ahead) const {
  if (_index + ahead >= _str.length()) {
    return std::nullopt;
  } else {
    return _str.at(_index + ahead);
  }
}

char Lexer::consume() {
  return _str.at(_index++);
}