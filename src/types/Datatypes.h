#pragma once

#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "../lexer/token.h"

enum class DataType : uint8_t {
  INVALID,
  VOID,

  CHAR,
  UCHAR,

  SHORT,
  USHORT,

  INT,
  UINT,

  LONG,
  ULONG,

  LONGLONG,
  ULONGLONG,

  FLOAT,
  DOUBLE,
  LONGDOUBLE,

  STRUCT,
  UNION,
  ENUM,
  TYPEDEF_NAME
};

inline std::ostream &operator<<(std::ostream &os, DataType type) {
  switch (type) {
  case DataType::VOID:
    return os << "void";
  case DataType::CHAR:
    return os << "char";
  case DataType::UCHAR:
    return os << "unsigned char";
  case DataType::SHORT:
    return os << "short";
  case DataType::USHORT:
    return os << "unsigned short";
  case DataType::INT:
    return os << "int";
  case DataType::UINT:
    return os << "unsigned int";
  case DataType::LONG:
    return os << "long";
  case DataType::LONGLONG:
    return os << "long long";
  case DataType::ULONG:
    return os << "unsigned long";
  case DataType::ULONGLONG:
    return os << "unsigned long long";
  case DataType::FLOAT:
    return os << "float";
  case DataType::DOUBLE:
    return os << "double";
  case DataType::LONGDOUBLE:
    return os << "long double";
  case DataType::STRUCT:
    return os << "struct";
  case DataType::UNION:
    return os << "union";
  case DataType::ENUM:
    return os << "enum";
  case DataType::TYPEDEF_NAME:
    return os << "typedef_name";
  default:
    return os << "invalid";
  }
}

inline DataType token_to_datatype(TokenType token) {
  switch (token) {
  case TokenType::DATATYPE_INT:
    return DataType::INT;

  case TokenType::DATATYPE_CHAR:
    return DataType::CHAR;

  case TokenType::DATATYPE_FLOAT:
    return DataType::FLOAT;

  case TokenType::DATATYPE_DOUBLE:
    return DataType::DOUBLE;

  case TokenType::DATATYPE_VOID:
    return DataType::VOID;
  case TokenType::STRUCT:
    return DataType::STRUCT;
  case TokenType::UNION:
    return DataType::UNION;
  case TokenType::ENUM:
    return DataType::ENUM;
  case TokenType::TYPEDEF:
    return DataType::TYPEDEF_NAME;

  default:
    return DataType::INVALID;
  }
}
