#include <catch2/catch_test_macros.hpp>

#include "common/test_utils.h"

TEST_CASE("Parser parses empty function definitions") {
  auto result = parse("int main() {}");

  REQUIRE(result.program.statements.size() == 1);
  REQUIRE(result.program.statements[0]->stmt_type() == StmtType::FUNCTION_DECL_STMT);

  auto *fn_decl_stmt = dynamic_cast<FunctionDeclStmt *>(result.program.statements[0].get());

  REQUIRE(fn_decl_stmt != nullptr);

  REQUIRE(fn_decl_stmt->declaration->name == "main");
  REQUIRE(fn_decl_stmt->declaration->parameters.empty());

  REQUIRE(fn_decl_stmt->declaration->body != nullptr);
  REQUIRE(fn_decl_stmt->declaration->body->statements.empty());

  REQUIRE(fn_decl_stmt->declaration->return_type.datatype == DataType::INT);
  REQUIRE_FALSE(fn_decl_stmt->declaration->is_prototype);
}

TEST_CASE("Parser parses function prototype with void parameter") {
  auto result = parse("int main(void);");

  REQUIRE(result.program.statements.size() == 1);
  REQUIRE(result.program.statements[0]->stmt_type() == StmtType::FUNCTION_DECL_STMT);

  auto *fn_decl_stmt = dynamic_cast<FunctionDeclStmt *>(result.program.statements[0].get());

  REQUIRE(fn_decl_stmt != nullptr);

  REQUIRE(fn_decl_stmt->declaration->name == "main");
  REQUIRE(fn_decl_stmt->declaration->parameters.empty());

  REQUIRE(fn_decl_stmt->declaration->body == nullptr);

  REQUIRE(fn_decl_stmt->declaration->return_type.datatype == DataType::INT);
  REQUIRE(fn_decl_stmt->declaration->is_prototype);
}

TEST_CASE("Parser parses function prototype with mixed parameters") {
  auto result = parse("int main(int, char, int, char, double x, float y);");

  REQUIRE(result.program.statements.size() == 1);
  REQUIRE(result.program.statements[0]->stmt_type() == StmtType::FUNCTION_DECL_STMT);

  auto *fn_decl_stmt = dynamic_cast<FunctionDeclStmt *>(result.program.statements[0].get());

  REQUIRE(fn_decl_stmt != nullptr);

  REQUIRE(fn_decl_stmt->declaration->name == "main");
  REQUIRE(fn_decl_stmt->declaration->parameters.size() == 6);

  REQUIRE(fn_decl_stmt->declaration->parameters[0].type.datatype == DataType::INT);
  REQUIRE(fn_decl_stmt->declaration->parameters[1].type.datatype == DataType::CHAR);
  REQUIRE(fn_decl_stmt->declaration->parameters[2].type.datatype == DataType::INT);
  REQUIRE(fn_decl_stmt->declaration->parameters[3].type.datatype == DataType::CHAR);
  REQUIRE(fn_decl_stmt->declaration->parameters[4].type.datatype == DataType::DOUBLE);
  REQUIRE(fn_decl_stmt->declaration->parameters[4].name == "x");

  REQUIRE(fn_decl_stmt->declaration->parameters[5].type.datatype == DataType::FLOAT);
  REQUIRE(fn_decl_stmt->declaration->parameters[5].name == "y");

  REQUIRE(fn_decl_stmt->declaration->body == nullptr);

  REQUIRE(fn_decl_stmt->declaration->return_type.datatype == DataType::INT);
  REQUIRE(fn_decl_stmt->declaration->is_prototype);
}

TEST_CASE("Parser parses function prototype with array parameters of mixed types") {
  auto result = parse("int main(int[3][4], double [9][10], char [10][100][200]);");

  REQUIRE(result.program.statements.size() == 1);
  REQUIRE(result.program.statements[0]->stmt_type() == StmtType::FUNCTION_DECL_STMT);

  auto *fn_decl_stmt = dynamic_cast<FunctionDeclStmt *>(result.program.statements[0].get());

  REQUIRE(fn_decl_stmt != nullptr);

  REQUIRE(fn_decl_stmt->declaration->name == "main");
  REQUIRE(fn_decl_stmt->declaration->parameters.size() == 3);

  REQUIRE(fn_decl_stmt->declaration->parameters[0].type.datatype == DataType::INT);
  REQUIRE(fn_decl_stmt->declaration->parameters[0].type.dimensions == std::vector<size_t>{3, 4});

  REQUIRE(fn_decl_stmt->declaration->parameters[1].type.datatype == DataType::DOUBLE);
  REQUIRE(fn_decl_stmt->declaration->parameters[1].type.dimensions == std::vector<size_t>{9, 10});

  REQUIRE(fn_decl_stmt->declaration->parameters[2].type.datatype == DataType::CHAR);
  REQUIRE(fn_decl_stmt->declaration->parameters[2].type.dimensions == std::vector<size_t>{10, 100, 200});

  REQUIRE(fn_decl_stmt->declaration->body == nullptr);

  REQUIRE(fn_decl_stmt->declaration->return_type.datatype == DataType::INT);
  REQUIRE(fn_decl_stmt->declaration->is_prototype);
}

TEST_CASE("Parser parses function prototype with pointer parameters of mixed types") {
  auto result = parse("int main(int*, int**, double*, double**, float*, float**, char* single_char_ptr, char**, int**** int_4_depth_ptr);");

  REQUIRE(result.program.statements.size() == 1);
  REQUIRE(result.program.statements[0]->stmt_type() == StmtType::FUNCTION_DECL_STMT);

  auto *fn_decl_stmt = dynamic_cast<FunctionDeclStmt *>(result.program.statements[0].get());

  REQUIRE(fn_decl_stmt != nullptr);

  REQUIRE(fn_decl_stmt->declaration->name == "main");
  REQUIRE(fn_decl_stmt->declaration->parameters.size() == 9);

  REQUIRE(fn_decl_stmt->declaration->parameters[0].type.datatype == DataType::INT);
  REQUIRE(fn_decl_stmt->declaration->parameters[0].type.pointer_depth == 1);

  REQUIRE(fn_decl_stmt->declaration->parameters[1].type.datatype == DataType::INT);
  REQUIRE(fn_decl_stmt->declaration->parameters[1].type.pointer_depth == 2);

  REQUIRE(fn_decl_stmt->declaration->parameters[2].type.datatype == DataType::DOUBLE);
  REQUIRE(fn_decl_stmt->declaration->parameters[2].type.pointer_depth == 1);

  REQUIRE(fn_decl_stmt->declaration->parameters[3].type.datatype == DataType::DOUBLE);
  REQUIRE(fn_decl_stmt->declaration->parameters[3].type.pointer_depth == 2);

  REQUIRE(fn_decl_stmt->declaration->parameters[4].type.datatype == DataType::FLOAT);
  REQUIRE(fn_decl_stmt->declaration->parameters[4].type.pointer_depth == 1);

  REQUIRE(fn_decl_stmt->declaration->parameters[5].type.datatype == DataType::FLOAT);
  REQUIRE(fn_decl_stmt->declaration->parameters[5].type.pointer_depth == 2);

  REQUIRE(fn_decl_stmt->declaration->parameters[6].type.datatype == DataType::CHAR);
  REQUIRE(fn_decl_stmt->declaration->parameters[6].type.pointer_depth == 1);
  REQUIRE(fn_decl_stmt->declaration->parameters[6].name == "single_char_ptr");

  REQUIRE(fn_decl_stmt->declaration->parameters[7].type.datatype == DataType::CHAR);
  REQUIRE(fn_decl_stmt->declaration->parameters[7].type.pointer_depth == 2);

  REQUIRE(fn_decl_stmt->declaration->parameters[8].type.datatype == DataType::INT);
  REQUIRE(fn_decl_stmt->declaration->parameters[8].type.pointer_depth == 4);
  REQUIRE(fn_decl_stmt->declaration->parameters[8].name == "int_4_depth_ptr");

  REQUIRE(fn_decl_stmt->declaration->body == nullptr);

  REQUIRE(fn_decl_stmt->declaration->return_type.datatype == DataType::INT);
  REQUIRE(fn_decl_stmt->declaration->is_prototype);
}

TEST_CASE("Parser parses multiple function definitions") {
  auto result = parse("int foo() {} double bar() {}");

  REQUIRE(result.program.statements.size() == 2);
  REQUIRE(result.program.statements[0]->stmt_type() == StmtType::FUNCTION_DECL_STMT);
  REQUIRE(result.program.statements[1]->stmt_type() == StmtType::FUNCTION_DECL_STMT);

  auto *fn_decl_stmt_1 = dynamic_cast<FunctionDeclStmt *>(result.program.statements[0].get());
  auto *fn_decl_stmt_2 = dynamic_cast<FunctionDeclStmt *>(result.program.statements[1].get());

  REQUIRE(fn_decl_stmt_1 != nullptr);
  REQUIRE(fn_decl_stmt_2 != nullptr);

  REQUIRE(fn_decl_stmt_1->declaration->name == "foo");
  REQUIRE(fn_decl_stmt_2->declaration->name == "bar");

  REQUIRE(fn_decl_stmt_1->declaration->parameters.empty());
  REQUIRE(fn_decl_stmt_2->declaration->parameters.empty());

  REQUIRE(fn_decl_stmt_1->declaration->body != nullptr);
  REQUIRE(fn_decl_stmt_2->declaration->body != nullptr);

  REQUIRE(fn_decl_stmt_1->declaration->body->statements.empty());
  REQUIRE(fn_decl_stmt_2->declaration->body->statements.empty());

  REQUIRE(fn_decl_stmt_1->declaration->return_type.datatype == DataType::INT);
  REQUIRE(fn_decl_stmt_2->declaration->return_type.datatype == DataType::DOUBLE);

  REQUIRE_FALSE(fn_decl_stmt_1->declaration->is_prototype);
  REQUIRE_FALSE(fn_decl_stmt_2->declaration->is_prototype);
}

TEST_CASE("Parser parses mixed function prototype and definition") {
  auto result = parse("int foo(); int foo() { return 69;}");

  REQUIRE(result.program.statements.size() == 2);
  REQUIRE(result.program.statements[0]->stmt_type() == StmtType::FUNCTION_DECL_STMT);
  REQUIRE(result.program.statements[1]->stmt_type() == StmtType::FUNCTION_DECL_STMT);

  auto *fn_decl_stmt_1 = dynamic_cast<FunctionDeclStmt *>(result.program.statements[0].get());
  auto *fn_decl_stmt_2 = dynamic_cast<FunctionDeclStmt *>(result.program.statements[1].get());

  REQUIRE(fn_decl_stmt_1 != nullptr);
  REQUIRE(fn_decl_stmt_2 != nullptr);

  REQUIRE(fn_decl_stmt_1->declaration->name == "foo");
  REQUIRE(fn_decl_stmt_2->declaration->name == "foo");

  REQUIRE(fn_decl_stmt_1->declaration->parameters.empty());
  REQUIRE(fn_decl_stmt_2->declaration->parameters.empty());

  REQUIRE(fn_decl_stmt_1->declaration->body == nullptr);
  REQUIRE(fn_decl_stmt_2->declaration->body != nullptr);

  REQUIRE_FALSE(fn_decl_stmt_2->declaration->body->statements.empty());

  REQUIRE(fn_decl_stmt_1->declaration->return_type.datatype == DataType::INT);
  REQUIRE(fn_decl_stmt_2->declaration->return_type.datatype == DataType::INT);

  REQUIRE(fn_decl_stmt_1->declaration->is_prototype);
  REQUIRE_FALSE(fn_decl_stmt_2->declaration->is_prototype);

  REQUIRE(fn_decl_stmt_2->declaration->body->statements.size() == 1);
  REQUIRE(fn_decl_stmt_2->declaration->body->statements[0]->stmt_type() == StmtType::RETURN_STMT);

  auto *return_stmt = dynamic_cast<ReturnStmt *>(fn_decl_stmt_2->declaration->body->statements[0].get());

  REQUIRE(return_stmt != nullptr);
  REQUIRE(return_stmt->expr_ptr != nullptr);

  auto *int_lit_expr = dynamic_cast<IntLetExpr *>(return_stmt->expr_ptr.get());
  REQUIRE(int_lit_expr->value == 69);
}

TEST_CASE("Parser parses multiple function prototypes with different return types") {
  auto result = parse("int main(); double main(); float main(); char main();");

  REQUIRE(result.program.statements.size() == 4);
  REQUIRE(result.program.statements[0]->stmt_type() == StmtType::FUNCTION_DECL_STMT);
  REQUIRE(result.program.statements[1]->stmt_type() == StmtType::FUNCTION_DECL_STMT);
  REQUIRE(result.program.statements[2]->stmt_type() == StmtType::FUNCTION_DECL_STMT);
  REQUIRE(result.program.statements[3]->stmt_type() == StmtType::FUNCTION_DECL_STMT);

  auto *fn_decl_stmt_1 = dynamic_cast<FunctionDeclStmt *>(result.program.statements[0].get());
  auto *fn_decl_stmt_2 = dynamic_cast<FunctionDeclStmt *>(result.program.statements[1].get());
  auto *fn_decl_stmt_3 = dynamic_cast<FunctionDeclStmt *>(result.program.statements[2].get());
  auto *fn_decl_stmt_4 = dynamic_cast<FunctionDeclStmt *>(result.program.statements[3].get());

  REQUIRE(fn_decl_stmt_1 != nullptr);
  REQUIRE(fn_decl_stmt_2 != nullptr);
  REQUIRE(fn_decl_stmt_3 != nullptr);
  REQUIRE(fn_decl_stmt_4 != nullptr);

  REQUIRE(fn_decl_stmt_1->declaration->name == "main");
  REQUIRE(fn_decl_stmt_1->declaration->parameters.empty());

  REQUIRE(fn_decl_stmt_2->declaration->name == "main");
  REQUIRE(fn_decl_stmt_2->declaration->parameters.empty());

  REQUIRE(fn_decl_stmt_3->declaration->name == "main");
  REQUIRE(fn_decl_stmt_3->declaration->parameters.empty());

  REQUIRE(fn_decl_stmt_4->declaration->name == "main");
  REQUIRE(fn_decl_stmt_4->declaration->parameters.empty());

  REQUIRE(fn_decl_stmt_1->declaration->body == nullptr);
  REQUIRE(fn_decl_stmt_2->declaration->body == nullptr);
  REQUIRE(fn_decl_stmt_3->declaration->body == nullptr);
  REQUIRE(fn_decl_stmt_4->declaration->body == nullptr);

  REQUIRE(fn_decl_stmt_1->declaration->return_type.datatype == DataType::INT);
  REQUIRE(fn_decl_stmt_2->declaration->return_type.datatype == DataType::DOUBLE);
  REQUIRE(fn_decl_stmt_3->declaration->return_type.datatype == DataType::FLOAT);
  REQUIRE(fn_decl_stmt_4->declaration->return_type.datatype == DataType::CHAR);

  REQUIRE(fn_decl_stmt_1->declaration->is_prototype);
  REQUIRE(fn_decl_stmt_2->declaration->is_prototype);
  REQUIRE(fn_decl_stmt_3->declaration->is_prototype);
  REQUIRE(fn_decl_stmt_4->declaration->is_prototype);
}
