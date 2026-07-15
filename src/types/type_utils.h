#pragma once

#include "Types.h"
#include "ast/exprs.h"

size_t sizeof_type(Type *);
bool is_integer_type(Type *);
bool is_floating_type(Type *);
bool is_arithmetic_type(Type *);
bool is_pointer_type(Type *);
bool is_scalar_type(Type *);
bool is_object_type(Type *);
bool is_complete_type(Type *);
bool is_function_type(Type *);
bool is_array_type(Type *);
bool is_struct_or_union_type(Type *);
bool is_modifiable_lvalue(const Expr *);
bool same_type(Type *, Type *);