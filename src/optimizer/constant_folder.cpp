#include "./constant_folder.h"

void ConstantFolder::replace_with_int(std::unique_ptr<Expr> &expr_ptr, int64_t value) {
  auto replacement = std::make_unique<IntLetExpr>(expr_ptr->token, value);
  replacement->type = expr_ptr->type;
  replacement->value_category = ValueCategory::RVALUE;
  expr_ptr = std::move(replacement);
}

bool ConstantFolder::fold_binary_constants(std::unique_ptr<Expr> &expr_ptr) {
  if (expr_ptr->expr_type() != ExprType::BINARY) {
    return false;
  }

  auto *binary = static_cast<BinaryExpr *>(expr_ptr.get());

  if (binary->left_expr->expr_type() != ExprType::INT_LITERAL)
    return false;

  if (binary->right_expr->expr_type() != ExprType::INT_LITERAL)
    return false;

  auto *lhs = static_cast<IntLetExpr *>(binary->left_expr.get());
  auto *rhs = static_cast<IntLetExpr *>(binary->right_expr.get());

  int64_t result;

  switch (binary->op) {
  case TokenType::PLUS: {
    result = lhs->value + rhs->value;
    break;
  }
  case TokenType::MINUS: {
    result = lhs->value - rhs->value;
    break;
  }
  case TokenType::MULTIPLY: {
    result = lhs->value * rhs->value;
    break;
  }
  case TokenType::DIVIDE: {
    if (rhs->value == 0 || (lhs->value == INT64_MIN && rhs->value == -1)) {
      return false;
    }
    result = lhs->value / rhs->value;
    break;
  }
  case TokenType::MODULO: {
    if (rhs->value == 0 || (lhs->value == INT64_MIN && rhs->value == -1)) {
      return false;
    }
    result = lhs->value % rhs->value;
    break;
  }
  case TokenType::SMALLER_THAN: {
    result = lhs->value < rhs->value;
    break;
  }
  case TokenType::GREATER_THAN: {
    result = lhs->value > rhs->value;
    break;
  }
  case TokenType::GREATER_THAN_EQUAL_THAN: {
    result = lhs->value >= rhs->value;
    break;
  }
  case TokenType::SMALLER_THAN_EQUAL_THAN: {
    result = lhs->value <= rhs->value;
    break;
  }
  case TokenType::DOUBLE_EQUALS: {
    result = lhs->value == rhs->value;
    break;
  }
  case TokenType::NOT_EQUALS: {
    result = lhs->value != rhs->value;
    break;
  }
  case TokenType::DOUBLE_AMPERSAND: {
    result = lhs->value && rhs->value;
    break;
  }
  case TokenType::DOUBLE_PIPE: {
    result = lhs->value || rhs->value;
    break;
  }
  case TokenType::AMPERSAND: {
    result = lhs->value & rhs->value;
    break;
  }
  case TokenType::PIPE: {
    result = lhs->value | rhs->value;
    break;
  }
  case TokenType::LEFT_SHIFT: {
    if ((rhs->value < 0) || (rhs->value >= 64)) {
      return false;
    }
    result = lhs->value << rhs->value;
    break;
  }
  case TokenType::RIGHT_SHIFT: {
    if ((rhs->value < 0) || (rhs->value >= 64)) {
      return false;
    }
    result = lhs->value >> rhs->value;
    break;
  }
  case TokenType::CARET: {
    result = lhs->value ^ rhs->value;
    break;
  }

  default: {
    return false;
  }
  }

  replace_with_int(expr_ptr, result);
  return true;
}

bool ConstantFolder::is_int_literal(const Expr *expr_ptr, int64_t value) {
  if (expr_ptr->expr_type() != ExprType::INT_LITERAL) {
    return false;
  }
  return ((static_cast<const IntLetExpr *>(expr_ptr))->value == value);
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
    if (is_int_literal(lhs, 0)) {
      expr_ptr = std::move(binary_expr->right_expr);
      return true;
    }

    if (is_int_literal(rhs, 0)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }

    return false;
  }
  case TokenType::MINUS: {
    if (is_int_literal(rhs, 0)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }

    return false;
  }
  case TokenType::MULTIPLY: {
    if (is_int_literal(lhs, 1)) {
      expr_ptr = std::move(binary_expr->right_expr);
      return true;
    }
    if (is_int_literal(rhs, 1)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }
    if (is_int_literal(lhs, 0)) {
      replace_with_int(expr_ptr, 0);
      return true;
    }
    if (is_int_literal(rhs, 0)) {
      replace_with_int(expr_ptr, 0);
      return true;
    }

    return false;
  }
  case TokenType::DIVIDE: {
    if (is_int_literal(rhs, 1)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }
    return false;
  }
  case TokenType::MODULO: {
    if (is_int_literal(rhs, 1)) {
      replace_with_int(expr_ptr, 0);
      return true;
    }
    return false;
  }
  case TokenType::PIPE: {
    if (is_int_literal(lhs, 0)) {
      expr_ptr = std::move(binary_expr->right_expr);
      return true;
    }
    if (is_int_literal(rhs, 0)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }
    return false;
  }
  case TokenType::CARET: {
    if (is_int_literal(lhs, 0)) {
      expr_ptr = std::move(binary_expr->right_expr);
      return true;
    }
    if (is_int_literal(rhs, 0)) {
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
    return false;
  }
  case TokenType::LEFT_SHIFT: {
    if (is_int_literal(rhs, 0)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }
    return false;
  }
  case TokenType::RIGHT_SHIFT: {
    if (is_int_literal(rhs, 0)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }
    return false;
  }
  case TokenType::DOUBLE_AMPERSAND: {
    if (is_int_literal(lhs, 0) || is_int_literal(rhs, 0)) {
      replace_with_int(expr_ptr, 0);
      return true;
    }
    if (is_int_literal(lhs, 1)) {
      expr_ptr = std::move(binary_expr->right_expr);
      return true;
    }
    if (is_int_literal(rhs, 1)) {
      expr_ptr = std::move(binary_expr->left_expr);
      return true;
    }
    return false;
  }
  case TokenType::DOUBLE_PIPE: {
    if (is_int_literal(lhs, 1) || is_int_literal(rhs, 1)) {
      replace_with_int(expr_ptr, 1);
      return true;
    }
    if (is_int_literal(lhs, 0)) {
      expr_ptr = std::move(binary_expr->right_expr);
      return true;
    }
    if (is_int_literal(rhs, 0)) {
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

  if (unary->right_expr->expr_type() != ExprType::INT_LITERAL) {
    return false;
  }

  auto *operand = static_cast<IntLetExpr *>(unary->right_expr.get());

  switch (unary->op) {
  case TokenType::PLUS: {
    expr_ptr = std::move(unary->right_expr);
    return true;
  }
  case TokenType::MINUS: {
    if (operand->value == INT64_MIN) {
      return false;
    }

    replace_with_int(expr_ptr, -(operand->value));
    return true;
  }
  case TokenType::EXCLAMATION: {
    replace_with_int(expr_ptr, !(operand->value));
    return true;
  }
  case TokenType::TILDE: {
    replace_with_int(expr_ptr, ~(operand->value));
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

  if (conditional->condition->expr_type() != ExprType::INT_LITERAL) {
    return false;
  }

  auto *value = static_cast<IntLetExpr *>(conditional->condition.get());

  if (value->value != 0) {
    expr_ptr = std::move(conditional->true_expr);
  } else {
    expr_ptr = std::move(conditional->false_expr);
  }

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
  case ExprType::BINARY:
    if (fold_binary_constants(expr_ptr)) {
      return true;
    }
    if (fold_algebraic(expr_ptr)) {
      return true;
    }
    break;
  case ExprType::UNARY:
    if (fold_unary(expr_ptr)) {
      return true;
    }
    break;
  case ExprType::CONDITIONAL:
    if (fold_conditional(expr_ptr)) {
      return true;
    }
    break;
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