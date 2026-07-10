#pragma once

#include "./Datatypes.h"

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

enum class TypeQualifier : uint8_t {
  CONST = 1 << 0,
  VOLATILE = 1 << 1,
  RESTRICT = 1 << 2,
  ATOMIC = 1 << 3,
};

enum class StorageClass : uint8_t {
  NONE,
  AUTO,
  REGISTER,
  STATIC,
  EXTERN,
  TYPEDEF
};

enum class FunctionSpecifier : uint8_t {
  NONE,
  INLINE,
  NORETURN
};

struct ParsedType {
  DataType datatype = DataType::INVALID;
  std::string custom_name;

  size_t pointer_depth = 0;

  std::vector<size_t> dimensions;

  using QualifierMask = uint8_t;
  QualifierMask qualifiers = 0;
  std::vector<QualifierMask> pointer_qualifiers;

  StorageClass storage = StorageClass::NONE;
  FunctionSpecifier function_specifier = FunctionSpecifier::NONE;

  bool variable_length_array = false;
  bool flexible_array = false;

  std::string to_string() const;
  void show(int) const;
};

inline void ParsedType::show(int indent) const {

  auto print_indent = [](int n) {
    for (int i = 0; i < n; i++)
      std::cout << "  ";
  };

  print_indent(indent);

  std::cout << "Type : " << to_string() << '\n';
}

inline std::string ParsedType::to_string() const {

  std::stringstream ss;

  switch (storage) {
  case StorageClass::STATIC:
    ss << "static ";
    break;
  case StorageClass::EXTERN:
    ss << "extern ";
    break;
  case StorageClass::REGISTER:
    ss << "register ";
    break;
  case StorageClass::AUTO:
    ss << "auto ";
    break;
  case StorageClass::TYPEDEF:
    ss << "typedef ";
    break;
  default:
    break;
  }

  if (function_specifier == FunctionSpecifier::INLINE)
    ss << "inline ";

  if (qualifiers & static_cast<uint8_t>(TypeQualifier::CONST))
    ss << "const ";

  if (qualifiers & static_cast<uint8_t>(TypeQualifier::VOLATILE))
    ss << "volatile ";

  if (qualifiers & static_cast<uint8_t>(TypeQualifier::RESTRICT))
    ss << "restrict ";

  switch (datatype) {

  case DataType::STRUCT:
    ss << "struct ";
    if (custom_name.empty())
      ss << "<anonymous>";
    else
      ss << custom_name;
    break;

  case DataType::UNION:
    ss << "union ";
    if (custom_name.empty())
      ss << "<anonymous>";
    else
      ss << custom_name;
    break;

  case DataType::ENUM:
    ss << "enum ";
    if (custom_name.empty())
      ss << "<anonymous>";
    else
      ss << custom_name;
    break;

  case DataType::TYPEDEF_NAME:
    ss << custom_name;
    break;

  default:
    ss << datatype;
    break;
  }

  for (size_t i = 0; i < pointer_depth; i++) {

    ss << "*";

    if (i < pointer_qualifiers.size()) {

      auto q = pointer_qualifiers[i];

      if (q & static_cast<uint8_t>(TypeQualifier::CONST))
        ss << " const";

      if (q & static_cast<uint8_t>(TypeQualifier::VOLATILE))
        ss << " volatile";

      if (q & static_cast<uint8_t>(TypeQualifier::RESTRICT))
        ss << " restrict";
    }
  }

  return ss.str();
}