#include "./dead_code_eliminator.h"

bool DeadCodeEliminator::optimize_block_stmt(BlockStmt *block_stmt_ptr) {
  if (!block_stmt_ptr) {
    return false;
  }

  bool changed = false;

  for (auto &stmt : block_stmt_ptr->statements) {
    changed |= optimize_stmt(stmt);
  }

  for (size_t i = 0; i < block_stmt_ptr->statements.size(); i++) {
    StmtType stmt_type = block_stmt_ptr->statements[i]->stmt_type();
    if (stmt_type == StmtType::RETURN_STMT || stmt_type == StmtType::BREAK_STMT || stmt_type == StmtType::CONTINUE_STMT) {
      if (i + 1 >= block_stmt_ptr->statements.size()) {
        break;
      }

      block_stmt_ptr->statements.erase(block_stmt_ptr->statements.begin() + i + 1, block_stmt_ptr->statements.end());
      changed = true;
      break;
    }
  }

  return changed;
}

bool DeadCodeEliminator::optimize_stmt(std::unique_ptr<Stmt> &stmt_ptr) {
  if (!stmt_ptr) {
    return false;
  }

  bool changed = false;

  switch (stmt_ptr->stmt_type()) {
  case StmtType::BLOCK_STMT: {
    changed |= optimize_block_stmt(static_cast<BlockStmt *>(stmt_ptr.get()));

    break;
  }
  case StmtType::FUNCTION_DECL_STMT: {
    auto &fn = static_cast<FunctionDeclStmt &>(*stmt_ptr);

    if (fn.declaration->body) {
      changed |= optimize_block_stmt(fn.declaration->body.get());
    }

    break;
  }
  case StmtType::WHILE_STMT: {
    auto &loop = static_cast<WhileStmt &>(*stmt_ptr);

    if (loop.body) {
      changed |= optimize_block_stmt(loop.body.get());
    }

    if (loop.condition && loop.condition->expr_type() == ExprType::INT_LITERAL) {
      auto *condition = static_cast<IntLetExpr *>(loop.condition.get());
      if (condition->value == 0) {
        stmt_ptr = std::make_unique<EmptyStmt>();
        return true;
      }
    }

    break;
  }
  case StmtType::DO_WHILE_STMT: {
    auto &loop = static_cast<DoWhileStmt &>(*stmt_ptr);

    if (loop.body) {
      changed |= optimize_block_stmt(loop.body.get());
    }

    if (loop.condition && loop.condition->expr_type() == ExprType::INT_LITERAL) {
      auto *condition = static_cast<IntLetExpr *>(loop.condition.get());
      if (condition->value == 0) {
        stmt_ptr = std::move(loop.body);
        return true;
      }
    }

    break;
  }
  case StmtType::FOR_STMT: {
    auto &loop = static_cast<ForStmt &>(*stmt_ptr);

    if (loop.init_stmt) {
      changed |= optimize_stmt(loop.init_stmt);
    }
    if (loop.update_stmt) {
      changed |= optimize_stmt(loop.update_stmt);
    }
    if (loop.body) {
      changed |= optimize_block_stmt(loop.body.get());
    }

    if (loop.condition && loop.condition->expr_type() == ExprType::INT_LITERAL) {
      auto *condition = static_cast<IntLetExpr *>(loop.condition.get());
      if (condition->value == 0) {
        auto block = std::make_unique<BlockStmt>();
        if (loop.init_stmt) {
          block->statements.push_back(std::move(loop.init_stmt));
        }

        stmt_ptr = std::move(block);
        return true;
      }
    }
    break;
  }
  case StmtType::SWITCH_STMT: {
    auto &switch_stmt = static_cast<SwitchStmt &>(*stmt_ptr);
    for (auto &cse : switch_stmt.cases) {
      if (cse.body) {
        changed |= optimize_block_stmt(cse.body.get());
      }
    }

    if (switch_stmt.default_body) {
      changed |= optimize_block_stmt(switch_stmt.default_body.get());
    }

    break;
  }
  case StmtType::IF_STMT: {
    auto &if_stmt = static_cast<IfStmt &>(*stmt_ptr);

    if (if_stmt.then_body) {
      changed |= optimize_block_stmt(if_stmt.then_body.get());
    }
    if (if_stmt.else_body) {
      changed |= optimize_block_stmt(if_stmt.else_body.get());
    }
    if (!if_stmt.condition || if_stmt.condition->expr_type() != ExprType::INT_LITERAL) {
      return changed;
    }

    auto *cond = static_cast<IntLetExpr *>(if_stmt.condition.get());

    if (cond->value != 0) {
      stmt_ptr = std::move(if_stmt.then_body);
      return true;
    } else {
      if (if_stmt.else_body) {
        stmt_ptr = std::move(if_stmt.else_body);
      } else {
        stmt_ptr = std::make_unique<EmptyStmt>();
      }
      return true;
    }
  }
  default: {
    return changed;
  }
  }

  return changed;
}

bool DeadCodeEliminator::optimize() {
  bool changed = false;
  for (auto &stmt : program.statements) {
    changed |= optimize_stmt(stmt);
  }
  return changed;
}