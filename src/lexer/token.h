#pragma once

#include <cstdint>

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