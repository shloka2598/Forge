#pragma once

#include "../lexer/lexer.h"
#include "../types/Datatypes.h"
#include "./ArrayInitializer.h"
#include "./custom_types.h"
#include "./exprs.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

enum class StmtType {
  EMPTY_STMT,
  RETURN_STMT,
  VARIABLE_DECL_STMT,
  IF_STMT,
  DO_WHILE_STMT,
  WHILE_STMT,
  FOR_STMT,
  BLOCK_STMT,
  SWITCH_STMT,
  BREAK_STMT,
  CONTINUE_STMT,
  EXPRESSION_STMT,
  STRUCT_DECL_STMT,
  UNION_DECL_STMT,
  FUNCTION_DECL_STMT,
  GLOBAL_VARIABLE_DECL_STMT,
  ENUM_DECL_STMT,
  TYPEDEF_DECL_STMT
};

struct Stmt {
  virtual void show_statement(int indent = 0) const = 0;
  virtual StmtType stmt_type() const = 0;

  inline void print_indent(int indent) const {
    for (int i = 0; i < indent; i++) {
      std::cout << "  ";
    }
  }

  virtual ~Stmt() = default;
};

struct EmptyStmt : Stmt {
  virtual StmtType stmt_type() const override {
    return StmtType::EMPTY_STMT;
  }

  void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "EmptyStmt\n";
  }
};

struct BlockStmt : Stmt {
  std::vector<std::unique_ptr<Stmt>> statements;

  BlockStmt() = default;

  explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts) : statements(std::move(stmts)) {
  }

  virtual StmtType stmt_type() const override {
    return StmtType::BLOCK_STMT;
  }

  size_t size() const {
    return statements.size();
  }

  bool empty() const {
    return statements.empty();
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "BlockStmt\n";

    for (const auto &stmt : statements)
      stmt->show_statement(indent + 1);
  }
};

struct VariableDeclarationStmt : Stmt {
  ParsedType type;
  std::string var_name;
  std::unique_ptr<Expr> expr_ptr;
  std::optional<ArrayInitializer> init;

  Type *resolved_type = nullptr;

  VariableDeclarationStmt(ParsedType type_, std::string var_name_, std::unique_ptr<Expr> expr_ptr_, std::optional<ArrayInitializer> init_ = std::nullopt) : type(std::move(type_)), var_name(std::move(var_name_)), expr_ptr(std::move(expr_ptr_)), init(std::move(init_)) {
  }

  virtual StmtType stmt_type() const override {
    return StmtType::VARIABLE_DECL_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);

    std::cout << "VariableDecl(";

    if (resolved_type) {
      Type *base = resolved_type;
      while (base->kind == TypeKind::ARRAY) {
        base = static_cast<ArrayType *>(base)->element_type;
      }
      std::cout << base->to_string();
    } else {
      std::cout << type.to_string();
    }
    std::cout << " " << var_name;

    if (resolved_type) {
      Type *t = resolved_type;
      while (t->kind == TypeKind::ARRAY) {
        auto *arr = static_cast<ArrayType *>(t);
        if (arr->variable_length) {
          std::cout << "[*]";
        } else if (arr->flexible) {
          std::cout << "[]";
        } else {
          std::cout << "[" << arr->size << "]";
        }
        t = arr->element_type;
      }

    } else {
      for (size_t dim : type.dimensions) {
        std::cout << "[" << dim << "]";
      }
    }

    std::cout << ")\n";

    if (expr_ptr) {
      print_indent(indent + 1);
      std::cout << "Initializer\n";
      expr_ptr->show_expr(indent + 2);
    }

    if (init.has_value()) {
      print_indent(indent + 1);
      std::cout << "ArrayInitializer\n";
      init->show_initializer(indent + 2);
    }
  }
};

struct ReturnStmt : Stmt {
  std::unique_ptr<Expr> expr_ptr;

  ReturnStmt(std::unique_ptr<Expr> expr_ptr) : expr_ptr(std::move(expr_ptr)) {
  }

  virtual StmtType stmt_type() const override {
    return StmtType::RETURN_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "ReturnStmt\n";

    if (expr_ptr) {
      expr_ptr->show_expr(indent + 1);
    } else {
      print_indent(indent + 1);
      std::cout << "<void return>\n";
    }
  }
};

struct ExpressionStmt : Stmt {
  std::unique_ptr<Expr> expr;

  ExpressionStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {
  }

  virtual StmtType stmt_type() const override {
    return StmtType::EXPRESSION_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "ExpressionStmt\n";

    if (expr) {
      expr->show_expr(indent + 1);
    }
  }
};

struct IfStmt : Stmt {
  std::unique_ptr<Expr> condition;
  std::unique_ptr<BlockStmt> then_body;
  std::unique_ptr<BlockStmt> else_body;

  IfStmt(std::unique_ptr<Expr> condn, std::unique_ptr<BlockStmt> then_bod, std::unique_ptr<BlockStmt> else_bod) : condition{std::move(condn)}, then_body(std::move(then_bod)), else_body(std::move(else_bod)) {
  }

  virtual StmtType stmt_type() const override {
    return StmtType::IF_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "IfStmt\n";

    print_indent(indent + 1);
    std::cout << "Condition\n";
    condition->show_expr(indent + 2);

    print_indent(indent + 1);
    std::cout << "Then\n";

    if (then_body) {
      then_body->show_statement(indent + 2);
    }

    if (else_body) {
      print_indent(indent + 1);
      std::cout << "Else\n";
      else_body->show_statement(indent + 2);
    }
  }
};

struct DoWhileStmt : Stmt {
  std::unique_ptr<BlockStmt> body;
  std::unique_ptr<Expr> condition;

  DoWhileStmt(std::unique_ptr<BlockStmt> body_, std::unique_ptr<Expr> condition_)
      : body(std::move(body_)), condition(std::move(condition_)) {
  }

  virtual StmtType stmt_type() const override {
    return StmtType::DO_WHILE_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "DoWhileStmt\n";

    print_indent(indent + 1);
    std::cout << "Body\n";

    if (body)
      body->show_statement(indent + 2);

    print_indent(indent + 1);
    std::cout << "Condition\n";

    if (condition) {
      condition->show_expr(indent + 2);
    }
  }
};

struct WhileStmt : Stmt {
  std::unique_ptr<Expr> condition;
  std::unique_ptr<BlockStmt> body;

  WhileStmt(std::unique_ptr<Expr> condn, std::unique_ptr<BlockStmt> bod) : condition{std::move(condn)}, body(std::move(bod)) {
  }

  virtual StmtType stmt_type() const override {
    return StmtType::WHILE_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "WhileStmt\n";

    print_indent(indent + 1);
    std::cout << "Condition\n";

    if (condition)
      condition->show_expr(indent + 2);

    print_indent(indent + 1);
    std::cout << "Body\n";

    if (body)
      body->show_statement(indent + 2);
  }
};

struct ForStmt : Stmt {
  std::unique_ptr<Stmt> init_stmt;
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> update_stmt;

  std::unique_ptr<BlockStmt> body;

  ForStmt(std::unique_ptr<Stmt> init, std::unique_ptr<Expr> condn, std::unique_ptr<Stmt> update, std::unique_ptr<BlockStmt> bod) : init_stmt(std::move(init)), condition{std::move(condn)}, update_stmt(std::move(update)), body(std::move(bod)) {
  }

  virtual StmtType stmt_type() const override {
    return StmtType::FOR_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "ForStmt\n";

    print_indent(indent + 1);
    std::cout << "Init\n";

    if (init_stmt) {
      init_stmt->show_statement(indent + 2);
    }

    print_indent(indent + 1);
    std::cout << "Condition\n";

    if (condition) {
      condition->show_expr(indent + 2);
    }

    print_indent(indent + 1);
    std::cout << "Update\n";

    if (update_stmt) {
      update_stmt->show_statement(indent + 2);
    }

    print_indent(indent + 1);
    std::cout << "Body\n";

    if (body)
      body->show_statement(indent + 2);
  }
};

struct BreakStmt : Stmt {
  virtual StmtType stmt_type() const override {
    return StmtType::BREAK_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "BreakStmt\n";
  }
};

struct ContinueStmt : Stmt {
  virtual StmtType stmt_type() const override {
    return StmtType::CONTINUE_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "ContinueStmt\n";
  }
};

struct SwitchCase {
  std::unique_ptr<Expr> value;
  std::unique_ptr<BlockStmt> body;

  SwitchCase(std::unique_ptr<Expr> value_, std::unique_ptr<BlockStmt> body_)
      : value(std::move(value_)), body(std::move(body_)) {
  }
};

struct SwitchStmt : Stmt {
  std::unique_ptr<Expr> condition;
  std::vector<SwitchCase> cases;
  std::unique_ptr<BlockStmt> default_body;

  SwitchStmt(std::unique_ptr<Expr> condition_, std::vector<SwitchCase> cases_, std::unique_ptr<BlockStmt> default_body_)
      : condition(std::move(condition_)), cases(std::move(cases_)), default_body(std::move(default_body_)) {
  }

  virtual StmtType stmt_type() const override {
    return StmtType::SWITCH_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "SwitchStmt\n";
    print_indent(indent + 1);
    std::cout << "Condition\n";
    if (condition) {
      condition->show_expr(indent + 2);
    }
    for (const auto &case_block : cases) {
      print_indent(indent + 1);
      std::cout << "Case\n";

      if (case_block.value) {
        case_block.value->show_expr(indent + 2);
      }

      if (case_block.body)
        case_block.body->show_statement(indent + 2);
    }
    if (default_body) {
      print_indent(indent + 1);
      std::cout << "Default\n";
      default_body->show_statement(indent + 2);
    }
  }
};

struct StructDeclarationStmt : Stmt {
  std::unique_ptr<StructDecl> declaration;

  explicit StructDeclarationStmt(std::unique_ptr<StructDecl> decl) : declaration(std::move(decl)) {
  }

  virtual StmtType stmt_type() const override {
    return StmtType::STRUCT_DECL_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "StructDeclarationStmt\n";
    if (declaration)
      declaration->show_struct();
  }
};

struct UnionDeclarationStmt : Stmt {
  std::unique_ptr<UnionDecl> declaration;

  explicit UnionDeclarationStmt(std::unique_ptr<UnionDecl> decl) : declaration(std::move(decl)) {
  }

  virtual StmtType stmt_type() const override {
    return StmtType::UNION_DECL_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "UnionDeclarationStmt\n";

    if (declaration)
      declaration->show_union();
  }
};

struct EnumDeclarationStmt : Stmt {
  std::unique_ptr<EnumDecl> declaration;

  explicit EnumDeclarationStmt(std::unique_ptr<EnumDecl> decl) : declaration(std::move(decl)) {
  }

  virtual StmtType stmt_type() const override {
    return StmtType::ENUM_DECL_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "EnumDeclarationStmt\n";

    if (declaration)
      declaration->show_enum(indent + 1);
  }
};

struct TypedefDeclarationStmt : Stmt {
  ParsedType aliased_type;
  std::string alias_name;
  Type *resolved_type = nullptr;

  std::unique_ptr<StructDecl> anonymous_struct;
  std::unique_ptr<UnionDecl> anonymous_union;
  std::unique_ptr<EnumDecl> anonymous_enum;

  TypedefDeclarationStmt(ParsedType type_, std::string alias_) : aliased_type(std::move(type_)), alias_name(std::move(alias_)) {
  }

  virtual StmtType stmt_type() const override {
    return StmtType::TYPEDEF_DECL_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);

    std::cout << "TypedefDecl(";
    if (resolved_type) {
      std::cout << resolved_type->to_string();
    } else {
      std::cout << aliased_type.to_string();
    }

    std::cout << " -> " << alias_name;

    for (size_t dim : aliased_type.dimensions) {
      std::cout << "[" << dim << "]";
    }

    std::cout << ")\n";
  }
};