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