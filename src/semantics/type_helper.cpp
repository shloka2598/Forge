#include "./semantics.h"

#include "types/type_utils.h"

Type *Semantics::convert_datatype(DataType datatype) {
  switch (datatype) {
  case DataType::VOID:
    return &type_context.void_type;

  case DataType::CHAR:
    return &type_context.char_type;

  case DataType::UCHAR:
    return &type_context.uchar_type;

  case DataType::SHORT:
    return &type_context.short_type;

  case DataType::USHORT:
    return &type_context.ushort_type;

  case DataType::INT:
    return &type_context.int_type;

  case DataType::UINT:
    return &type_context.uint_type;

  case DataType::LONG:
    return &type_context.long_type;

  case DataType::ULONG:
    return &type_context.ulong_type;

  case DataType::LONGLONG:
    return &type_context.longlong_type;

  case DataType::ULONGLONG:
    return &type_context.ulonglong_type;

  case DataType::FLOAT:
    return &type_context.float_type;

  case DataType::DOUBLE:
    return &type_context.double_type;

  case DataType::LONGDOUBLE:
    return &type_context.longdouble_type;

  default:
    return &type_context.error_type;
  }
}

bool Semantics::is_integer_type(Type *type) {
  if (type->kind == TypeKind::ENUM) {
    return true;
  }

  if (type->kind != TypeKind::BUILTIN) {
    return false;
  }

  return static_cast<BuiltinType *>(type)->is_integer_type();
}

bool Semantics::is_floating_type(Type *type) {
  if (type->kind != TypeKind::BUILTIN)
    return false;

  auto *builtin = static_cast<BuiltinType *>(type);

  return builtin->is_floating_type();
}

bool Semantics::is_arithmetic_type(Type *type) {
  if (type->kind == TypeKind::ENUM) {
    return true;
  }

  if (type->kind != TypeKind::BUILTIN) {
    return false;
  }

  auto *b = static_cast<BuiltinType *>(type);

  return b->is_integer_type() || b->is_floating_type();
}

bool Semantics::is_object_type(Type *type) {
  return type->kind != TypeKind::FUNCTION;
}

bool Semantics::is_complete_type(Type *type) {
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

bool Semantics::is_struct_or_union_type(Type *type) {
  return type->kind == TypeKind::STRUCT || type->kind == TypeKind::UNION;
}

bool Semantics::is_function_type(Type *type) {
  return type->kind == TypeKind::FUNCTION;
}

bool Semantics::is_array_type(Type *type) {
  return type->kind == TypeKind::ARRAY;
}

bool Semantics::can_assign(Type *lhs, Type *rhs) {
  if (lhs->kind == TypeKind::ERROR || rhs->kind == TypeKind::ERROR) {
    return true;
  }

  rhs = decay(rhs);

  if (lhs->equals(rhs)) {
    return true;
  }

  if (is_arithmetic_type(lhs) && is_arithmetic_type(rhs)) {
    return true;
  }

  if (compatible_pointer_types(lhs, rhs)) {
    return true;
  }

  return false;
}

bool Semantics::is_pointer_type(Type *type) {
  return type->kind == TypeKind::POINTER;
}

bool Semantics::is_scalar_type(Type *type) {
  return is_arithmetic_type(type) || is_pointer_type(type);
}

Type *Semantics::integer_promotion(Type *type) {
  if (type->kind == TypeKind::ENUM) {
    return &type_context.int_type;
  }

  if (type->kind != TypeKind::BUILTIN) {
    return type;
  }

  auto *builtin = static_cast<BuiltinType *>(type);

  if (!builtin->is_integer_type())
    return type;

  if (builtin->rank >= IntegerRank::INT) {
    return type;
  }

  // char / short -> int if int can represent them
  return &type_context.int_type;
}

Type *Semantics::default_argument_promotion(Type *type) {
  type = decay(type);

  if (type->kind != TypeKind::BUILTIN) {
    return type;
  }

  auto *builtin = static_cast<BuiltinType *>(type);

  if (builtin->builtin_kind == BuiltinKind::FLOAT) {
    return &type_context.double_type;
  }

  return integer_promotion(type);
}

Type *Semantics::usual_arithmetic_conversion(Type *lhs, Type *rhs) {

  if (lhs->kind == TypeKind::ERROR || rhs->kind == TypeKind::ERROR) {
    return &type_context.error_type;
  }

  lhs = integer_promotion(lhs);
  rhs = integer_promotion(rhs);

  if (!is_arithmetic_type(lhs) || !is_arithmetic_type(rhs)) {
    return &type_context.error_type;
  }

  auto *left = static_cast<BuiltinType *>(lhs);
  auto *right = static_cast<BuiltinType *>(rhs);

  // Floating types
  if (left->is_floating_type() || right->is_floating_type()) {
    return common_real_type(lhs, rhs);
  }

  // Same type
  if (left->equals(right)) {
    return left;
  }

  // Same signedness
  if (left->is_signed == right->is_signed) {
    if (left->rank >= right->rank) {
      return left;
    }
    return right;
  }

  // Mixed signed / unsigned
  BuiltinType *signed_type = left->is_signed ? left : right;
  BuiltinType *unsigned_type = left->is_signed ? right : left;

  if (unsigned_type->rank >= signed_type->rank) {
    return unsigned_type;
  }

  if (signed_type->bit_width > unsigned_type->bit_width) {
    return signed_type;
  }

  // convert to unsigned version
  switch (*(signed_type->rank)) {
  case IntegerRank::INT:
    return &type_context.uint_type;

  case IntegerRank::LONG:
    return &type_context.ulong_type;

  case IntegerRank::LONGLONG:
    return &type_context.ulonglong_type;

  default:
    return &type_context.uint_type;
  }
}

Type *Semantics::common_real_type(Type *lhs, Type *rhs) {
  auto *left = static_cast<BuiltinType *>(lhs);
  auto *right = static_cast<BuiltinType *>(rhs);

  if (left->builtin_kind == BuiltinKind::LONGDOUBLE || right->builtin_kind == BuiltinKind::LONGDOUBLE) {
    return &type_context.longdouble_type;
  }

  if (left->builtin_kind == BuiltinKind::DOUBLE || right->builtin_kind == BuiltinKind::DOUBLE) {
    return &type_context.double_type;
  }

  return &type_context.float_type;
}

Type *Semantics::decay(Type *type) {
  if (type->kind == TypeKind::ARRAY) {
    auto *arr = static_cast<ArrayType *>(type);
    return type_context.get_pointer_type(arr->element_type);
  }

  return type;
}

bool Semantics::compatible_pointer_types(Type *lhs, Type *rhs) {
  if (lhs->kind != TypeKind::POINTER || rhs->kind != TypeKind::POINTER) {
    return false;
  }

  auto *lp = static_cast<PointerType *>(lhs);
  auto *rp = static_cast<PointerType *>(rhs);

  Type *L = lp->pointee_type;
  Type *R = rp->pointee_type;

  // identical types
  if (L->equals(R)) {
    return true;
  }

  // void* is compatible with any object pointer
  if (L->kind == TypeKind::BUILTIN && static_cast<BuiltinType *>(L)->builtin_kind == BuiltinKind::VOID) {
    return is_object_type(R);
  }

  if (R->kind == TypeKind::BUILTIN && static_cast<BuiltinType *>(R)->builtin_kind == BuiltinKind::VOID) {
    return is_object_type(L);
  }

  return false;
}

bool Semantics::is_comparable_pointer_types(Type *lhs, Type *rhs) {
  if (!is_pointer_type(lhs) || !is_pointer_type(rhs))
    return false;

  if (compatible_pointer_types(lhs, rhs))
    return true;

  auto *lp = static_cast<PointerType *>(lhs);
  auto *rp = static_cast<PointerType *>(rhs);

  if (lp->pointee_type->kind == TypeKind::BUILTIN && static_cast<BuiltinType *>(lp->pointee_type)->builtin_kind == BuiltinKind::VOID) {
    return is_object_type(rp->pointee_type);
  }

  if (rp->pointee_type->kind == TypeKind::BUILTIN && static_cast<BuiltinType *>(rp->pointee_type)->builtin_kind == BuiltinKind::VOID) {
    return is_object_type(lp->pointee_type);
  }

  return false;
}

bool Semantics::is_null_pointer_constant(const Expr *expr) {
  auto *literal = dynamic_cast<const IntLetExpr *>(expr);

  if (!literal) {
    return false;
  }

  return literal->value == 0;
}

bool Semantics::can_implicitly_convert(Type *from, Type *to) {
  if (from->kind == TypeKind::ERROR || to->kind == TypeKind::ERROR) {
    return true;
  }

  if (same_type(from, to)) {
    return true;
  }

  from = decay(from);

  if (is_arithmetic_type(from) && is_arithmetic_type(to)) {
    return true;
  }

  // pointer conversions
  if (is_pointer_type(from) && is_pointer_type(to)) {
    return compatible_pointer_types(to, from);
  }

  // integer constant 0 -> pointer
  return false;
}

bool Semantics::can_explicitly_cast(Type *from, Type *to) {
  if (from->kind == TypeKind::ERROR || to->kind == TypeKind::ERROR) {
    return true;
  }

  if (same_type(from, to)) {
    return true;
  }

  from = decay(from);

  // arithmetic <-> arithmetic
  if (is_arithmetic_type(from) && is_arithmetic_type(to)) {
    return true;
  }

  // pointer -> pointer
  if (is_pointer_type(from) && is_pointer_type(to)) {
    return true;
  }

  // integer -> pointer
  if (is_integer_type(from) && is_pointer_type(to)) {
    return true;
  }

  // pointer -> integer
  if (is_pointer_type(from) && is_integer_type(to)) {
    return true;
  }

  return false;
}

bool Semantics::same_type(Type *lhs, Type *rhs) {
  return lhs->equals(rhs);
}

bool Semantics::contains_void_object(Type *type) {
  switch (type->kind) {

  case TypeKind::BUILTIN: {
    auto *builtin = static_cast<BuiltinType *>(type);
    return builtin->builtin_kind == BuiltinKind::VOID;
  }

  case TypeKind::ARRAY: {
    auto *arr = static_cast<ArrayType *>(type);
    return contains_void_object(arr->element_type);
  }

  case TypeKind::POINTER:
    return false;

  default:
    return false;
  }
}

Type *Semantics::lookup_tag_type(Scope &scope, const Token &token, const std::string &name, SymbolKind expected_kind) {
  Symbol *sym = scope.lookup_tag(name);

  if (!sym) {
    switch (expected_kind) {
    case SymbolKind::STRUCT: {
      error(token, "Unknown struct '" + name + "'");
      break;
    }

    case SymbolKind::UNION: {
      error(token, "Unknown union '" + name + "'");
      break;
    }

    case SymbolKind::ENUM: {
      error(token, "Unknown enum '" + name + "'");
      break;
    }

    default: {
      error(token, "Unknown tag '" + name + "'");
      break;
    }
    }

    return &type_context.error_type;
  }

  if (sym->kind != expected_kind) {
    error(token, "Tag '" + name + "' has wrong kind");
    return &type_context.error_type;
  }

  return sym->type;
}

Type *Semantics::build_type(const ParsedType &parsed, Scope &scope) {
  Type *type = nullptr;

  // Base type
  switch (parsed.datatype) {
  case DataType::VOID:
  case DataType::CHAR:
  case DataType::UCHAR:
  case DataType::SHORT:
  case DataType::USHORT:
  case DataType::INT:
  case DataType::UINT:
  case DataType::LONG:
  case DataType::ULONG:
  case DataType::LONGLONG:
  case DataType::ULONGLONG:
  case DataType::FLOAT:
  case DataType::DOUBLE:
  case DataType::LONGDOUBLE:
    type = convert_datatype(parsed.datatype);
    break;

  case DataType::STRUCT: {
    type = lookup_tag_type(scope, parsed.token, parsed.custom_name, SymbolKind::STRUCT);
    break;
  }
  case DataType::UNION: {
    type = lookup_tag_type(scope, parsed.token, parsed.custom_name, SymbolKind::UNION);
    break;
  }
  case DataType::ENUM: {
    type = lookup_tag_type(scope, parsed.token, parsed.custom_name, SymbolKind::ENUM);
    break;
  }
  case DataType::TYPEDEF_NAME: {
    Symbol *sym = scope.lookup_identifier(parsed.custom_name);

    if (!sym || sym->kind != SymbolKind::TYPEDEF) {
      error(parsed.token, "Unknown typedef '" + parsed.custom_name + "'");
      return &type_context.error_type;
    }

    type = sym->type;
    break;
  }
  default:
    return &type_context.error_type;
  }

  // Top level qualifiers
  type = type_context.qualify(type, parsed.qualifiers);

  if (type == &type_context.error_type) {
    return type;
  }

  // Pointers
  for (size_t i = 0; i < parsed.pointer_depth; ++i) {
    Type::QualifierMask quals{};
    if (i < parsed.pointer_qualifiers.size()) {
      quals = parsed.pointer_qualifiers[i];
    }
    type = type_context.get_pointer_type(type, quals);
  }

  // Arrays
  for (auto it = parsed.dimensions.rbegin(); it != parsed.dimensions.rend(); ++it) {
    bool flexible = false;
    bool variable = false;

    size_t size = *it;

    if (size == 0) {
      if (parsed.flexible_array) {
        flexible = true;
      } else if (parsed.variable_length_array) {
        variable = true;
      }
    }

    type = type_context.get_array_type(type, size, variable, flexible);
  }

  return type;
}

std::optional<int64_t> Semantics::evaluate_constant_expr(const Expr *expr) {
  switch (expr->expr_type()) {

  case ExprType::INT_LITERAL: {
    const auto &lit = static_cast<const IntLetExpr &>(*expr);
    return lit.value;
  }
  case ExprType::IDENTIFIER: {
    const auto &id = static_cast<const IdentifierExpr &>(*expr);

    Symbol *sym = global_scope.lookup_identifier(id.identifier_name);

    if (!sym) {
      return std::nullopt;
    }

    if (sym->kind != SymbolKind::ENUM_CONSTANT) {
      return std::nullopt;
    }

    return sym->enum_value;
  }
  case ExprType::UNARY: {
    const auto &unary_expr = static_cast<const UnaryExpr &>(*expr);
    auto operand = evaluate_constant_expr(unary_expr.right_expr.get());
    if (!operand) {
      return std::nullopt;
    }

    switch (unary_expr.op) {
    case TokenType::PLUS:
      return *operand;

    case TokenType::MINUS:
      return -*operand;

    case TokenType::TILDE:
      return ~(*operand);

    case TokenType::EXCLAMATION:
      return !(*operand);

    default:
      return std::nullopt;
    }
  }

  case ExprType::BINARY: {
    auto *binary_expr = static_cast<const BinaryExpr *>(expr);

    auto lhs = evaluate_constant_expr(binary_expr->left_expr.get());
    auto rhs = evaluate_constant_expr(binary_expr->right_expr.get());

    if (!lhs || !rhs) {
      return std::nullopt;
    }

    switch (binary_expr->op) {
    case TokenType::PLUS:
      return *lhs + *rhs;

    case TokenType::MINUS:
      return *lhs - *rhs;

    case TokenType::MULTIPLY:
      return *lhs * *rhs;

    case TokenType::DIVIDE:
      if (*rhs == 0) {
        return std::nullopt;
      }
      return *lhs / *rhs;

    case TokenType::MODULO:
      if (*rhs == 0) {
        return std::nullopt;
      }
      return *lhs % *rhs;

    case TokenType::LEFT_SHIFT:
      if (*rhs < 0) {
        return std::nullopt;
      }
      return *lhs << *rhs;

    case TokenType::RIGHT_SHIFT:
      if (*rhs < 0) {
        return std::nullopt;
      }
      return *lhs >> *rhs;
    case TokenType::AMPERSAND:
      return *lhs & *rhs;

    case TokenType::PIPE:
      return *lhs | *rhs;

    case TokenType::CARET:
      return *lhs ^ *rhs;

    case TokenType::DOUBLE_AMPERSAND:
      return (*lhs && *rhs);

    case TokenType::DOUBLE_PIPE:
      return (*lhs || *rhs);

    case TokenType::SMALLER_THAN:
      return *lhs < *rhs;

    case TokenType::GREATER_THAN:
      return *lhs > *rhs;

    case TokenType::SMALLER_THAN_EQUAL_THAN:
      return *lhs <= *rhs;

    case TokenType::GREATER_THAN_EQUAL_THAN:
      return *lhs >= *rhs;

    case TokenType::DOUBLE_EQUALS:
      return *lhs == *rhs;

    case TokenType::NOT_EQUALS:
      return *lhs != *rhs;

    case TokenType::COMMA:
      return rhs;

    default:
      return std::nullopt;
    }
  }

  case ExprType::CONDITIONAL: {
    auto *cond = static_cast<const ConditionalExpr *>(expr);
    auto c = evaluate_constant_expr(cond->condition.get());

    if (!c) {
      return std::nullopt;
    }

    if (*c) {
      return evaluate_constant_expr(cond->true_expr.get());
    }

    return evaluate_constant_expr(cond->false_expr.get());
  }

  case ExprType::SIZEOF: {
    auto *sz = static_cast<const SizeofExpr *>(expr);

    if (sz->parsed_type.has_value()) {
      Type *t = build_type(sz->parsed_type.value(), global_scope);
      return sizeof_type(t);
    }

    Type *t = analyze_expr(sz->expr.get(), global_scope);
    return sizeof_type(t);
  }

  default:
    return std::nullopt;
  }
}

bool Semantics::is_modifiable_lvalue(const Expr *expr) {
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

Type *Semantics::composite_pointer_type(Type *lhs, Type *rhs) {

  if (!is_pointer_type(lhs) || !is_pointer_type(rhs)) {
    return &type_context.error_type;
  }

  auto *lp = static_cast<PointerType *>(lhs);
  auto *rp = static_cast<PointerType *>(rhs);

  Type *L = lp->pointee_type;
  Type *R = rp->pointee_type;

  if (L->equals(R)) {
    return lhs;
  }

  if (L->kind == TypeKind::BUILTIN && static_cast<BuiltinType *>(L)->builtin_kind == BuiltinKind::VOID) {
    return lhs;
  }

  if (R->kind == TypeKind::BUILTIN && static_cast<BuiltinType *>(R)->builtin_kind == BuiltinKind::VOID) {
    return rhs;
  }

  return &type_context.error_type;
}