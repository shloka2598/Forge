#pragma once

#include "./stmts.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct Program {
  std::vector<std::unique_ptr<Stmt>> statements;

  void show() const;
};

struct GlobalVariableDecl {
  ParsedType type;
  std::string name;
  std::unique_ptr<Expr> initializer;
  std::optional<ArrayInitializer> array_initializer;
  Token token;
  Type *resolved_type = nullptr;

  void show_global() const {
    std::cout << "Global Variable\n";

    std::cout << "  ";

    if (resolved_type) {
      Type *base = resolved_type;
      while (base->kind == TypeKind::ARRAY) {
        base = static_cast<ArrayType *>(base)->element_type;
      }
      std::cout << base->to_string();
    } else {
      std::cout << type.to_string();
    }
    std::cout << " " << name;
    for (size_t dim : type.dimensions) {
      std::cout << "[" << dim << "]";
    }
    std::cout << '\n';
    if (initializer) {
      std::cout << "  Initializer\n";
      initializer->show_expr(2);
    }
    if (array_initializer.has_value()) {
      std::cout << "  ArrayInitializer\n";
      array_initializer->show_initializer(2);
    }
  }
};

struct Parameter {
  Token token;
  ParsedType type;
  std::optional<std::string> name;

  Type *resolved_type = nullptr;
};

struct FunctionDecl {
  Token token;
  ParsedType return_type;
  Type *resolved_return_type = nullptr;
  std::string name;
  std::vector<Parameter> parameters;
  std::unique_ptr<BlockStmt> body;

  bool is_prototype = false;

  FunctionDecl(Token token, ParsedType return_type_, std::string name_, std::vector<Parameter> params, std::unique_ptr<BlockStmt> body_) : token{token}, return_type(std::move(return_type_)), name(std::move(name_)), parameters(std::move(params)), body(std::move(body_)), is_prototype(false) {
  }

  FunctionDecl(Token token, ParsedType return_type_, std::string name_, std::vector<Parameter> params) : token{token}, return_type(std::move(return_type_)), name(std::move(name_)), parameters(std::move(params)), is_prototype(true) {
  }

  void show_function() const {
    if (is_prototype) {
      std::cout << "Function Prototype\n";
    } else {
      std::cout << "Function Definition\n";
    }

    std::cout << "function_name -> " << name << std::endl;
    std::cout << "function return type -> ";

    if (resolved_return_type) {
      std::cout << resolved_return_type->to_string();
    } else {
      std::cout << return_type.to_string();
    }

    std::cout << '\n';

    std::cout << "Parameters - " << std::endl;
    for (const Parameter &param : parameters) {

      if (param.resolved_type) {
        std::cout << param.resolved_type->to_string();
      } else {
        std::cout << param.type.to_string();
      }

      if (param.name) {
        std::cout << " " << *param.name;
      }
      for (size_t dim : param.type.dimensions) {
        std::cout << "[" << dim << "]";
      }
      std::cout << '\n';
    }
    if (body) {
      std::cout << "Body -> " << std::endl;
      body->show_statement();
    }
  }
};

struct GlobalVariableDeclStmt : Stmt {
  std::unique_ptr<GlobalVariableDecl> declaration;

  explicit GlobalVariableDeclStmt(std::unique_ptr<GlobalVariableDecl> decl) : declaration(std::move(decl)) {
  }

  virtual StmtType stmt_type() const override {
    return StmtType::GLOBAL_VARIABLE_DECL_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "GlobalVariableDeclStmt\n";

    if (declaration)
      declaration->show_global();
  }
};

struct FunctionDeclStmt : Stmt {
  std::unique_ptr<FunctionDecl> declaration;

  explicit FunctionDeclStmt(Token token, std::unique_ptr<FunctionDecl> decl) : declaration(std::move(decl)) {
    this->token = std::move(token);
  }

  virtual StmtType stmt_type() const override {
    return StmtType::FUNCTION_DECL_STMT;
  }

  virtual void show_statement(int indent = 0) const override {
    print_indent(indent);
    std::cout << "FunctionDeclStmt\n";

    if (declaration)
      declaration->show_function();
  }
};