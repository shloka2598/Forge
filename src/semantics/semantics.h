#pragma once

#include "../ast/program.h"
#include "../ast/stmts.h"
#include "../parser/parser.h"
#include "../types/TypeContext.h"
#include "./scope.h"

class Semantics {
private:
  const Program &program;
  DiagnosticEngine &diagnostics;

  Scope global_scope;
  TypeContext type_context;

  bool has_error = false;

  Type *current_function_return_type = nullptr;
  int loop_depth = 0;
  int switch_depth = 0;

  void analyze_function(FunctionDecl *, Scope &);
  bool analyze_stmt(Stmt *, Scope &);
  Type *analyze_expr(Expr *, Scope &);
  Type *analyze_binary_expr(const BinaryExpr &, Scope &);
  Type *analyze_unary_expr(UnaryExpr &, Scope &);
  void analyze_array_initializer(const ArrayInitializer &init, Type *expected_type, Scope &scope);

  // Type stuff

  Type *convert_datatype(DataType);
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
  bool contains_void_object(Type *);
  bool can_implicitly_convert(Type *from, Type *to);
  bool can_explicitly_cast(Type *from, Type *to);
  bool can_assign(Type *lhs, Type *rhs);
  bool compatible_pointer_types(Type *, Type *);
  bool is_null_pointer_constant(const Expr *);
  bool is_comparable_pointer_types(Type *, Type *);

  Type *decay(Type *);
  Type *integer_promotion(Type *);
  Type *default_argument_promotion(Type *);
  Type *usual_arithmetic_conversion(Type *, Type *);
  Type *common_real_type(Type *, Type *);
  Type *build_type(const ParsedType &, Scope &);
  Type *lookup_tag_type(Scope &scope, const Token &, const std::string &name, SymbolKind expected_kind);

  Type *composite_pointer_type(Type *, Type *);

  size_t sizeof_type(Type *);

  std::optional<int64_t> evaluate_constant_expr(const Expr *expr);

  bool register_symbol(Scope &scope, SymbolKind kind, const std::string &name, Type *type);
  void error(const Token &, const std::string &);
  void warning(const Token &, const std::string &);

public:
  explicit Semantics(const Program &_program, DiagnosticEngine &engine) : program{_program}, diagnostics(engine) {
  }

  void analyze();
};