#include "semantics.h"
#include "types/type_utils.h"

#include <unordered_set>

void Semantics::error(const Token &token, const std::string &msg) {
  has_error = true;
  diagnostics.error(msg, token.line, token.column, token.length);
}

void Semantics::warning(const Token &token, const std::string &msg) {
  diagnostics.warning(msg, token.line, token.column, token.length);
}

bool Semantics::register_symbol(Scope &scope, SymbolKind kind, const std::string &name, Type *type) {
  return scope.insert({.kind = kind, .name = name, .type = type, .is_defined = false, .decl_node = nullptr, .enum_value = std::nullopt});
}

void Semantics::analyze_function(FunctionDecl *fn_decl, Scope &parent_scope) {
  Scope fn_scope{&parent_scope};

  current_function_return_type = build_type(fn_decl->return_type, parent_scope);
  fn_decl->resolved_return_type = current_function_return_type;

  if (contains_void_object(current_function_return_type) && !same_type(current_function_return_type, &type_context.void_type)) {
    error(fn_decl->token, "Function cannot return object of void type");
  }

  for (Parameter &param : fn_decl->parameters) {
    Type *param_type = build_type(param.type, fn_scope);
    param.resolved_type = param_type;

    if (!is_complete_type(param_type) && !is_pointer_type(param_type)) {
      error(param.token, "Parameter has incomplete type");
    }

    param_type = decay(param_type);

    if (is_function_type(param_type)) {
      param_type = type_context.get_pointer_type(param_type);
    }

    if (contains_void_object(param_type) && !param.type.pointer_depth) {
      error(param.token, "Object cannot have void type");
    }

    if (param.name.has_value()) {
      if (is_function_type(param_type)) {
        error(param.token, "Function parameter cannot have function type");
      }
      if (!register_symbol(fn_scope, SymbolKind::PARAMETER, *param.name, param_type)) {
        error(param.token, "Duplicate parameter " + *param.name);
      }
    }
  }
  if (fn_decl->body) {
    bool returns = analyze_stmt(fn_decl->body.get(), fn_scope);
    if (!same_type(current_function_return_type, &type_context.void_type) && !returns) {
      error(fn_decl->token, "Control reaches end of non-void function");
    }
  }
}

bool Semantics::analyze_stmt(Stmt *stmt, Scope &scope) {
  switch (stmt->stmt_type()) {
  case StmtType::VARIABLE_DECL_STMT: {
    auto &vd_stmt = static_cast<VariableDeclarationStmt &>(*stmt);

    Type *type = build_type(vd_stmt.type, scope);
    vd_stmt.resolved_type = type;

    if (!is_complete_type(type) && !is_pointer_type(type)) {
      error(vd_stmt.token, "Variable has incomplete type");
    }
    if (contains_void_object(type) && vd_stmt.type.pointer_depth == 0) {
      error(vd_stmt.token, "Object cannot have void type");
    }
    if (!register_symbol(scope, SymbolKind::VARIABLE, vd_stmt.var_name, type)) {
      error(vd_stmt.token, "Duplicate local variable " + vd_stmt.var_name);
    }
    if (is_function_type(type)) {
      error(vd_stmt.token, "Variable cannot have function type");
    }

    if (vd_stmt.expr_ptr) {
      Type *rhs_type = analyze_expr(vd_stmt.expr_ptr.get(), scope);
      rhs_type = default_argument_promotion(decay(rhs_type));
      if (!can_assign(type, rhs_type)) {
        error(vd_stmt.expr_ptr->token, "Cannot initialize '" + vd_stmt.var_name + "' of type '" + type->to_string() + "' with expression of type '" + rhs_type->to_string() + "'");
      }
    }

    if (vd_stmt.init) {
      if (!is_array_type(type)) {
        error(vd_stmt.init->token, "Brace initializer requires array type");
      }
      analyze_array_initializer(*vd_stmt.init, type, scope);
    }

    return false;
  }
  case StmtType::EXPRESSION_STMT: {
    const ExpressionStmt &expr_stmt = static_cast<const ExpressionStmt &>(*stmt);
    analyze_expr(expr_stmt.expr.get(), scope);
    return false;
  }
  case StmtType::BLOCK_STMT: {
    const auto &block_stmt = static_cast<const BlockStmt &>(*stmt);
    Scope block_scope(&scope);

    bool returned = false;

    for (const auto &statement : block_stmt.statements) {
      if (returned) {
        warning(statement->token, "Unreachable statement");
      }
      returned |= analyze_stmt(statement.get(), block_scope);
    }
    return returned;
  }
  case StmtType::RETURN_STMT: {
    const auto &return_stmt = static_cast<const ReturnStmt &>(*stmt);

    if (same_type(current_function_return_type, &type_context.void_type)) {
      if (return_stmt.expr_ptr) {
        error(return_stmt.token, "Void function cannot return a value");
      }
      return true;
    }

    if (!return_stmt.expr_ptr) {
      error(return_stmt.token, "Missing return value");
      return true;
    }

    Type *expr_type = analyze_expr(return_stmt.expr_ptr.get(), scope);
    expr_type = decay(expr_type);
    expr_type = default_argument_promotion(expr_type);
    if (!can_assign(current_function_return_type, expr_type)) {
      error(return_stmt.expr_ptr->token, "Cannot return " + expr_type->to_string() + " from function returning " + current_function_return_type->to_string());
    }

    return true;
  }
  case StmtType::IF_STMT: {
    const auto &if_stmt = static_cast<const IfStmt &>(*stmt);
    Type *condition_type = default_argument_promotion(decay(analyze_expr(if_stmt.condition.get(), scope)));

    if (!is_scalar_type(condition_type)) {
      error(if_stmt.condition->token, "Condition must have scalar type");
    }

    bool then_returns = analyze_stmt(if_stmt.then_body.get(), scope);

    if (!if_stmt.else_body) {
      return false;
    }

    bool else_returns = analyze_stmt(if_stmt.else_body.get(), scope);

    return then_returns && else_returns;
  }
  case StmtType::WHILE_STMT: {
    const auto &while_stmt = static_cast<const WhileStmt &>(*stmt);
    Type *condition_type = default_argument_promotion(decay(analyze_expr(while_stmt.condition.get(), scope)));
    if (!is_scalar_type(condition_type)) {
      error(while_stmt.condition->token, "Condition must have scalar type");
    }
    loop_depth++;
    analyze_stmt(while_stmt.body.get(), scope);
    loop_depth--;
    return false;
  }
  case StmtType::DO_WHILE_STMT: {
    const auto &do_stmt = static_cast<const DoWhileStmt &>(*stmt);
    loop_depth++;
    analyze_stmt(do_stmt.body.get(), scope);
    loop_depth--;
    Type *condition_type = default_argument_promotion(decay(analyze_expr(do_stmt.condition.get(), scope)));
    if (!is_scalar_type(condition_type)) {
      error(do_stmt.condition->token, "Condition must have scalar type");
    }
    return false;
  }
  case StmtType::FOR_STMT: {
    const auto &for_stmt = static_cast<const ForStmt &>(*stmt);
    Scope loop_scope(&scope);
    if (for_stmt.init_stmt) {
      analyze_stmt(for_stmt.init_stmt.get(), loop_scope);
    }
    if (for_stmt.condition) {
      Type *cond_type = default_argument_promotion(decay(analyze_expr(for_stmt.condition.get(), loop_scope)));

      if (!is_scalar_type(cond_type)) {
        error(for_stmt.condition->token, "Condition must have scalar type");
      }
    }

    loop_depth++;

    analyze_stmt(for_stmt.body.get(), loop_scope);

    if (for_stmt.update_stmt) {
      analyze_stmt(for_stmt.update_stmt.get(), loop_scope);
    }

    loop_depth--;
    return false;
  }
  case StmtType::SWITCH_STMT: {
    const auto &switch_stmt = static_cast<const SwitchStmt &>(*stmt);
    Type *cond_type = decay(analyze_expr(switch_stmt.condition.get(), scope));
    cond_type = integer_promotion(cond_type);

    if (!is_integer_type(cond_type)) {
      error(switch_stmt.condition->token, "Switch condition must be integer");
    }
    switch_depth++;

    std::unordered_set<int64_t> seen_cases;
    bool all_cases_return = true;
    for (const auto &case_stmt : switch_stmt.cases) {
      Type *case_type = integer_promotion(decay(analyze_expr(case_stmt.value.get(), scope)));
      if (!is_integer_type(case_type)) {
        error(case_stmt.value->token, "case label must have integer type");
      }
      auto value = evaluate_constant_expr(case_stmt.value.get());

      if (!value) {
        error(case_stmt.value->token, "Aase label must be a constant expression");
      } else if (!seen_cases.insert(*value).second) {
        error(case_stmt.value->token, "Duplicate case label");
      }
      if (!analyze_stmt(case_stmt.body.get(), scope)) {
        all_cases_return = false;
      }
    }

    if (switch_stmt.default_body) {
      if (!analyze_stmt(switch_stmt.default_body.get(), scope)) {
        all_cases_return = false;
      }
    } else {
      all_cases_return = false;
    }

    switch_depth--;

    return all_cases_return;
  }
  case StmtType::STRUCT_DECL_STMT: {
    const auto &struct_stmt = static_cast<const StructDeclarationStmt &>(*stmt);
    const auto &decl = struct_stmt.declaration;

    Type *new_type = type_context.get_struct_type(decl->name);

    Symbol *sym = scope.insert_or_get_tag(SymbolKind::STRUCT, decl->name, new_type);

    StructType *st = static_cast<StructType *>(sym->type);

    if (!decl->is_definition) {
      if (st->complete) {
        return false;
      }
      return false;
    }

    if (st->complete) {
      error(struct_stmt.token, "Redefinition of struct '" + decl->name + "'");
      return false;
    }

    std::unordered_set<std::string> member_names;
    bool valid = true;

    for (const auto &field : decl->fields) {
      Type *field_type = build_type(field.type, scope);
      bool ok = true;

      if (is_function_type(field_type)) {
        error(struct_stmt.token, "Struct member cannot have function type");
        ok = false;
        valid = false;
      }
      if (contains_void_object(field_type) && field.type.pointer_depth == 0) {
        error(struct_stmt.token, "Struct member cannot have void type");
        ok = false;
        valid = false;
      }
      if (!is_complete_type(field_type) && !is_pointer_type(field_type)) {
        error(struct_stmt.token, "Struct member has incomplete type");
        ok = false;
        valid = false;
      }

      if (!member_names.insert(field.name).second) {
        error(struct_stmt.token, "Duplicate member '" + field.name + "'");
        ok = false;
        valid = false;
      }

      if (ok) {
        st->members.push_back({field.name, field_type});
      }
    }

    if (valid) {
      st->complete = true;
    }
    return false;
  }
  case StmtType::UNION_DECL_STMT: {
    const auto &union_stmt = static_cast<const UnionDeclarationStmt &>(*stmt);
    const auto &decl = union_stmt.declaration;
    Type *new_type = type_context.get_union_type(decl->name);

    Symbol *sym = scope.insert_or_get_tag(SymbolKind::UNION, decl->name, new_type);

    UnionType *ut = static_cast<UnionType *>(sym->type);

    if (!decl->is_definition) {
      if (ut->complete) {
        return false;
      }
      return false;
    }

    if (ut->complete) {
      error(union_stmt.token, "Redefinition of union '" + decl->name + "'");
      return false;
    }

    std::unordered_set<std::string> member_names;
    bool valid = true;

    for (const auto &field : decl->fields) {
      bool ok = true;
      Type *field_type = build_type(field.type, scope);
      if (is_function_type(field_type)) {
        error(union_stmt.token, "Union member cannot have function type");
        ok = false;
        valid = false;
      }
      if (contains_void_object(field_type) && field.type.pointer_depth == 0) {
        error(union_stmt.token, "Union member cannot have void type");
        ok = false;
        valid = false;
      }
      if (!is_complete_type(field_type) && !is_pointer_type(field_type)) {
        error(union_stmt.token, "Union member has incomplete type");
        ok = false;
        valid = false;
      }

      if (!member_names.insert(field.name).second) {
        error(union_stmt.token, "Duplicate member '" + field.name + "'");
        ok = false;
        valid = false;
      }

      if (ok) {
        ut->members.push_back({field.name, field_type});
      }
    }

    if (valid) {
      ut->complete = true;
    }
    return false;
  }
  case StmtType::FUNCTION_DECL_STMT: {
    const auto &fn_stmt = static_cast<const FunctionDeclStmt &>(*stmt);
    FunctionDecl *fn = fn_stmt.declaration.get();
    std::vector<Type *> params;

    for (const auto &param : fn->parameters) {
      Type *param_type = build_type(param.type, scope);

      if (is_array_type(param_type)) {
        param_type = decay(param_type);
      }

      if (is_function_type(param_type)) {
        param_type = type_context.get_pointer_type(param_type);
      }

      params.push_back(param_type);
    }

    Type *ret_type = build_type(fn->return_type, scope);
    if (contains_void_object(ret_type) && fn->return_type.pointer_depth == 0 && !same_type(ret_type, &type_context.void_type)) {
      error(fn_stmt.token, "Function cannot return object of void type");
    }
    FunctionType *fn_type = type_context.get_function_type(ret_type, std::move(params));

    Symbol *existing = scope.lookup_local_identifier(fn->name);

    if (!existing) {
      register_symbol(scope, SymbolKind::FUNCTION, fn->name, fn_type);
      existing = scope.lookup_identifier(fn->name);
      if (!fn->is_prototype) {
        existing->is_defined = true;
      }
    } else {
      if (existing->kind != SymbolKind::FUNCTION) {
        error(fn_stmt.token, fn->name + " already exists as non-function");
        return false;
      }
      auto *old_type = static_cast<FunctionType *>(existing->type);
      if (!old_type->equals(fn_type)) {
        error(fn_stmt.token, "Conflicting declarations for function " + fn->name);
        return false;
      }
      if (!fn->is_prototype) {
        if (existing->is_defined) {
          error(fn_stmt.token, "Multiple definitions of function " + fn->name);
          return false;
        }
        existing->is_defined = true;
      }
    }

    if (!fn->is_prototype) {
      analyze_function(fn, scope);
    }
    return false;
  }
  case StmtType::GLOBAL_VARIABLE_DECL_STMT: {
    auto &glbl_stmt = static_cast<GlobalVariableDeclStmt &>(*stmt);

    auto *glbl = glbl_stmt.declaration.get();
    Type *type = build_type(glbl->type, scope);
    glbl->resolved_type = type;

    if (!is_complete_type(type) && !is_pointer_type(type)) {
      error(glbl->token, "Variable has incomplete type");
    }
    if (contains_void_object(type) && glbl->type.pointer_depth == 0) {
      error(glbl->token, "Object cannot have void type");
    }
    if (is_function_type(type)) {
      error(glbl->token, "Variable cannot have function type");
    }

    if (!register_symbol(scope, SymbolKind::VARIABLE, glbl->name, type)) {
      error(glbl->token, "Variable " + glbl->name + " already exists");
    }

    if (glbl->initializer) {
      Type *rhs = analyze_expr(glbl->initializer.get(), scope);
      rhs = default_argument_promotion(decay(rhs));

      if (!can_assign(type, rhs)) {
        error(glbl->initializer->token,
              "Cannot initialize '" + glbl->name + "' of type '" + type->to_string() + "' with expression of type '" + rhs->to_string() + "'");
      }
    }

    if (glbl->array_initializer) {
      if (!is_array_type(type)) {
        error(glbl->array_initializer->token, "Brace initializer requires array type");
      }

      analyze_array_initializer(*glbl->array_initializer, type, scope);
    }

    return false;
  }
  case StmtType::ENUM_DECL_STMT: {
    const auto &enum_stmt = static_cast<const EnumDeclarationStmt &>(*stmt);
    const auto &decl = enum_stmt.declaration;

    Type *new_type = type_context.get_enum_type(decl->name);

    Symbol *sym = scope.insert_or_get_tag(SymbolKind::ENUM, decl->name, new_type);

    EnumType *et = static_cast<EnumType *>(sym->type);

    // Forward declaration only
    if (!decl->is_definition) {
      return false;
    }

    if (!et->constants.empty()) {
      error(enum_stmt.token, "Redefinition of enum '" + decl->name + "'");
      return false;
    }

    int64_t current_value = 0;

    std::unordered_set<std::string> enumerator_names;

    for (const auto &member : decl->members) {
      if (!enumerator_names.insert(member.name).second) {
        error(enum_stmt.token, "Duplicate enumerator '" + member.name + "'");
        continue;
      }

      if (member.value) {
        auto value = evaluate_constant_expr(member.value.get());

        if (!value) {
          error(member.value->token, "Enumerator '" + member.name + "' is not a constant expression");
          continue;
        }

        current_value = *value;
      }

      Symbol constant{
          .kind = SymbolKind::ENUM_CONSTANT,
          .name = member.name,
          .type = et,
          .is_defined = true,
          .decl_node = &member,
          .enum_value = current_value,
      };

      et->constants.push_back({member.name, current_value});

      if (!scope.insert(constant)) {
        error(enum_stmt.token, "Enumerator '" + member.name + "' already exists");
      }

      ++current_value;
    }

    return false;
  }
  case StmtType::TYPEDEF_DECL_STMT: {
    auto &typedef_stmt = static_cast<const TypedefDeclarationStmt &>(*stmt);

    if (typedef_stmt.anonymous_struct) {
      StructType *st = type_context.get_struct_type(typedef_stmt.alias_name);

      Symbol *sym = scope.insert_or_get_tag(SymbolKind::STRUCT, typedef_stmt.alias_name, st);
      st = static_cast<StructType *>(sym->type);
      std::unordered_set<std::string> member_names;

      bool valid = true;

      for (const auto &field : typedef_stmt.anonymous_struct->fields) {

        Type *field_type = build_type(field.type, scope);

        bool ok = true;

        if (is_function_type(field_type)) {
          error(typedef_stmt.token, "Struct member cannot have function type");
          ok = valid = false;
        }

        if (contains_void_object(field_type) && field.type.pointer_depth == 0) {
          error(typedef_stmt.token, "Struct member cannot have void type");
          ok = valid = false;
        }

        if (!is_complete_type(field_type) && !is_pointer_type(field_type)) {
          error(typedef_stmt.token, "Struct member has incomplete type");
          ok = valid = false;
        }

        if (!member_names.insert(field.name).second) {
          error(typedef_stmt.token, "Duplicate member '" + field.name + "'");
          ok = valid = false;
        }

        if (ok) {
          st->members.push_back({field.name, field_type});
        }
      }

      if (valid)
        st->complete = true;
    } else if (typedef_stmt.anonymous_union) {
      UnionType *ut = type_context.get_union_type(typedef_stmt.alias_name);

      Symbol *sym = scope.insert_or_get_tag(SymbolKind::UNION, typedef_stmt.alias_name, ut);
      ut = static_cast<UnionType *>(sym->type);
      std::unordered_set<std::string> member_names;

      bool valid = true;

      for (const auto &field : typedef_stmt.anonymous_union->fields) {

        Type *field_type = build_type(field.type, scope);

        bool ok = true;

        if (is_function_type(field_type)) {
          error(typedef_stmt.token, "Union member cannot have function type");
          ok = valid = false;
        }

        if (contains_void_object(field_type) && field.type.pointer_depth == 0) {
          error(typedef_stmt.token, "Union member cannot have void type");
          ok = valid = false;
        }

        if (!is_complete_type(field_type) && !is_pointer_type(field_type)) {
          error(typedef_stmt.token, "Union member has incomplete type");
          ok = valid = false;
        }

        if (!member_names.insert(field.name).second) {
          error(typedef_stmt.token, "Duplicate member '" + field.name + "'");
          ok = valid = false;
        }

        if (ok) {
          ut->members.push_back({field.name, field_type});
        }
      }

      if (valid) {
        ut->complete = true;
      }
    } else if (typedef_stmt.anonymous_enum) {
      EnumType *et = type_context.get_enum_type(typedef_stmt.alias_name);

      Symbol *sym = scope.insert_or_get_tag(SymbolKind::ENUM, typedef_stmt.alias_name, et);

      et = static_cast<EnumType *>(sym->type);

      if (!et->constants.empty()) {
        error(typedef_stmt.token, "Redefinition of enum '" + typedef_stmt.alias_name + "'");
      } else {

        int64_t current_value = 0;
        std::unordered_set<std::string> enumerator_names;

        for (const auto &member : typedef_stmt.anonymous_enum->members) {
          if (!enumerator_names.insert(member.name).second) {
            error(typedef_stmt.token, "Duplicate enumerator '" + member.name + "'");
            continue;
          }

          if (member.value) {
            auto value = evaluate_constant_expr(member.value.get());
            if (!value) {
              error(member.value->token, "Enumerator '" + member.name + "' is not a constant expression");
              continue;
            }
            current_value = *value;
          }

          Symbol constant;
          constant.kind = SymbolKind::ENUM_CONSTANT;
          constant.name = member.name;
          constant.type = et;
          constant.is_defined = true;
          constant.decl_node = &member;
          constant.enum_value = current_value;

          et->constants.push_back({member.name, current_value});

          if (!scope.insert(constant)) {
            error(typedef_stmt.token, "Enumerator '" + member.name + "' already exists");
          }

          ++current_value;
        }
      }
    }

    ParsedType parsed = typedef_stmt.aliased_type;

    if (typedef_stmt.anonymous_struct) {
      parsed.custom_name = typedef_stmt.alias_name;
    }

    if (typedef_stmt.anonymous_union) {
      parsed.custom_name = typedef_stmt.alias_name;
    }

    if (typedef_stmt.anonymous_enum) {
      parsed.custom_name = typedef_stmt.alias_name;
    }

    Type *type = build_type(parsed, scope);

    if (type->kind == TypeKind::ERROR) {
      return false;
    }

    if (type->kind == TypeKind::STRUCT) {
      auto *st = static_cast<StructType *>(type);

      if (st->name.empty()) {
        type_context.rename_struct(st, typedef_stmt.alias_name);
      }
    } else if (type->kind == TypeKind::UNION) {
      auto *ut = static_cast<UnionType *>(type);

      if (ut->name.empty()) {
        type_context.rename_union(ut, typedef_stmt.alias_name);
      }
    } else if (type->kind == TypeKind::ENUM) {
      auto *et = static_cast<EnumType *>(type);

      if (et->name.empty()) {
        type_context.rename_enum(et, typedef_stmt.alias_name);
      }
    }

    Symbol symbol;

    symbol.kind = SymbolKind::TYPEDEF;
    symbol.name = typedef_stmt.alias_name;
    symbol.type = type;

    if (!scope.insert(symbol)) {
      error(typedef_stmt.token, "Redefinition of typedef '" + typedef_stmt.alias_name + "'");
    }

    return false;
  }
  case StmtType::BREAK_STMT: {
    const auto &break_stmt = static_cast<const BreakStmt &>(*stmt);

    if (loop_depth == 0 && switch_depth == 0) {
      error(break_stmt.token, "break statement not inside loop or switch");
    }
    return false;
  }
  case StmtType::CONTINUE_STMT: {
    const auto &continue_stmt = static_cast<const ContinueStmt &>(*stmt);
    if (loop_depth == 0) {
      error(continue_stmt.token, "continue statement not inside loop");
    }
    return false;
  }
  default:
    break;
  }

  return false;
}

Type *Semantics::analyze_expr(Expr *expr, Scope &scope) {
  switch (expr->expr_type()) {
  case ExprType::IDENTIFIER: {
    const IdentifierExpr &var_expr = static_cast<const IdentifierExpr &>(*expr);
    Symbol *sym = scope.lookup_identifier(var_expr.identifier_name);
    if (!sym) {
      error(var_expr.token, "Use of unresolved identifier " + var_expr.identifier_name);
      return &type_context.error_type;
    }

    expr->type = sym->type;

    if (sym->kind == SymbolKind::ENUM_CONSTANT) {
      expr->value_category = ValueCategory::RVALUE;
    } else {
      expr->value_category = ValueCategory::LVALUE;
    }

    return sym->type;
  }
  case ExprType::ASSIGNMENT: {
    const AssignmentExpr &asgn_expr = static_cast<const AssignmentExpr &>(*expr);

    Type *lhs_type = analyze_expr(asgn_expr.lhs.get(), scope);
    Type *rhs_type = default_argument_promotion(decay(analyze_expr(asgn_expr.rhs.get(), scope)));

    if (!is_modifiable_lvalue(asgn_expr.lhs.get())) {
      error(asgn_expr.lhs->token, "Expression is not a modifiable lvalue");
      return &type_context.error_type;
    }

    if (lhs_type->kind == TypeKind::ERROR) {
      return lhs_type;
    }

    if (!can_assign(lhs_type, rhs_type)) {
      error(asgn_expr.rhs->token, "Cannot assign expression of type '" + rhs_type->to_string() + "' to object of type '" + lhs_type->to_string() + "'");
      return &type_context.error_type;
    }

    expr->type = lhs_type;
    expr->value_category = ValueCategory::RVALUE;
    return lhs_type;
  }
  case ExprType::BINARY: {
    const BinaryExpr &binary_expr = static_cast<const BinaryExpr &>(*expr);
    Type *result = analyze_binary_expr(binary_expr, scope);

    expr->type = result;
    expr->value_category = ValueCategory::RVALUE;
    return result;
  }
  case ExprType::UNARY: {
    UnaryExpr &unary_expr = static_cast<UnaryExpr &>(*expr);

    Type *result = analyze_unary_expr(unary_expr, scope);

    expr->type = result;

    return result;
  }
  case ExprType::CAST: {
    auto &cast_expr = static_cast<CastExpr &>(*expr);

    Type *operand_type = decay(analyze_expr(cast_expr.expr.get(), scope));
    Type *target_type = build_type(cast_expr.target_type, scope);

    if (!can_explicitly_cast(operand_type, target_type)) {
      error(cast_expr.expr->token, "Invalid cast");
      return &type_context.error_type;
    }

    expr->type = target_type;
    expr->value_category = ValueCategory::RVALUE;
    return target_type;
  }
  case ExprType::POST_INCREMENT:
  case ExprType::POST_DECREMENT:
  case ExprType::PRE_INCREMENT:
  case ExprType::PRE_DECREMENT: {
    const IncrementExpr &increment_expr = static_cast<const IncrementExpr &>(*expr);
    Type *operand_type = analyze_expr(increment_expr.operand.get(), scope);

    if (operand_type->kind == TypeKind::ARRAY) {
      error(increment_expr.operand->token, "Cannot increment array");
      return &type_context.error_type;
    }

    if (!is_modifiable_lvalue(increment_expr.operand.get())) {
      error(increment_expr.operand->token, "Operand of ++ or -- must be a modifiable lvalue");
      return &type_context.error_type;
    }

    if (is_arithmetic_type(operand_type)) {
      // no problem bc
    } else if (operand_type->kind == TypeKind::POINTER) {
      auto *ptr = static_cast<PointerType *>(operand_type);
      if (contains_void_object(ptr->pointee_type)) {
        error(increment_expr.operand->token, "Cannot increment void *");
        return &type_context.error_type;
      }
    } else {
      error(increment_expr.operand->token, "++ or -- requires arithmetic or pointer type");
      return &type_context.error_type;
    }

    expr->type = operand_type;
    expr->value_category = ValueCategory::RVALUE;
    return operand_type;
  }
  case ExprType::CONDITIONAL: {
    const ConditionalExpr &conditional_expr = static_cast<const ConditionalExpr &>(*expr);
    Type *condition_type = decay(analyze_expr(conditional_expr.condition.get(), scope));

    if (!is_scalar_type(condition_type)) {
      error(conditional_expr.condition->token, "Condition must have scalar type");
    }

    Type *true_type = decay(analyze_expr(conditional_expr.true_expr.get(), scope));
    Type *false_type = decay(analyze_expr(conditional_expr.false_expr.get(), scope));
    Type *result = nullptr;

    if (same_type(true_type, false_type)) {
      result = true_type;
    } else if (is_arithmetic_type(true_type) && is_arithmetic_type(false_type)) {
      result = usual_arithmetic_conversion(true_type, false_type);
    } else if (is_pointer_type(true_type) && is_pointer_type(false_type)) {
      if (!is_comparable_pointer_types(true_type, false_type)) {
        error(conditional_expr.false_expr->token, "Incompatible pointer operands to ?:");
        result = &type_context.error_type;
      } else {
        result = composite_pointer_type(true_type, false_type);
      }
    } else {
      error(conditional_expr.false_expr->token, "Incompatible operands to ?:");
      result = &type_context.error_type;
    }

    expr->type = result;
    expr->value_category = ValueCategory::RVALUE;

    return result;
  }
  case ExprType::FUNCTION_CALL: {
    const FunctionCallExpr &fn_call_expr = static_cast<const FunctionCallExpr &>(*expr);
    Symbol *sym = scope.lookup_identifier(fn_call_expr.function_name);

    if (!sym || (sym->kind != SymbolKind::FUNCTION)) {
      error(fn_call_expr.token, "Call to undefined function '" + fn_call_expr.function_name + "'");
      return &type_context.error_type;
    }

    auto *fn_type = static_cast<FunctionType *>(sym->type);

    if (fn_call_expr.arguments.size() != fn_type->parameter_types.size()) {
      error(fn_call_expr.token, "Wrong number of arguments");
      expr->type = &type_context.error_type;
      return &type_context.error_type;
    }

    for (size_t i = 0; i < fn_call_expr.arguments.size(); i++) {
      Type *arg_type = analyze_expr(fn_call_expr.arguments[i].get(), scope);
      arg_type = default_argument_promotion(decay(arg_type));
      if (!can_assign(fn_type->parameter_types[i], arg_type)) {
        error(fn_call_expr.arguments[i]->token, "Cannot convert expression of type '" + arg_type->to_string() + "' to parameter of type '" + fn_type->parameter_types[i]->to_string() + "'");
        return &type_context.error_type;
      }
    }

    expr->value_category = ValueCategory::RVALUE;
    expr->type = fn_type->return_type;
    return fn_type->return_type;
  }
  case ExprType::ARRAY_ACCESS: {
    const ArrayAccessExpr &arr_access_expr = static_cast<const ArrayAccessExpr &>(*expr);

    Type *base_type = analyze_expr(arr_access_expr.base_expr.get(), scope);
    Type *index_type = decay(analyze_expr(arr_access_expr.index_expr.get(), scope));

    base_type = decay(base_type);
    index_type = integer_promotion(index_type);

    if (!is_integer_type(index_type)) {
      error(arr_access_expr.index_expr->token, "Array index must have integer type");
      return &type_context.error_type;
    }

    if (is_pointer_type(base_type)) {
      auto *ptr = static_cast<PointerType *>(base_type);
      if (contains_void_object(ptr->pointee_type)) {
        error(arr_access_expr.base_expr->token, "Cannot index void *");
        return &type_context.error_type;
      }
      expr->type = ptr->pointee_type;
      expr->value_category = ValueCategory::LVALUE;
      return ptr->pointee_type;
    }

    error(arr_access_expr.base_expr->token, "Subscripted value is not an array or pointer");
    return &type_context.error_type;

    break;
  }
  case ExprType::MEMBER_ACCESS: {
    auto &member_access = static_cast<MemberAccessExpr &>(*expr);

    Type *base_type = analyze_expr(member_access.base_expr.get(), scope);

    if (member_access.op == TokenType::DOT) {
      if (base_type->kind != TypeKind::STRUCT && base_type->kind != TypeKind::UNION) {
        error(member_access.base_expr->token, "'.' requires struct or union operand");
        return &type_context.error_type;
      }
    } else {
      // ->
      base_type = decay(base_type);
      if (!is_pointer_type(base_type)) {
        error(member_access.base_expr->token, "'->' requires pointer operand");
        return &type_context.error_type;
      }
      auto *ptr = static_cast<PointerType *>(base_type);
      base_type = ptr->pointee_type;
      if (base_type->kind != TypeKind::STRUCT && base_type->kind != TypeKind::UNION) {
        error(member_access.base_expr->token, "'->' requires pointer to struct or union");
        return &type_context.error_type;
      }
    }

    if (base_type->kind == TypeKind::STRUCT) {
      auto *st = static_cast<StructType *>(base_type);
      for (const auto &member : st->members) {
        if (member.name == member_access.member_name) {
          expr->type = member.type;
          expr->value_category = ValueCategory::LVALUE;
          return member.type;
        }
      }
      error(member_access.token, "No member named '" + member_access.member_name + "' in struct '" + st->name + "'");
      return &type_context.error_type;
    }

    auto *ut = static_cast<UnionType *>(base_type);

    for (const auto &member : ut->members) {
      if (member.name == member_access.member_name) {
        expr->type = member.type;
        expr->value_category = ValueCategory::LVALUE;
        return member.type;
      }
    }

    error(member_access.token, "No member named '" + member_access.member_name + "' in union '" + ut->name + "'");

    return &type_context.error_type;
  }
  case ExprType::SIZEOF: {
    auto &sizeof_expr = static_cast<SizeofExpr &>(*expr);

    if (sizeof_expr.parsed_type) {
      Type *type = build_type(*sizeof_expr.parsed_type, scope);
      if (!is_complete_type(type)) {
        error(sizeof_expr.token, "sizeof requires a complete object type");
        expr->type = &type_context.error_type;
        expr->value_category = ValueCategory::RVALUE;
        return &type_context.error_type;
      }
    } else {
      Type *type = analyze_expr(sizeof_expr.expr.get(), scope);
      if (!is_complete_type(type)) {
        error(sizeof_expr.token, "sizeof requires a complete object type");
        expr->type = &type_context.error_type;
        expr->value_category = ValueCategory::RVALUE;
        return &type_context.error_type;
      }
    }

    expr->type = &type_context.ulong_type;
    expr->value_category = ValueCategory::RVALUE;

    return expr->type;
  }
  case ExprType::INT_LITERAL:
    expr->type = &type_context.int_type;
    expr->value_category = ValueCategory::RVALUE;
    return &type_context.int_type;
  case ExprType::DOUBLE_LITERAL:
    expr->type = &type_context.double_type;
    expr->value_category = ValueCategory::RVALUE;
    return &type_context.double_type;
  case ExprType::FLOAT_LITERAL:
    expr->type = &type_context.float_type;
    expr->value_category = ValueCategory::RVALUE;
    return &type_context.float_type;
  case ExprType::CHAR_LITERAL:
    expr->type = &type_context.char_type;
    expr->value_category = ValueCategory::RVALUE;
    return &type_context.char_type;
  case ExprType::STRING_LITERAL: {
    PointerType *ptr = type_context.get_pointer_type(&type_context.char_type);
    expr->type = ptr;
    expr->value_category = ValueCategory::RVALUE;
    return ptr;
  }
  default: {
    error(expr->token, "Unknown expression type");
    return &type_context.error_type;
  }
  }
}

Type *Semantics::analyze_binary_expr(const BinaryExpr &binary_expr, Scope &scope) {
  Type *lhs_type = analyze_expr(binary_expr.left_expr.get(), scope);
  Type *rhs_type = analyze_expr(binary_expr.right_expr.get(), scope);

  if (lhs_type->kind == TypeKind::ERROR || rhs_type->kind == TypeKind::ERROR) {
    return &type_context.error_type;
  }

  lhs_type = decay(lhs_type);
  rhs_type = decay(rhs_type);

  switch (binary_expr.op) {
  case TokenType::PLUS: {
    if (lhs_type->kind == TypeKind::POINTER && is_integer_type(integer_promotion(rhs_type))) {
      auto *ptr = static_cast<PointerType *>(lhs_type);
      if (contains_void_object(ptr->pointee_type)) {
        error(binary_expr.left_expr->token, "Pointer arithmetic on void *");
        return &type_context.error_type;
      }

      if (!is_complete_type(ptr->pointee_type)) {
        error(binary_expr.left_expr->token, "Pointer arithmetic requires complete object type");
        return &type_context.error_type;
      }

      return lhs_type;
    }

    if (rhs_type->kind == TypeKind::POINTER && is_integer_type(integer_promotion(lhs_type))) {
      auto *ptr = static_cast<PointerType *>(rhs_type);
      if (contains_void_object(ptr->pointee_type)) {
        error(binary_expr.right_expr->token, "Pointer arithmetic on void *");
        return &type_context.error_type;
      }

      if (!is_complete_type(ptr->pointee_type)) {
        error(binary_expr.right_expr->token, "Pointer arithmetic requires complete object type");
        return &type_context.error_type;
      }

      return rhs_type;
    }

    if (lhs_type->kind == TypeKind::POINTER && rhs_type->kind == TypeKind::POINTER) {
      error(binary_expr.token, "Cannot add two pointers");
      return &type_context.error_type;
    }

    if (lhs_type->kind == TypeKind::POINTER || rhs_type->kind == TypeKind::POINTER) {
      if (lhs_type->kind == TypeKind::POINTER) {
        error(binary_expr.right_expr->token, "Pointer can only be added to an integer");
        return &type_context.error_type;
      }

      if (rhs_type->kind == TypeKind::POINTER) {
        error(binary_expr.left_expr->token, "Pointer can only be added to an integer");
        return &type_context.error_type;
      }
    }

    return usual_arithmetic_conversion(lhs_type, rhs_type);
  }

  case TokenType::MINUS: {
    if (lhs_type->kind == TypeKind::POINTER && is_integer_type(integer_promotion(rhs_type))) {
      auto *ptr = static_cast<PointerType *>(lhs_type);

      if (contains_void_object(ptr->pointee_type)) {
        error(binary_expr.left_expr->token, "Pointer arithmetic on void *");
        return &type_context.error_type;
      }

      if (!is_complete_type(ptr->pointee_type)) {
        error(binary_expr.left_expr->token, "Pointer arithmetic requires complete object type");
        return &type_context.error_type;
      }

      return lhs_type;
    }

    if (lhs_type->kind == TypeKind::POINTER && rhs_type->kind == TypeKind::POINTER) {
      auto *lp = static_cast<PointerType *>(lhs_type);
      auto *rp = static_cast<PointerType *>(rhs_type);

      if (contains_void_object(lp->pointee_type) || contains_void_object(rp->pointee_type)) {
        error(binary_expr.token, "Pointer subtraction on void *");
        return &type_context.error_type;
      }

      if (!is_complete_type(lp->pointee_type) || !is_complete_type(rp->pointee_type)) {
        error(binary_expr.token, "Pointer subtraction requires complete object type");
        return &type_context.error_type;
      }

      if (!compatible_pointer_types(lhs_type, rhs_type)) {
        error(binary_expr.token, "Pointer subtraction requires compatible object types");
        return &type_context.error_type;
      }

      if (!lp->pointee_type->equals(rp->pointee_type)) {
        error(binary_expr.token, "Pointer subtraction requires identical element types");
        return &type_context.error_type;
      }

      return &type_context.long_type;
    }

    if (lhs_type->kind == TypeKind::POINTER || rhs_type->kind == TypeKind::POINTER) {
      if (lhs_type->kind == TypeKind::POINTER) {
        error(binary_expr.right_expr->token, "Pointer can only be subtracted by an integer or compatible pointer");
        return &type_context.error_type;
      }

      if (rhs_type->kind == TypeKind::POINTER) {
        error(binary_expr.left_expr->token, "Only pointers can be subtracted by pointers");
        return &type_context.error_type;
      }
    }

    return usual_arithmetic_conversion(lhs_type, rhs_type);
  }

  case TokenType::MULTIPLY: {
    if (!is_arithmetic_type(lhs_type) || !is_arithmetic_type(rhs_type)) {
      error(binary_expr.token, "'*' requires arithmetic operands");
      return &type_context.error_type;
    }

    lhs_type = integer_promotion(lhs_type);
    rhs_type = integer_promotion(rhs_type);

    return usual_arithmetic_conversion(lhs_type, rhs_type);
  }
  case TokenType::DIVIDE: {
    if (!is_arithmetic_type(lhs_type) || !is_arithmetic_type(rhs_type)) {
      error(binary_expr.token, "'/' requires arithmetic operands");
      return &type_context.error_type;
    }
    lhs_type = integer_promotion(lhs_type);
    rhs_type = integer_promotion(rhs_type);
    return usual_arithmetic_conversion(lhs_type, rhs_type);
  }

  case TokenType::MODULO: {
    if (!is_integer_type(lhs_type) || !is_integer_type(rhs_type)) {
      error(binary_expr.token, "'%' requires integer operands");
      return &type_context.error_type;
    }

    lhs_type = integer_promotion(lhs_type);
    rhs_type = integer_promotion(rhs_type);

    return usual_arithmetic_conversion(lhs_type, rhs_type);
  }
  case TokenType::SMALLER_THAN:
  case TokenType::SMALLER_THAN_EQUAL_THAN:
  case TokenType::GREATER_THAN:
  case TokenType::GREATER_THAN_EQUAL_THAN:
  case TokenType::DOUBLE_EQUALS:
  case TokenType::NOT_EQUALS: {
    if (is_arithmetic_type(lhs_type) && is_arithmetic_type(rhs_type)) {
      lhs_type = usual_arithmetic_conversion(lhs_type, rhs_type);

      if (lhs_type->kind == TypeKind::ERROR) {
        return lhs_type;
      }

      return &type_context.int_type;
    }

    if (lhs_type->kind == TypeKind::POINTER && rhs_type->kind == TypeKind::POINTER) {
      if (!is_comparable_pointer_types(lhs_type, rhs_type)) {
        error(binary_expr.token, "Comparison of incompatible pointers");
        return &type_context.error_type;
      }

      return &type_context.int_type;
    }
    if (lhs_type->kind == TypeKind::POINTER && is_null_pointer_constant(binary_expr.right_expr.get())) {
      return &type_context.int_type;
    }

    if (rhs_type->kind == TypeKind::POINTER && is_null_pointer_constant(binary_expr.left_expr.get())) {
      return &type_context.int_type;
    }

    if (lhs_type->kind == TypeKind::POINTER || rhs_type->kind == TypeKind::POINTER) {
      if (lhs_type->kind == TypeKind::POINTER) {
        error(binary_expr.right_expr->token, "Pointer can only be compared with another pointer or a null pointer constant");
        return &type_context.error_type;
      }

      if (rhs_type->kind == TypeKind::POINTER) {
        error(binary_expr.left_expr->token, "Pointer can only be compared with another pointer or a null pointer constant");
        return &type_context.error_type;
      }
    }

    error(binary_expr.token, "Invalid comparison");
    return &type_context.error_type;
  }
  // logical
  case TokenType::DOUBLE_AMPERSAND:
  case TokenType::DOUBLE_PIPE: {

    if ((!is_scalar_type(lhs_type)) || (!is_scalar_type(rhs_type))) {
      error(binary_expr.token, "Logical operators require scalar operands");
      return &type_context.error_type;
    }

    lhs_type = decay(lhs_type);
    rhs_type = decay(rhs_type);

    return &type_context.int_type;
  }

  case TokenType::LEFT_SHIFT:
  case TokenType::RIGHT_SHIFT: {
    if (!is_integer_type(lhs_type) || !is_integer_type(rhs_type)) {
      error(binary_expr.token, "Shift operators require integer operands");
      return &type_context.error_type;
    }

    lhs_type = integer_promotion(lhs_type);
    rhs_type = integer_promotion(rhs_type);

    if (!is_integer_type(lhs_type) || !is_integer_type(rhs_type)) {
      error(binary_expr.token, "Shift operators require integer operands");
      return &type_context.error_type;
    }

    return lhs_type;
  }

  case TokenType::AMPERSAND:
  case TokenType::PIPE:
  case TokenType::CARET: {
    if (!is_integer_type(lhs_type) || !is_integer_type(rhs_type)) {
      error(binary_expr.token, "Bitwise operators require integer operands");
      return &type_context.error_type;
    }

    lhs_type = integer_promotion(lhs_type);
    rhs_type = integer_promotion(rhs_type);

    return usual_arithmetic_conversion(lhs_type, rhs_type);
  }
  case TokenType::COMMA: {
    return rhs_type;
  }
  default:
    error(binary_expr.token, "Unsupported binary operator");
    return &type_context.error_type;
  }
}

Type *Semantics::analyze_unary_expr(UnaryExpr &unary_expr, Scope &scope) {
  Type *operand = analyze_expr(unary_expr.right_expr.get(), scope);

  if (is_function_type(operand)) {
    operand = type_context.get_pointer_type(operand);
  }

  if (operand->kind == TypeKind::ERROR) {
    return &type_context.error_type;
  }

  switch (unary_expr.op) {
  case TokenType::PLUS:
  case TokenType::MINUS: {
    operand = decay(operand);
    if (!is_arithmetic_type(operand)) {
      error(unary_expr.token, "Unary +/- requires arithmetic operand");
      return &type_context.error_type;
    }

    operand = integer_promotion(operand);
    unary_expr.value_category = ValueCategory::RVALUE;
    return operand;
  }

  case TokenType::EXCLAMATION: {
    operand = decay(operand);
    if (!is_scalar_type(operand)) {
      error(unary_expr.token, "! requires scalar operand");
      return &type_context.error_type;
    }

    unary_expr.value_category = ValueCategory::RVALUE;
    return &type_context.int_type;
  }

  case TokenType::AMPERSAND: {
    if (unary_expr.right_expr->value_category != ValueCategory::LVALUE) {
      error(unary_expr.token, "'&' requires an lvalue");
      return &type_context.error_type;
    }
    unary_expr.value_category = ValueCategory::RVALUE;

    return type_context.get_pointer_type(operand);
  }

  case TokenType::MULTIPLY: {
    if (operand->kind != TypeKind::POINTER) {
      error(unary_expr.token, "Cannot dereference non-pointer");
      return &type_context.error_type;
    }

    auto *ptr = static_cast<PointerType *>(operand);

    if (ptr->pointee_type->kind == TypeKind::FUNCTION) {
      unary_expr.value_category = ValueCategory::LVALUE;
      return ptr->pointee_type;
    }

    if (contains_void_object(ptr->pointee_type)) {
      error(unary_expr.token, "Cannot dereference void*");
      return &type_context.error_type;
    }

    if (!is_complete_type(ptr->pointee_type)) {
      error(unary_expr.token, "Cannot dereference incomplete type");
      return &type_context.error_type;
    }

    unary_expr.value_category = ValueCategory::LVALUE;

    return ptr->pointee_type;
  }

  case TokenType::TILDE: {
    operand = decay(operand);
    if (!is_integer_type(operand)) {
      error(unary_expr.token, "Cannot dereference incomplete type");
      return &type_context.error_type;
    }

    operand = integer_promotion(operand);
    unary_expr.value_category = ValueCategory::RVALUE;
    return operand;
  }

  default: {
    error(unary_expr.token, "Unknown unary operator");
    return &type_context.error_type;
  }
  }
}

void Semantics::analyze_array_initializer(const ArrayInitializer &init, Type *expected_type, Scope &scope) {
  if (init.is_leaf) {
    Type *expr_type = decay(analyze_expr(init.expr.get(), scope));
    if (!can_assign(expected_type, expr_type)) {
      error(init.expr->token, "Invalid array initializer");
    }
    return;
  }
  if (expected_type->kind != TypeKind::ARRAY) {
    error(init.token, "Too many initializer braces");
    return;
  }
  auto *array_type = static_cast<ArrayType *>(expected_type);
  if (init.children.size() > array_type->size) {
    error(init.token, "Too many initializers");
    return;
  }
  for (const auto &child : init.children) {
    analyze_array_initializer(child, array_type->element_type, scope);
  }
}

void Semantics::analyze() {

  // Pass 1
  for (const auto &stmt : program.statements) {
    switch (stmt->stmt_type()) {

    case StmtType::STRUCT_DECL_STMT:
    case StmtType::UNION_DECL_STMT:
    case StmtType::ENUM_DECL_STMT:
    case StmtType::TYPEDEF_DECL_STMT:
      analyze_stmt(stmt.get(), global_scope);
      break;

    default:
      break;
    }
  }

  // Pass 2
  for (const auto &stmt : program.statements) {
    switch (stmt->stmt_type()) {

    case StmtType::STRUCT_DECL_STMT:
    case StmtType::UNION_DECL_STMT:
    case StmtType::ENUM_DECL_STMT:
    case StmtType::TYPEDEF_DECL_STMT:
      break;

    default:
      analyze_stmt(stmt.get(), global_scope);
      break;
    }
  }
}