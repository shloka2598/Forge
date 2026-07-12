#pragma once

#include <cstdint>
#include <string_view>

enum class TokenType : uint8_t {
  // Special
  INVALID,    // invalid token
  IDENTIFIER, // identifier

  // Literals
  INT_LET,    // integer literal
  DOUBLE_LET, // double literal
  FLOAT_LET,  // float literal
  CHAR_LET,   // character literal
  STRING_LET, // string literal

  // Primitive data types
  DATATYPE_VOID,   // void
  DATATYPE_CHAR,   // char
  DATATYPE_INT,    // int
  DATATYPE_FLOAT,  // float
  DATATYPE_DOUBLE, // double
  SHORT,           // short
  LONG,            // long
  SIGNED,          // signed
  UNSIGNED,        // unsigned

  // Type qualifiers
  CONST,    // const
  VOLATILE, // volatile
  RESTRICT, // restrict

  // User defined Types
  STRUCT,  // struct
  UNION,   // union
  ENUM,    // enum
  TYPEDEF, // typedef

  // Storage Classes
  AUTO,     // auto
  EXTERN,   // extern
  REGISTER, // register
  STATIC,   // static

  // Function Specifiers
  INLINE, // inline

  // Control Flow
  IF,       // if
  ELSE,     // else
  SWITCH,   // switch
  CASE,     // case
  DEFAULT,  // default
  FOR,      // for
  WHILE,    // while
  DO,       // do
  BREAK,    // break
  CONTINUE, // continue
  GOTO,     // goto
  RETURN,   // return

  // Compile-time Operators
  SIZEOF, // sizeof

  // Arithmetic Operators
  PLUS,        // +
  MINUS,       // -
  MULTIPLY,    // *
  DIVIDE,      // /
  MODULO,      // %
  PLUS_PLUS,   // ++
  MINUS_MINUS, // --

  // Assignment Operators
  EQUALS,             // =
  PLUS_EQUALS,        // +=
  MINUS_EQUALS,       // -=
  MULTIPLY_EQUALS,    // *=
  DIVIDE_EQUALS,      // /=
  MOD_EQUALS,         // %=
  LEFT_SHIFT_EQUALS,  // <<=
  RIGHT_SHIFT_EQUALS, // >>=
  AMPERSAND_EQUALS,   // &=
  PIPE_EQUALS,        // |=
  CARET_EQUALS,       // ^=

  // Comparison Operators
  DOUBLE_EQUALS,           // ==
  NOT_EQUALS,              // !=
  GREATER_THAN,            // >
  SMALLER_THAN,            // <
  GREATER_THAN_EQUAL_THAN, // >=
  SMALLER_THAN_EQUAL_THAN, // <=

  // Logical Operators
  EXCLAMATION,      // !
  DOUBLE_AMPERSAND, // &&
  DOUBLE_PIPE,      // ||

  // Bitwise Operators
  AMPERSAND,   // &
  PIPE,        // |
  CARET,       // ^
  TILDE,       // ~
  LEFT_SHIFT,  // <<
  RIGHT_SHIFT, // >>

  // Miscellaneous Operators
  QUESTION_MARK, // ?
  COLON,         // :
  DOT,           // .
  ARROW,         // ->

  // Punctuation
  COMMA,      // ,
  SEMI_COLON, // ;
  ELLIPSIS,   // ...

  // Delimiters
  PARENTHESIS_OPEN,      // (
  PARENTHESIS_CLOSE,     // )
  SQUARE_BRACKETS_OPEN,  // [
  SQUARE_BRACKETS_CLOSE, // ]
  BRACES_OPEN,           // {
  BRACES_CLOSE,          // }
};

constexpr std::string_view token_name(TokenType token) {
  switch (token) {
  case TokenType::INVALID:
    return "invalid token";
  case TokenType::IDENTIFIER:
    return "identifier";

  case TokenType::INT_LET:
    return "integer literal";
  case TokenType::DOUBLE_LET:
    return "double literal";
  case TokenType::FLOAT_LET:
    return "float literal";
  case TokenType::CHAR_LET:
    return "character literal";
  case TokenType::STRING_LET:
    return "string literal";

  case TokenType::DATATYPE_VOID:
    return "'void'";
  case TokenType::DATATYPE_CHAR:
    return "'char'";
  case TokenType::DATATYPE_INT:
    return "'int'";
  case TokenType::DATATYPE_FLOAT:
    return "'float'";
  case TokenType::DATATYPE_DOUBLE:
    return "'double'";
  case TokenType::SHORT:
    return "'short'";
  case TokenType::LONG:
    return "'long'";
  case TokenType::SIGNED:
    return "'signed'";
  case TokenType::UNSIGNED:
    return "'unsigned'";

  case TokenType::CONST:
    return "'const'";
  case TokenType::VOLATILE:
    return "'volatile'";
  case TokenType::RESTRICT:
    return "'restrict'";

  case TokenType::STRUCT:
    return "'struct'";
  case TokenType::UNION:
    return "'union'";
  case TokenType::ENUM:
    return "'enum'";
  case TokenType::TYPEDEF:
    return "'typedef'";

  case TokenType::AUTO:
    return "'auto'";
  case TokenType::EXTERN:
    return "'extern'";
  case TokenType::REGISTER:
    return "'register'";
  case TokenType::STATIC:
    return "'static'";

  case TokenType::INLINE:
    return "'inline'";

  case TokenType::IF:
    return "'if'";
  case TokenType::ELSE:
    return "'else'";
  case TokenType::SWITCH:
    return "'switch'";
  case TokenType::CASE:
    return "'case'";
  case TokenType::DEFAULT:
    return "'default'";
  case TokenType::FOR:
    return "'for'";
  case TokenType::WHILE:
    return "'while'";
  case TokenType::DO:
    return "'do'";
  case TokenType::BREAK:
    return "'break'";
  case TokenType::CONTINUE:
    return "'continue'";
  case TokenType::GOTO:
    return "'goto'";
  case TokenType::RETURN:
    return "'return'";

  case TokenType::SIZEOF:
    return "'sizeof'";

  case TokenType::PLUS:
    return "'+'";
  case TokenType::MINUS:
    return "'-'";
  case TokenType::MULTIPLY:
    return "'*'";
  case TokenType::DIVIDE:
    return "'/'";
  case TokenType::MODULO:
    return "'%'";
  case TokenType::PLUS_PLUS:
    return "'++'";
  case TokenType::MINUS_MINUS:
    return "'--'";

  case TokenType::EQUALS:
    return "'='";
  case TokenType::PLUS_EQUALS:
    return "'+='";
  case TokenType::MINUS_EQUALS:
    return "'-='";
  case TokenType::MULTIPLY_EQUALS:
    return "'*='";
  case TokenType::DIVIDE_EQUALS:
    return "'/='";
  case TokenType::MOD_EQUALS:
    return "'%='";
  case TokenType::LEFT_SHIFT_EQUALS:
    return "'<<='";
  case TokenType::RIGHT_SHIFT_EQUALS:
    return "'>>='";
  case TokenType::AMPERSAND_EQUALS:
    return "'&='";
  case TokenType::PIPE_EQUALS:
    return "'|='";
  case TokenType::CARET_EQUALS:
    return "'^='";

  case TokenType::DOUBLE_EQUALS:
    return "'=='";
  case TokenType::NOT_EQUALS:
    return "'!='";
  case TokenType::GREATER_THAN:
    return "'>'";
  case TokenType::SMALLER_THAN:
    return "'<'";
  case TokenType::GREATER_THAN_EQUAL_THAN:
    return "'>='";
  case TokenType::SMALLER_THAN_EQUAL_THAN:
    return "'<='";

  case TokenType::EXCLAMATION:
    return "'!'";
  case TokenType::DOUBLE_AMPERSAND:
    return "'&&'";
  case TokenType::DOUBLE_PIPE:
    return "'||'";

  case TokenType::AMPERSAND:
    return "'&'";
  case TokenType::PIPE:
    return "'|'";
  case TokenType::CARET:
    return "'^'";
  case TokenType::TILDE:
    return "'~'";
  case TokenType::LEFT_SHIFT:
    return "'<<'";
  case TokenType::RIGHT_SHIFT:
    return "'>>'";

  case TokenType::QUESTION_MARK:
    return "'?'";
  case TokenType::COLON:
    return "':'";
  case TokenType::DOT:
    return "'.'";
  case TokenType::ARROW:
    return "'->'";

  case TokenType::COMMA:
    return "','";
  case TokenType::SEMI_COLON:
    return "';'";
  case TokenType::ELLIPSIS:
    return "'...'";

  case TokenType::PARENTHESIS_OPEN:
    return "'('";
  case TokenType::PARENTHESIS_CLOSE:
    return "')'";
  case TokenType::SQUARE_BRACKETS_OPEN:
    return "'['";
  case TokenType::SQUARE_BRACKETS_CLOSE:
    return "']'";
  case TokenType::BRACES_OPEN:
    return "'{'";
  case TokenType::BRACES_CLOSE:
    return "'}'";
  }

  return "<unknown token>";
}