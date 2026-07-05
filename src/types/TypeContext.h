#pragma once

#include "./Types.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class TypeContext {
private:
  std::vector<std::unique_ptr<Type>> allocated_types;
  std::unordered_map<std::string, StructType *> struct_types;
  std::unordered_map<std::string, UnionType *> union_types;
  std::unordered_map<std::string, EnumType *> enum_types;

public:
  BuiltinType void_type{BuiltinKind::VOID, false, false, false, std::nullopt, 0};
  BuiltinType char_type{BuiltinKind::CHAR, true, false, true, IntegerRank::CHAR, 8};
  BuiltinType uchar_type{BuiltinKind::UCHAR, true, false, false, IntegerRank::CHAR, 8};
  BuiltinType short_type{BuiltinKind::SHORT, true, false, true, IntegerRank::SHORT, 16};
  BuiltinType ushort_type{BuiltinKind::USHORT, true, false, false, IntegerRank::SHORT, 16};
  BuiltinType int_type{BuiltinKind::INT, true, false, true, IntegerRank::INT, 32};
  BuiltinType uint_type{BuiltinKind::UINT, true, false, false, IntegerRank::INT, 32};
  BuiltinType long_type{BuiltinKind::LONG, true, false, true, IntegerRank::LONG, 64};
  BuiltinType ulong_type{BuiltinKind::ULONG, true, false, false, IntegerRank::LONG, 64};
  BuiltinType longlong_type{BuiltinKind::LONGLONG, true, false, true, IntegerRank::LONGLONG, 64};
  BuiltinType ulonglong_type{BuiltinKind::ULONGLONG, true, false, false, IntegerRank::LONGLONG, 64};
  BuiltinType float_type{BuiltinKind::FLOAT, false, true, true, std::nullopt, 32};
  BuiltinType double_type{BuiltinKind::DOUBLE, false, true, true, std::nullopt, 64};
  BuiltinType longdouble_type{BuiltinKind::LONGDOUBLE, false, true, true, std::nullopt, 128};

  ErrorType error_type;

  Type *qualify(Type *, Type::QualifierMask);

  PointerType *get_pointer_type(Type *, Type::QualifierMask qualifiers = 0);
  ArrayType *get_array_type(Type *, size_t, bool variable = false, bool flexible = false);
  FunctionType *get_function_type(Type *, std::vector<Type *>);
  StructType *get_struct_type(const std::string &);
  UnionType *get_union_type(const std::string &);
  EnumType *get_enum_type(const std::string &);

  void rename_struct(StructType *type, const std::string &new_name);
  void rename_union(UnionType *type, const std::string &new_name);
  void rename_enum(EnumType *type, const std::string &new_name);
};