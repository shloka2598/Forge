#include "./type_utils.h"

size_t sizeof_type(Type *type) {
  switch (type->kind) {
  case TypeKind::BUILTIN: {
    auto *b = static_cast<BuiltinType *>(type);
    return b->bit_width / 8;
  }

  case TypeKind::POINTER:
    return 8;

  case TypeKind::ENUM:
    return 4; //

  case TypeKind::ARRAY: {
    auto *a = static_cast<ArrayType *>(type);
    return a->size * sizeof_type(a->element_type);
  }

  case TypeKind::STRUCT: {
    auto *s = static_cast<StructType *>(type);

    size_t size = 0;

    for (auto &member : s->members)
      size += sizeof_type(member.type);

    return size;
  }

  case TypeKind::UNION: {
    auto *u = static_cast<UnionType *>(type);

    size_t max_size = 0;

    for (auto &member : u->members) {
      max_size = std::max(max_size, sizeof_type(member.type));
    }

    return max_size;
  }

  default:
    return 0;
  }
}

bool is_integer_type(Type *type) {
  if (type->kind == TypeKind::ENUM) {
    return true;
  }

  if (type->kind != TypeKind::BUILTIN) {
    return false;
  }

  return static_cast<BuiltinType *>(type)->is_integer_type();
}

bool is_floating_type(Type *type) {
  if (type->kind != TypeKind::BUILTIN)
    return false;

  auto *builtin = static_cast<BuiltinType *>(type);

  return builtin->is_floating_type();
}

bool is_arithmetic_type(Type *type) {
  if (type->kind == TypeKind::ENUM) {
    return true;
  }

  if (type->kind != TypeKind::BUILTIN) {
    return false;
  }

  auto *b = static_cast<BuiltinType *>(type);

  return b->is_integer_type() || b->is_floating_type();
}

bool is_object_type(Type *type) {
  return type->kind != TypeKind::FUNCTION;
}

bool is_complete_type(Type *type) {
  switch (type->kind) {

  case TypeKind::ERROR:
    return false;

  case TypeKind::FUNCTION:
    return false;

  case TypeKind::BUILTIN: {
    auto *builtin = static_cast<BuiltinType *>(type);

    return builtin->builtin_kind != BuiltinKind::VOID;
  }

  case TypeKind::POINTER:
    return true;

  case TypeKind::ENUM:
    return true;

  case TypeKind::STRUCT: {
    auto *st = static_cast<StructType *>(type);
    return st->complete;
  }

  case TypeKind::UNION: {
    auto *un = static_cast<UnionType *>(type);
    return un->complete;
  }

  case TypeKind::ARRAY: {
    auto *arr = static_cast<ArrayType *>(type);

    if (arr->flexible)
      return false;

    if (arr->variable_length)
      return true;

    return is_complete_type(arr->element_type);
  }
  }

  return false;
}

bool is_struct_or_union_type(Type *type) {
  return type->kind == TypeKind::STRUCT || type->kind == TypeKind::UNION;
}

bool is_function_type(Type *type) {
  return type->kind == TypeKind::FUNCTION;
}

bool is_array_type(Type *type) {
  return type->kind == TypeKind::ARRAY;
}

bool is_pointer_type(Type *type) {
  return type->kind == TypeKind::POINTER;
}

bool is_scalar_type(Type *type) {
  return is_arithmetic_type(type) || is_pointer_type(type);
}

bool same_type(Type *lhs, Type *rhs) {
  return lhs->equals(rhs);
}

bool is_modifiable_lvalue(const Expr *expr) {
  if (!expr) {
    return false;
  }

  if (expr->value_category != ValueCategory::LVALUE) {
    return false;
  }

  Type *type = expr->type;

  if (!type) {
    return false;
  }

  if (!is_object_type(type)) {
    return false;
  }

  if (!is_complete_type(type)) {
    return false;
  }

  if (is_array_type(type)) {
    return false;
  }

  if (type->qualifiers & static_cast<Type::QualifierMask>(TypeQualifier::CONST)) {
    return false;
  }

  return true;
}