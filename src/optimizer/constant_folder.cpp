#include "./constant_folder.h"
#include "types/type_utils.h"

#include <cassert>

void ConstantFolder::replace_with_expr(std::unique_ptr<Expr> &dst, std::unique_ptr<Expr> src) {
  if (src->type == nullptr) {
    src->type = dst->type;
  }
  src->value_category = ValueCategory::RVALUE;
  dst = std::move(src);
}

std::unique_ptr<Expr> ConstantFolder::make_constant_expr(const Token &token, Type *type, long double value) {
  auto *builtin = static_cast<BuiltinType *>(type);

  switch (builtin->builtin_kind) {
  case BuiltinKind::CHAR:
    return std::make_unique<CharLetExpr>(token, static_cast<char>(value));

  case BuiltinKind::UCHAR:
    return std::make_unique<CharLetExpr>(token, static_cast<unsigned char>(value));

  case BuiltinKind::SHORT:
  case BuiltinKind::USHORT:
  case BuiltinKind::INT:
  case BuiltinKind::UINT:
  case BuiltinKind::LONG:
  case BuiltinKind::ULONG:
  case BuiltinKind::LONGLONG:
  case BuiltinKind::ULONGLONG:
    return std::make_unique<IntLetExpr>(token, static_cast<int64_t>(value));

  case BuiltinKind::FLOAT:
    return std::make_unique<FloatLetExpr>(token, static_cast<float>(value));

  case BuiltinKind::DOUBLE:
    return std::make_unique<DoubleLetExpr>(token, static_cast<double>(value));

  case BuiltinKind::LONGDOUBLE:
    return std::make_unique<DoubleLetExpr>(token, static_cast<double>(value));

  default:
    assert(false);
    return nullptr;
  }
}

bool ConstantFolder::fold_binary_constants(std::unique_ptr<Expr> &expr_ptr) {
  if (expr_ptr->expr_type() != ExprType::BINARY) {
    return false;
  }

  auto *binary = static_cast<BinaryExpr *>(expr_ptr.get());

  if (!is_constant_literal(binary->left_expr.get()))
    return false;

  if (!is_constant_literal(binary->right_expr.get()))
    return false;

  Type *type = expr_ptr->type;

  long double lhs = 0;
  long double rhs = 0;

  if (is_floating_type(type)) {
    lhs = get_double_value(binary->left_expr.get());
    rhs = get_double_value(binary->right_expr.get());
  } else {
    lhs = get_int_value(binary->left_expr.get());
    rhs = get_int_value(binary->right_expr.get());
  }

  long double result;

  switch (binary->op) {
  case TokenType::PLUS: {
    result = lhs + rhs;
    break;
  }
  case TokenType::MINUS: {
    result = lhs - rhs;
    break;
  }
  case TokenType::MULTIPLY: {
    result = lhs * rhs;
    break;
  }
  case TokenType::DIVIDE: {
    if (rhs == 0) {
      return false;
    }

    if (!is_floating_type(type)) {
      int64_t ilhs = get_int_value(binary->left_expr.get());
      int64_t irhs = get_int_value(binary->right_expr.get());

      if (ilhs == INT64_MIN && irhs == -1) {
        return false;
      }
    }

    result = lhs / rhs;
    break;
  }
  case TokenType::MODULO: {
    int64_t ilhs = get_int_value(binary->left_expr.get());
    int64_t irhs = get_int_value(binary->right_expr.get());

    if (irhs == 0 || (ilhs == INT64_MIN && irhs == -1)) {
      return false;
    }
    result = ilhs % irhs;
    break;
  }
  case TokenType::SMALLER_THAN: {
    result = lhs < rhs;
    break;
  }
  case TokenType::GREATER_THAN: {
    result = lhs > rhs;
    break;
  }
  case TokenType::GREATER_THAN_EQUAL_THAN: {
    result = lhs >= rhs;
    break;
  }
  case TokenType::SMALLER_THAN_EQUAL_THAN: {
    result = lhs <= rhs;
    break;
  }
  case TokenType::DOUBLE_EQUALS: {
    result = lhs == rhs;
    break;
  }
  case TokenType::NOT_EQUALS: {
    result = lhs != rhs;
    break;
  }
  case TokenType::DOUBLE_AMPERSAND: {
    result = lhs && rhs;
    break;
  }
  case TokenType::DOUBLE_PIPE: {
    result = lhs || rhs;
    break;
  }
  case TokenType::AMPERSAND: {
    int64_t ilhs = get_int_value(binary->left_expr.get());
    int64_t irhs = get_int_value(binary->right_expr.get());

    result = ilhs & irhs;

    break;
  }
  case TokenType::PIPE: {
    int64_t ilhs = get_int_value(binary->left_expr.get());
    int64_t irhs = get_int_value(binary->right_expr.get());

    result = ilhs | irhs;
    break;
  }
  case TokenType::LEFT_SHIFT: {
    int64_t ilhs = get_int_value(binary->left_expr.get());
    int64_t irhs = get_int_value(binary->right_expr.get());

    if ((irhs < 0) || (irhs >= static_cast<int64_t>(sizeof(int64_t) * CHAR_BIT))) {
      return false;
    }
    result = ilhs << irhs;
    break;
  }
  case TokenType::RIGHT_SHIFT: {
    int64_t ilhs = get_int_value(binary->left_expr.get());
    int64_t irhs = get_int_value(binary->right_expr.get());

    if ((irhs < 0) || (irhs >= static_cast<int64_t>(sizeof(int64_t) * CHAR_BIT))) {
      return false;
    }

    result = ilhs >> irhs;
    break;
  }
  case TokenType::CARET: {
    int64_t ilhs = get_int_value(binary->left_expr.get());
    int64_t irhs = get_int_value(binary->right_expr.get());

    result = ilhs ^ irhs;
    break;
  }
  case TokenType::COMMA: {
    result = rhs;
    break;
  }

  default: {
    return false;
  }
  }

  replace_with_expr(expr_ptr, make_constant_expr(expr_ptr->token, expr_ptr->type, result));
  return true;
}

bool ConstantFolder::is_int_literal(const Expr *expr_ptr, int64_t value) {
  if (expr_ptr->expr_type() != ExprType::INT_LITERAL) {
    return false;
  }
  return ((static_cast<const IntLetExpr *>(expr_ptr))->value == value);
}

bool ConstantFolder::is_zero_literal(const Expr *expr) {
  if (!is_constant_literal(expr)) {
    return false;
  }

  if (is_floating_type(expr->type)) {
    return get_double_value(expr) == 0.0;
  }

  return get_int_value(expr) == 0;
}

bool ConstantFolder::is_one_literal(const Expr *expr) {
  if (!is_constant_literal(expr)) {
    return false;
  }

  if (is_floating_type(expr->type)) {
    return get_double_value(expr) == 1.0;
  }

  return get_int_value(expr) == 1;
}

bool ConstantFolder::is_constant_literal(const Expr *expr) {
  switch (expr->expr_type()) {
  case ExprType::INT_LITERAL:
  case ExprType::FLOAT_LITERAL:
  case ExprType::DOUBLE_LITERAL:
  case ExprType::CHAR_LITERAL:
    return true;

  default:
    return false;
  }
}

int64_t ConstantFolder::get_int_value(const Expr *expr) {
  switch (expr->expr_type()) {
  case ExprType::INT_LITERAL:
    return static_cast<const IntLetExpr *>(expr)->value;

  case ExprType::CHAR_LITERAL:
    return static_cast<const CharLetExpr *>(expr)->value;

  case ExprType::FLOAT_LITERAL:
    return static_cast<int64_t>(static_cast<const FloatLetExpr *>(expr)->value);

  case ExprType::DOUBLE_LITERAL:
    return static_cast<int64_t>(static_cast<const DoubleLetExpr *>(expr)->value);

  default:
    assert(false && "Invalid constant literal");
    return 0;
  }
}

float ConstantFolder::get_float_value(const Expr *expr) {
  switch (expr->expr_type()) {
  case ExprType::INT_LITERAL:
    return static_cast<float>(static_cast<const IntLetExpr *>(expr)->value);

  case ExprType::CHAR_LITERAL:
    return static_cast<float>(static_cast<const CharLetExpr *>(expr)->value);

  case ExprType::FLOAT_LITERAL:
    return static_cast<const FloatLetExpr *>(expr)->value;

  case ExprType::DOUBLE_LITERAL:
    return static_cast<float>(static_cast<const DoubleLetExpr *>(expr)->value);

  default:
    assert(false && "Invalid constant literal");
    return 0;
  }
}

double ConstantFolder::get_double_value(const Expr *expr) {
  switch (expr->expr_type()) {
  case ExprType::INT_LITERAL:
    return static_cast<double>(static_cast<const IntLetExpr *>(expr)->value);

  case ExprType::CHAR_LITERAL:
    return static_cast<double>(static_cast<const CharLetExpr *>(expr)->value);

  case ExprType::FLOAT_LITERAL:
    return static_cast<double>(static_cast<const FloatLetExpr *>(expr)->value);

  case ExprType::DOUBLE_LITERAL:
    return static_cast<const DoubleLetExpr *>(expr)->value;

  default:
    assert(false && "Invalid constant literal");
    return 0;
  }
}

bool ConstantFolder::fold_algebraic(std::unique_ptr<Expr> &expr_ptr) {
  if (expr_ptr->expr_type() != ExprType::BINARY) {
    return false;
  }

  auto *binary_expr = static_cast<BinaryExpr *>(expr_ptr.get());

  Expr *lhs = binary_expr->left_expr.get();
  Expr *rhs = binary_expr->right_expr.get();

  switch (binary_expr->op) {
  case TokenType::PLUS: {
    if (is_zero_literal(lhs)) {
      expr_ptr = std::move(binary_expr->right_expr);
      return true;
    }

    if (is_zero_literal(rhs)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }

    return false;
  }
  case TokenType::MINUS: {
    if (is_zero_literal(rhs)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }

    return false;
  }
  case TokenType::MULTIPLY: {
    if (is_one_literal(lhs)) {
      expr_ptr = std::move(binary_expr->right_expr);
      return true;
    }
    if (is_one_literal(rhs)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }

    if (is_zero_literal(lhs)) {
      replace_with_expr(expr_ptr, make_constant_expr(expr_ptr->token, expr_ptr->type, 0));
      return true;
    }
    if (is_zero_literal(rhs)) {
      replace_with_expr(expr_ptr, make_constant_expr(expr_ptr->token, expr_ptr->type, 0));
      return true;
    }

    return false;
  }
  case TokenType::DIVIDE: {
    if (is_one_literal(rhs)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }
    if (is_zero_literal(lhs) && !is_zero_literal(rhs)) {
      replace_with_expr(expr_ptr, make_constant_expr(expr_ptr->token, expr_ptr->type, 0));
      return true;
    }
    return false;
  }
  case TokenType::MODULO: {
    if (is_one_literal(rhs)) {
      replace_with_expr(expr_ptr, std::make_unique<IntLetExpr>(expr_ptr->token, 0));
      return true;
    }
    return false;
  }
  case TokenType::PIPE: {
    if (is_zero_literal(lhs)) {
      expr_ptr = std::move(binary_expr->right_expr);
      return true;
    }
    if (is_zero_literal(rhs)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }
    if (is_int_literal(lhs, -1) || is_int_literal(rhs, -1)) {
      replace_with_expr(expr_ptr, std::make_unique<IntLetExpr>(expr_ptr->token, -1));
      return true;
    }
    return false;
  }
  case TokenType::CARET: {
    if (is_zero_literal(lhs)) {
      expr_ptr = std::move(binary_expr->right_expr);
      return true;
    }
    if (is_zero_literal(rhs)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }
    return false;
  }
  case TokenType::AMPERSAND: {
    if (is_int_literal(lhs, -1)) {
      expr_ptr = std::move(binary_expr->right_expr);
      return true;
    }
    if (is_int_literal(rhs, -1)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }
    if (is_zero_literal(lhs) || is_zero_literal(rhs)) {
      replace_with_expr(expr_ptr, make_constant_expr(expr_ptr->token, expr_ptr->type, 0));
      return true;
    }

    return false;
  }
  case TokenType::LEFT_SHIFT: {
    if (is_zero_literal(rhs)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }
    return false;
  }
  case TokenType::RIGHT_SHIFT: {
    if (is_zero_literal(rhs)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }
    return false;
  }
  case TokenType::DOUBLE_AMPERSAND: {
    if (is_zero_literal(lhs) || is_zero_literal(rhs)) {
      replace_with_expr(expr_ptr, make_constant_expr(expr_ptr->token, expr_ptr->type, 0));
      return true;
    }
    if (is_one_literal(lhs)) {
      expr_ptr = std::move(binary_expr->right_expr);
      return true;
    }
    if (is_one_literal(rhs)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }
    return false;
  }
  case TokenType::DOUBLE_PIPE: {
    if (is_one_literal(lhs) || is_one_literal(rhs)) {
      replace_with_expr(expr_ptr, make_constant_expr(expr_ptr->token, expr_ptr->type, 1));
      return true;
    }
    if (is_zero_literal(lhs)) {
      expr_ptr = std::move(binary_expr->right_expr);
      return true;
    }
    if (is_zero_literal(rhs)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }
    return false;
  }
  default: {
    return false;
  }
  }
}

bool ConstantFolder::fold_unary(std::unique_ptr<Expr> &expr_ptr) {
  if (expr_ptr->expr_type() != ExprType::UNARY) {
    return false;
  }

  auto *unary = static_cast<UnaryExpr *>(expr_ptr.get());

  if (!is_constant_literal(unary->right_expr.get())) {
    return false;
  }

  Type *type = unary->type;

  long double value;

  if (is_floating_type(type)) {
    value = get_double_value(unary->right_expr.get());
  } else {
    value = get_int_value(unary->right_expr.get());
  }

  switch (unary->op) {
  case TokenType::PLUS: {
    replace_with_expr(expr_ptr, make_constant_expr(expr_ptr->token, type, value));
    return true;
  }
  case TokenType::MINUS: {
    replace_with_expr(expr_ptr, make_constant_expr(expr_ptr->token, type, -value));
    return true;
  }
  case TokenType::EXCLAMATION: {
    replace_with_expr(expr_ptr, make_constant_expr(expr_ptr->token, expr_ptr->type, !value));
    return true;
  }
  case TokenType::TILDE: {
    if (is_floating_type(type)) {
      return false;
    }

    replace_with_expr(expr_ptr, make_constant_expr(expr_ptr->token, type, ~get_int_value(unary->right_expr.get())));
    return true;
  }
  default: {
    return false;
  }
  }
}

bool ConstantFolder::fold_conditional(std::unique_ptr<Expr> &expr_ptr) {
  if (expr_ptr->expr_type() != ExprType::CONDITIONAL) {
    return false;
  }

  auto *conditional = static_cast<ConditionalExpr *>(expr_ptr.get());

  if (!is_constant_literal(conditional->condition.get())) {
    return false;
  }

  bool condition;

  if (is_floating_type(conditional->condition->type)) {
    condition = get_double_value(conditional->condition.get()) != 0.0;
  } else {
    condition = get_int_value(conditional->condition.get()) != 0;
  }

  if (condition) {
    expr_ptr = std::move(conditional->true_expr);
  } else {
    expr_ptr = std::move(conditional->false_expr);
  }

  return true;
}

bool ConstantFolder::fold_sizeof(std::unique_ptr<Expr> &expr_ptr) {
  if (expr_ptr->expr_type() != ExprType::SIZEOF) {
    return false;
  }

  auto *sizeof_expr = static_cast<SizeofExpr *>(expr_ptr.get());

  if (!is_complete_type(sizeof_expr->type)) {
    return false;
  }

  replace_with_expr(expr_ptr, std::make_unique<IntLetExpr>(expr_ptr->token, sizeof_type(sizeof_expr->type)));

  return true;
}

bool ConstantFolder::fold_cast(std::unique_ptr<Expr> &expr_ptr) {
  if (expr_ptr->expr_type() != ExprType::CAST) {
    return false;
  }

  auto *cast_expr = static_cast<CastExpr *>(expr_ptr.get());

  if (!is_constant_literal(cast_expr->expr.get())) {
    return false;
  }

  if (cast_expr->type->kind != TypeKind::BUILTIN) {
    return false;
  }

  long double value;

  if (is_floating_type(cast_expr->expr->type)) {
    value = get_double_value(cast_expr->expr.get());
  } else {
    value = get_int_value(cast_expr->expr.get());
  }

  replace_with_expr(expr_ptr, make_constant_expr(expr_ptr->token, cast_expr->type, value));

  return true;
}

bool ConstantFolder::optimize_array_initializer(ArrayInitializer &arr_initializer) {
  bool changed = false;

  for (auto &child : arr_initializer.children) {
    if (child.is_leaf) {
      changed |= optimize_expr(child.expr);
    } else {
      changed |= optimize_array_initializer(child);
    }
  }

  return changed;
}

bool ConstantFolder::optimize_stmt(Stmt *stmt_ptr) {
  if (!stmt_ptr) {
    return false;
  }

  bool changed = false;

  switch (stmt_ptr->stmt_type()) {
  case StmtType::VARIABLE_DECL_STMT: {
    VariableDeclarationStmt &vd_stmt = static_cast<VariableDeclarationStmt &>(*stmt_ptr);
    if (vd_stmt.expr_ptr) {
      changed |= optimize_expr(vd_stmt.expr_ptr);
    }
    if (vd_stmt.init) {
      changed |= optimize_array_initializer(*vd_stmt.init);
    }
    break;
  }
  case StmtType::GLOBAL_VARIABLE_DECL_STMT: {
    GlobalVariableDeclStmt &glbl_stmt = static_cast<GlobalVariableDeclStmt &>(*stmt_ptr);
    if (glbl_stmt.declaration->initializer) {
      changed |= optimize_expr(glbl_stmt.declaration->initializer);
    }
    if (glbl_stmt.declaration->array_initializer.has_value()) {
      changed |= optimize_array_initializer(*glbl_stmt.declaration->array_initializer);
    }
    break;
  }
  case StmtType::FUNCTION_DECL_STMT: {
    auto &fn = static_cast<FunctionDeclStmt &>(*stmt_ptr);

    if (fn.declaration->body) {
      changed |= optimize_stmt(fn.declaration->body.get());
    }

    break;
  }
  case StmtType::BLOCK_STMT: {
    BlockStmt &block_stmt = static_cast<BlockStmt &>(*stmt_ptr);
    for (auto &stmt : block_stmt.statements) {
      changed |= optimize_stmt(stmt.get());
    }
    break;
  }
  case StmtType::EXPRESSION_STMT: {
    ExpressionStmt &expr_stmt = static_cast<ExpressionStmt &>(*stmt_ptr);
    if (expr_stmt.expr) {
      changed |= optimize_expr(expr_stmt.expr);
    }
    break;
  }
  case StmtType::RETURN_STMT: {
    ReturnStmt &return_stmt = static_cast<ReturnStmt &>(*stmt_ptr);
    if (return_stmt.expr_ptr) {
      changed |= optimize_expr(return_stmt.expr_ptr);
    }
    break;
  }
  case StmtType::IF_STMT: {
    IfStmt &if_stmt = static_cast<IfStmt &>(*stmt_ptr);
    if (if_stmt.condition) {
      changed |= optimize_expr(if_stmt.condition);
    }
    if (if_stmt.then_body) {
      changed |= optimize_stmt(if_stmt.then_body.get());
    }
    if (if_stmt.else_body) {
      changed |= optimize_stmt(if_stmt.else_body.get());
    }
    break;
  }
  case StmtType::WHILE_STMT: {
    WhileStmt &while_stmt = static_cast<WhileStmt &>(*stmt_ptr);
    if (while_stmt.condition) {
      changed |= optimize_expr(while_stmt.condition);
    }
    if (while_stmt.body) {
      changed |= optimize_stmt(while_stmt.body.get());
    }
    break;
  }
  case StmtType::DO_WHILE_STMT: {
    DoWhileStmt &do_while_stmt = static_cast<DoWhileStmt &>(*stmt_ptr);
    if (do_while_stmt.condition) {
      changed |= optimize_expr(do_while_stmt.condition);
    }
    if (do_while_stmt.body) {
      changed |= optimize_stmt(do_while_stmt.body.get());
    }
    break;
  }
  case StmtType::FOR_STMT: {
    ForStmt &for_stmt = static_cast<ForStmt &>(*stmt_ptr);
    if (for_stmt.condition) {
      changed |= optimize_expr(for_stmt.condition);
    }
    if (for_stmt.init_stmt) {
      changed |= optimize_stmt(for_stmt.init_stmt.get());
    }
    if (for_stmt.update_stmt) {
      changed |= optimize_stmt(for_stmt.update_stmt.get());
    }
    if (for_stmt.body) {
      changed |= optimize_stmt(for_stmt.body.get());
    }
    break;
  }
  case StmtType::SWITCH_STMT: {
    SwitchStmt &switch_stmt = static_cast<SwitchStmt &>(*stmt_ptr);
    if (switch_stmt.condition) {
      changed |= optimize_expr(switch_stmt.condition);
    }
    for (auto &_case : switch_stmt.cases) {
      changed |= optimize_expr(_case.value);
      if (_case.body) {
        changed |= optimize_stmt(_case.body.get());
      }
    }
    if (switch_stmt.default_body) {
      changed |= optimize_stmt(switch_stmt.default_body.get());
    }
    break;
  }
  case StmtType::BREAK_STMT:
  case StmtType::CONTINUE_STMT:
  case StmtType::EMPTY_STMT:
  default: {
    break;
  }
  }

  return changed;
}

bool ConstantFolder::optimize_expr(std::unique_ptr<Expr> &expr_ptr) {
  if (!expr_ptr) {
    return false;
  }
  bool changed = false;

  switch (expr_ptr->expr_type()) {
  case ExprType::BINARY: {
    auto &expr = static_cast<BinaryExpr &>(*expr_ptr);
    changed |= optimize_expr(expr.left_expr);
    changed |= optimize_expr(expr.right_expr);
    break;
  }
  case ExprType::UNARY: {
    auto &expr = static_cast<UnaryExpr &>(*expr_ptr);
    changed |= optimize_expr(expr.right_expr);
    break;
  }
  case ExprType::PRE_INCREMENT:
  case ExprType::PRE_DECREMENT:
  case ExprType::POST_INCREMENT:
  case ExprType::POST_DECREMENT: {
    auto &expr = static_cast<IncrementExpr &>(*expr_ptr);
    changed |= optimize_expr(expr.operand);
    break;
  }
  case ExprType::FUNCTION_CALL: {
    auto &expr = static_cast<FunctionCallExpr &>(*expr_ptr);
    for (auto &arg : expr.arguments) {
      changed |= optimize_expr(arg);
    }
    break;
  }
  case ExprType::CAST: {
    auto &expr = static_cast<CastExpr &>(*expr_ptr);
    changed |= optimize_expr(expr.expr);
    break;
  }
  case ExprType::ARRAY_ACCESS: {
    auto &expr = static_cast<ArrayAccessExpr &>(*expr_ptr);
    changed |= optimize_expr(expr.base_expr);
    changed |= optimize_expr(expr.index_expr);
    break;
  }
  case ExprType::ASSIGNMENT: {
    auto &expr = static_cast<AssignmentExpr &>(*expr_ptr);
    changed |= optimize_expr(expr.lhs);
    changed |= optimize_expr(expr.rhs);
    break;
  }
  case ExprType::CONDITIONAL: {
    auto &expr = static_cast<ConditionalExpr &>(*expr_ptr);
    changed |= optimize_expr(expr.condition);
    changed |= optimize_expr(expr.true_expr);
    changed |= optimize_expr(expr.false_expr);
    break;
  }
  case ExprType::MEMBER_ACCESS: {
    auto &expr = static_cast<MemberAccessExpr &>(*expr_ptr);
    changed |= optimize_expr(expr.base_expr);
    break;
  }
  case ExprType::SIZEOF: {
    auto &expr = static_cast<SizeofExpr &>(*expr_ptr);

    if (expr.expr) {
      changed |= optimize_expr(expr.expr);
    }
    break;
  }
  case ExprType::INT_LITERAL:
  case ExprType::DOUBLE_LITERAL:
  case ExprType::FLOAT_LITERAL:
  case ExprType::CHAR_LITERAL:
  case ExprType::STRING_LITERAL:
  case ExprType::IDENTIFIER:
  default:
    break;
  }

  // FOLDING Routines

  switch (expr_ptr->expr_type()) {
  case ExprType::BINARY: {
    if (fold_binary_constants(expr_ptr)) {
      return true;
    }
    if (fold_algebraic(expr_ptr)) {
      return true;
    }
    break;
  }
  case ExprType::UNARY: {
    if (fold_unary(expr_ptr)) {
      return true;
    }
    break;
  }
  case ExprType::CONDITIONAL: {
    if (fold_conditional(expr_ptr)) {
      return true;
    }
    break;
  }
  case ExprType::SIZEOF: {
    if (fold_sizeof(expr_ptr)) {
      return true;
    }
    break;
  }
  case ExprType::CAST: {
    if (fold_cast(expr_ptr)) {
      return true;
    }
    break;
  }
  default: {
    break;
  }
  }

  return changed;
}

bool ConstantFolder::optimize() {
  bool changed = false;

  for (std::unique_ptr<Stmt> &stmt : program.statements) {
    changed |= optimize_stmt(stmt.get());
  }

  return changed;
}