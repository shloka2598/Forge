#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "./ParsedTypes.h"

enum class TypeKind {
  BUILTIN,
  POINTER,
  ARRAY,
  FUNCTION,
  STRUCT,
  UNION,
  ENUM,
  ERROR
};

enum class IntegerRank {
  CHAR,
  SHORT,
  INT,
  LONG,
  LONGLONG
};

struct Type {
  const TypeKind kind;

  using QualifierMask = uint8_t;
  QualifierMask qualifiers = 0;

  explicit Type(TypeKind k) : kind{k} {
  }

  bool qualifiers_equal(const Type *other) const {
    return qualifiers == other->qualifiers;
  }

  virtual bool equals(const Type *other) const = 0;
  virtual std::string to_string() const = 0;

  virtual ~Type() = default;
};

enum class BuiltinKind {
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
  LONGDOUBLE
};

struct BuiltinType : Type {
  BuiltinKind builtin_kind;

  bool is_integer = false;
  bool is_signed = false;
  bool is_floating = false;

  std::optional<IntegerRank> rank;

  uint32_t bit_width = 0;

  BuiltinType(BuiltinKind kind, bool integer, bool floating, bool signedness, std::optional<IntegerRank> r,
              uint32_t bits) : Type(TypeKind::BUILTIN), builtin_kind(kind), is_integer(integer), is_signed(signedness), is_floating(floating), rank(std::move(r)), bit_width(bits) {
  }

  bool equals(const Type *other) const override {
    if (!qualifiers_equal(other))
      return false;
    if (other->kind != TypeKind::BUILTIN)
      return false;

    auto rhs = static_cast<const BuiltinType *>(other);

    return builtin_kind == rhs->builtin_kind;
  }

  bool is_unsigned() const {
    return is_integer && !is_signed;
  }

  bool is_signed_integer() const {
    return is_integer && is_signed;
  }

  bool is_integer_type() const {
    return is_integer;
  }

  bool is_floating_type() const {
    return is_floating;
  }

  virtual std::string to_string() const override {
    switch (builtin_kind) {
    case BuiltinKind::VOID:
      return "void";
    case BuiltinKind::CHAR:
      return "char";
    case BuiltinKind::INT:
      return "int";
    case BuiltinKind::UINT:
      return "unsigned int";
    case BuiltinKind::FLOAT:
      return "float";
    case BuiltinKind::DOUBLE:
      return "double";
    case BuiltinKind::LONGDOUBLE:
      return "long double";
    case BuiltinKind::UCHAR:
      return "unsigned char";
    case BuiltinKind::SHORT:
      return "short";
    case BuiltinKind::USHORT:
      return "unsigned short";
    case BuiltinKind::LONG:
      return "long";
    case BuiltinKind::LONGLONG:
      return "long long";
    case BuiltinKind::ULONGLONG:
      return "unsigned long long";
    case BuiltinKind::ULONG:
      return "unsigned long";
    default:
      return "<builtin>";
    }
  }
};

struct PointerType : Type {
  Type *const pointee_type;

  explicit PointerType(Type *pointee, QualifierMask q = 0) : Type(TypeKind::POINTER), pointee_type(pointee) {
    qualifiers = q;
  }

  bool equals(const Type *other) const override {
    if (!qualifiers_equal(other))
      return false;
    if (other->kind != TypeKind::POINTER)
      return false;

    auto rhs = static_cast<const PointerType *>(other);

    return pointee_type->equals(rhs->pointee_type);
  }

  virtual std::string to_string() const override {
    return pointee_type->to_string() + "*";
  }
};

struct ArrayType : Type {
  Type *const element_type;
  size_t size;

  bool variable_length = false;
  bool flexible = false;

  ArrayType(Type *ele, size_t _size, bool variable = false, bool flex = false) : Type(TypeKind::ARRAY), element_type(ele), size(_size), variable_length(variable), flexible(flex) {
  }

  bool equals(const Type *other) const override {
    if (!qualifiers_equal(other))
      return false;
    if (other->kind != TypeKind::ARRAY)
      return false;

    auto rhs = static_cast<const ArrayType *>(other);

    return size == rhs->size && variable_length == rhs->variable_length && flexible == rhs->flexible && element_type->equals(rhs->element_type);
  }
  virtual std::string to_string() const override {
    std::string dim;

    if (variable_length) {
      dim = "[*]";
    } else if (flexible) {
      dim = "[]";
    } else {
      dim = "[" + std::to_string(size) + "]";
    }

    if (element_type->kind == TypeKind::ARRAY) {
      const auto *arr = static_cast<const ArrayType *>(element_type);
      std::string base = arr->element_type->to_string();
      return base + dim + arr->to_string().substr(base.size());
    }

    return element_type->to_string() + dim;
  }
};

struct FunctionType : Type {
  Type *const return_type;
  std::vector<Type *> parameter_types;

  FunctionType(Type *ret, std::vector<Type *> params) : Type(TypeKind::FUNCTION), return_type(ret), parameter_types(std::move(params)) {
  }

  bool equals(const Type *other) const override {
    if (!qualifiers_equal(other))
      return false;
    if (other->kind != TypeKind::FUNCTION)
      return false;

    auto rhs = static_cast<const FunctionType *>(other);

    if (!return_type->equals(rhs->return_type))
      return false;

    if (parameter_types.size() != rhs->parameter_types.size())
      return false;

    for (size_t i = 0; i < parameter_types.size(); ++i) {
      if (!parameter_types[i]->equals(rhs->parameter_types[i]))
        return false;
    }

    return true;
  }

  virtual std::string to_string() const override {
    std::string s = return_type->to_string() + "(";
    for (size_t i = 0; i < parameter_types.size(); ++i) {
      if (i) {
        s += ", ";
      }
      s += parameter_types[i]->to_string();
    }
    s += ")";
    return s;
  }
};

struct Member {
  std::string name;
  Type *type;
};

struct StructType : Type {
  std::string name;
  std::vector<Member> members;
  bool complete = false;

  explicit StructType(std::string n) : Type(TypeKind::STRUCT), name(std::move(n)) {
  }

  bool equals(const Type *other) const override {
    if (!qualifiers_equal(other))
      return false;
    if (other->kind != TypeKind::STRUCT)
      return false;

    auto rhs = static_cast<const StructType *>(other);

    return name == rhs->name;
  }

  virtual std::string to_string() const override {
    return "struct " + name;
  }
};

struct UnionType : Type {
  std::string name;
  std::vector<Member> members;
  bool complete = false;

  explicit UnionType(std::string n) : Type(TypeKind::UNION), name(std::move(n)) {
  }

  bool equals(const Type *other) const override {
    if (!qualifiers_equal(other))
      return false;
    if (other->kind != TypeKind::UNION)
      return false;

    auto rhs = static_cast<const UnionType *>(other);

    return name == rhs->name;
  }

  virtual std::string to_string() const override {
    return "union " + name;
  }
};

struct EnumConstant {
  std::string name;
  int64_t value;
};

struct EnumType : Type {
  std::string name;
  std::vector<EnumConstant> constants;

  explicit EnumType(std::string n, std::vector<EnumConstant> c = {}) : Type(TypeKind::ENUM), name(std::move(n)), constants(std::move(c)) {
  }

  bool equals(const Type *other) const override {
    if (!qualifiers_equal(other)) {
      return false;
    }

    if (other->kind != TypeKind::ENUM) {
      return false;
    }

    auto rhs = static_cast<const EnumType *>(other);

    return name == rhs->name;
  }

  std::string to_string() const override {
    return "enum " + name;
  }
};

struct ErrorType : Type {
  ErrorType() : Type(TypeKind::ERROR) {
  }

  bool equals(const Type *other) const override {
    if (!qualifiers_equal(other))
      return false;
    return other->kind == TypeKind::ERROR;
  }

  virtual std::string to_string() const override {
    return "<error>";
  }
};
