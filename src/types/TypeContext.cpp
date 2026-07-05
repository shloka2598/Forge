#include "./TypeContext.h"
#include "./Types.h"

PointerType *TypeContext::get_pointer_type(Type *pointee, Type::QualifierMask qualifiers) {
  auto ptr_type = std::make_unique<PointerType>(pointee, qualifiers);
  PointerType *result = ptr_type.get();
  allocated_types.push_back(std::move(ptr_type));

  return result;
}
ArrayType *TypeContext::get_array_type(Type *ele_type, size_t arr_size, bool variable, bool flexible) {
  auto arr_type = std::make_unique<ArrayType>(ele_type, arr_size, variable, flexible);
  ArrayType *result = arr_type.get();
  allocated_types.push_back(std::move(arr_type));

  return result;
}
FunctionType *TypeContext::get_function_type(Type *return_type, std::vector<Type *> params) {
  auto fn_type = std::make_unique<FunctionType>(return_type, std::move(params));

  FunctionType *result = fn_type.get();

  allocated_types.push_back(std::move(fn_type));

  return result;
}

StructType *TypeContext::get_struct_type(const std::string &name) {
  auto it = struct_types.find(name);
  if (it != struct_types.end()) {
    return it->second;
  }
  auto struct_type = std::make_unique<StructType>(name);
  StructType *result = struct_type.get();
  allocated_types.push_back(std::move(struct_type));
  struct_types[name] = result;

  return result;
}

UnionType *TypeContext::get_union_type(const std::string &name) {
  auto it = union_types.find(name);
  if (it != union_types.end()) {
    return it->second;
  }
  auto union_type = std::make_unique<UnionType>(name);
  UnionType *result = union_type.get();
  allocated_types.push_back(std::move(union_type));
  union_types[name] = result;

  return result;
}

EnumType *TypeContext::get_enum_type(const std::string &name) {
  auto it = enum_types.find(name);
  if (it != enum_types.end()) {
    return it->second;
  }
  auto enum_type = std::make_unique<EnumType>(name);
  EnumType *result = enum_type.get();
  allocated_types.push_back(std::move(enum_type));
  enum_types[name] = result;

  return result;
}

Type *TypeContext::qualify(Type *type, Type::QualifierMask qualifiers) {
  if (qualifiers == 0) {
    return type;
  }

  switch (type->kind) {
  case TypeKind::BUILTIN: {
    auto *old = static_cast<BuiltinType *>(type);
    auto copy = std::make_unique<BuiltinType>(*old);
    copy->qualifiers = qualifiers;
    auto *result = copy.get();
    allocated_types.push_back(std::move(copy));

    return result;
  }
  case TypeKind::POINTER: {
    auto *old = static_cast<PointerType *>(type);
    auto copy = std::make_unique<PointerType>(*old);
    copy->qualifiers = qualifiers;
    auto *result = copy.get();
    allocated_types.push_back(std::move(copy));

    return result;
  }
  case TypeKind::ARRAY: {
    auto *old = static_cast<ArrayType *>(type);
    auto copy = std::make_unique<ArrayType>(*old);
    copy->qualifiers = qualifiers;
    auto *result = copy.get();
    allocated_types.push_back(std::move(copy));

    return result;
  }
  case TypeKind::FUNCTION: {
    auto *old = static_cast<FunctionType *>(type);
    auto copy = std::make_unique<FunctionType>(*old);
    copy->qualifiers = qualifiers;
    auto *result = copy.get();
    allocated_types.push_back(std::move(copy));

    return result;
  }
  case TypeKind::STRUCT: {
    auto *old = static_cast<StructType *>(type);
    auto copy = std::make_unique<StructType>(*old);
    copy->qualifiers = qualifiers;
    auto *result = copy.get();
    allocated_types.push_back(std::move(copy));

    return result;
  }
  case TypeKind::UNION: {
    auto *old = static_cast<UnionType *>(type);
    auto copy = std::make_unique<UnionType>(*old);
    copy->qualifiers = qualifiers;
    auto *result = copy.get();
    allocated_types.push_back(std::move(copy));

    return result;
  }
  case TypeKind::ENUM: {
    auto *old = static_cast<EnumType *>(type);
    auto copy = std::make_unique<EnumType>(*old);
    copy->qualifiers = qualifiers;
    auto *result = copy.get();
    allocated_types.push_back(std::move(copy));

    return result;
  }

  case TypeKind::ERROR:
    return type;
  }

  return type;
}

void TypeContext::rename_struct(StructType *type, const std::string &new_name) {
  struct_types.erase(type->name);
  type->name = new_name;
  struct_types[new_name] = type;
}

void TypeContext::rename_union(UnionType *type, const std::string &new_name) {
  union_types.erase(type->name);
  type->name = new_name;
  union_types[new_name] = type;
}

void TypeContext::rename_enum(EnumType *type, const std::string &new_name) {
  enum_types.erase(type->name);
  type->name = new_name;
  enum_types[new_name] = type;
}
