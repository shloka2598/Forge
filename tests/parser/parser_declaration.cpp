#include <catch2/catch_test_macros.hpp>
// #include <catch2/generators/catch_generators.hpp>

#include "common/test_utils.h"

TEST_CASE("Parser parses struct forward declaration") {
  auto result = parse(R"(
    struct Foo;
  )");

  REQUIRE(result.program.statements.size() == 1);

  auto *stmt = dynamic_cast<StructDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->name == "Foo");
  REQUIRE(stmt->declaration->fields.empty());
}

TEST_CASE("Parser parses empty struct") {
  auto result = parse(R"(
    struct Foo {
    };
  )");

  auto *stmt = dynamic_cast<StructDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->name == "Foo");
  REQUIRE(stmt->declaration->fields.empty());
}

TEST_CASE("Parser parses struct with one field") {
  auto result = parse(R"(
    struct Foo {
        int x;
    };
  )");

  auto *stmt = dynamic_cast<StructDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->fields.size() == 1);

  REQUIRE(stmt->declaration->fields[0].name == "x");
  REQUIRE(stmt->declaration->fields[0].type.datatype == DataType::INT);
}

TEST_CASE("Parser parses struct with multiple fields") {
  auto result = parse(R"(
    struct Foo {
        int a;
        double b;
        char c;
    };
  )");

  auto *stmt = dynamic_cast<StructDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->fields.size() == 3);

  REQUIRE(stmt->declaration->fields[0].type.datatype == DataType::INT);
  REQUIRE(stmt->declaration->fields[1].type.datatype == DataType::DOUBLE);
  REQUIRE(stmt->declaration->fields[2].type.datatype == DataType::CHAR);
}

TEST_CASE("Parser parses struct pointer fields") {
  auto result = parse(R"(
    struct Foo {
        int *a;
        double **b;
        char ***c;
    };
  )");

  auto *stmt = dynamic_cast<StructDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->fields[0].type.pointer_depth == 1);
  REQUIRE(stmt->declaration->fields[1].type.pointer_depth == 2);
  REQUIRE(stmt->declaration->fields[2].type.pointer_depth == 3);
}

TEST_CASE("Parser parses struct array fields") {
  auto result = parse(R"(
    struct Foo {
        int a[10];
        char b[20][30];
    };
  )");

  auto *stmt = dynamic_cast<StructDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->fields[0].type.dimensions == std::vector<size_t>{10});
  REQUIRE(stmt->declaration->fields[1].type.dimensions == std::vector<size_t>{20, 30});
}

TEST_CASE("Parser parses mixed struct fields") {
  auto result = parse(R"(
    struct Foo {
        int a;
        char *b;
        double c[5];
    };
  )");

  auto *stmt = dynamic_cast<StructDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->fields.size() == 3);
  REQUIRE(stmt->declaration->fields[0].type.datatype == DataType::INT);
  REQUIRE(stmt->declaration->fields[1].type.pointer_depth == 1);
  REQUIRE(stmt->declaration->fields[2].type.dimensions == std::vector<size_t>{5});
}

TEST_CASE("Parser parses nested struct declaration") {
  auto result = parse(R"(
    struct Outer {
        struct Inner {
            int x;
        } inner;
    };
  )");

  auto *stmt = dynamic_cast<StructDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->fields.size() == 1);
  REQUIRE(stmt->declaration->fields[0].type.datatype == DataType::STRUCT);
}

TEST_CASE("Parser parses struct field of another struct type") {
  auto result = parse(R"(
    struct Vec2 {
        int x;
        int y;
    };

    struct Player {
        struct Vec2 position;
    };
  )");

  REQUIRE(result.program.statements.size() == 2);

  auto *player = dynamic_cast<StructDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(player != nullptr);
  REQUIRE(player->declaration->fields.size() == 1);
  REQUIRE(player->declaration->fields[0].type.datatype == DataType::STRUCT);
  REQUIRE(player->declaration->fields[0].type.custom_name == "Vec2");
}

TEST_CASE("Parser parses self referential struct pointer") {
  auto result = parse(R"(
    struct Node {
        struct Node *next;
    };
  )");

  auto *stmt = dynamic_cast<StructDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->fields.size() == 1);
  REQUIRE(stmt->declaration->fields[0].type.datatype == DataType::STRUCT);
  REQUIRE(stmt->declaration->fields[0].type.custom_name == "Node");
  REQUIRE(stmt->declaration->fields[0].type.pointer_depth == 1);
}

TEST_CASE("Parser parses union forward declaration") {
  auto result = parse(R"(
    union Value;
  )");

  REQUIRE(result.program.statements.size() == 1);

  auto *stmt = dynamic_cast<UnionDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->name == "Value");
  REQUIRE(stmt->declaration->fields.empty());
}

TEST_CASE("Parser parses empty union") {
  auto result = parse(R"(
    union Value {
    };
  )");

  auto *stmt = dynamic_cast<UnionDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->fields.empty());
}

TEST_CASE("Parser parses union with one field") {
  auto result = parse(R"(
    union Value {
      int x;
    };
  )");

  auto *stmt = dynamic_cast<UnionDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->fields.size() == 1);

  REQUIRE(stmt->declaration->fields[0].name == "x");
  REQUIRE(stmt->declaration->fields[0].type.datatype == DataType::INT);
}

TEST_CASE("Parser parses union with multiple fields") {
  auto result = parse(R"(
    union Value {
      int i;
      double d;
      char c;
    };
  )");

  auto *stmt = dynamic_cast<UnionDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->fields.size() == 3);

  REQUIRE(stmt->declaration->fields[0].type.datatype == DataType::INT);
  REQUIRE(stmt->declaration->fields[1].type.datatype == DataType::DOUBLE);
  REQUIRE(stmt->declaration->fields[2].type.datatype == DataType::CHAR);
}

TEST_CASE("Parser parses pointer members in union") {
  auto result = parse(R"(
    union Value {
      int *a;
      double **b;
      char ***c;
    };
  )");

  auto *stmt = dynamic_cast<UnionDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->fields[0].type.pointer_depth == 1);
  REQUIRE(stmt->declaration->fields[1].type.pointer_depth == 2);
  REQUIRE(stmt->declaration->fields[2].type.pointer_depth == 3);
}

TEST_CASE("Parser parses array members in union") {
  auto result = parse(R"(
    union Value {
      int a[10];
      char b[20][30];
    };
  )");

  auto *stmt = dynamic_cast<UnionDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->fields[0].type.dimensions == std::vector<size_t>{10});
  REQUIRE(stmt->declaration->fields[1].type.dimensions == std::vector<size_t>{20, 30});
}

TEST_CASE("Parser parses mixed union members") {
  auto result = parse(R"(
    union Value {
      int a;
      char *b;
      double c[5];
    };
  )");

  auto *stmt = dynamic_cast<UnionDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->fields.size() == 3);

  REQUIRE(stmt->declaration->fields[0].type.datatype == DataType::INT);
  REQUIRE(stmt->declaration->fields[1].type.pointer_depth == 1);
  REQUIRE(stmt->declaration->fields[2].type.dimensions == std::vector<size_t>{5});
}

TEST_CASE("Parser parses nested union declaration") {
  auto result = parse(R"(
    union Outer {
      union Inner {
        int x;
      } inner;
    };
  )");

  auto *stmt = dynamic_cast<UnionDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->fields.size() == 1);
  REQUIRE(stmt->declaration->fields[0].type.datatype == DataType::UNION);
}

TEST_CASE("Parser parses union member of another union type") {
  auto result = parse(R"(
    union A {
      int x;
    };

    union B {
      union A value;
    };
  )");

  REQUIRE(result.program.statements.size() == 2);

  auto *stmt = dynamic_cast<UnionDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->fields.size() == 1);
  REQUIRE(stmt->declaration->fields[0].type.datatype == DataType::UNION);
  REQUIRE(stmt->declaration->fields[0].type.custom_name == "A");
}

TEST_CASE("Parser parses self referential union pointer") {
  auto result = parse(R"(
    union Node {
      union Node *next;
    };
  )");

  auto *stmt = dynamic_cast<UnionDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->fields.size() == 1);
  REQUIRE(stmt->declaration->fields[0].type.datatype == DataType::UNION);
  REQUIRE(stmt->declaration->fields[0].type.custom_name == "Node");
  REQUIRE(stmt->declaration->fields[0].type.pointer_depth == 1);
}

TEST_CASE("Parser parses enum forward declaration") {
  auto result = parse(R"(
    enum Color;
  )");

  REQUIRE(result.program.statements.size() == 1);

  auto *stmt = dynamic_cast<EnumDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->name == "Color");
  REQUIRE(stmt->declaration->members.empty());
  REQUIRE_FALSE(stmt->declaration->is_definition);
}

TEST_CASE("Parser parses anonymous enum") {
  auto result = parse(R"(
    enum {
      A,
      B
    };
  )");

  auto *stmt = dynamic_cast<EnumDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->name.empty());
  REQUIRE(stmt->declaration->members.size() == 2);
  REQUIRE(stmt->declaration->members[0].name == "A");
  REQUIRE(stmt->declaration->members[1].name == "B");
}

TEST_CASE("Parser parses enum with one member") {
  auto result = parse(R"(
    enum Color {
      RED
    };
  )");

  auto *stmt = dynamic_cast<EnumDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->members.size() == 1);
  REQUIRE(stmt->declaration->members[0].name == "RED");
  REQUIRE(stmt->declaration->members[0].value == nullptr);
}

TEST_CASE("Parser parses enum with multiple members") {
  auto result = parse(R"(
    enum Color {
      RED,
      GREEN,
      BLUE
    };
  )");

  auto *stmt = dynamic_cast<EnumDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->members.size() == 3);
  REQUIRE(stmt->declaration->members[0].name == "RED");
  REQUIRE(stmt->declaration->members[1].name == "GREEN");
  REQUIRE(stmt->declaration->members[2].name == "BLUE");
}

TEST_CASE("Parser parses enum integer initializer") {
  auto result = parse(R"(
    enum Color {
      RED = 5
    };
  )");

  auto *stmt = dynamic_cast<EnumDeclarationStmt *>(result.program.statements[0].get());
  auto &member = stmt->declaration->members[0];

  REQUIRE(member.value != nullptr);
  REQUIRE(member.value->expr_type() == ExprType::INT_LITERAL);

  auto *lit = dynamic_cast<IntLetExpr *>(member.value.get());

  REQUIRE(lit != nullptr);
  REQUIRE(lit->value == 5);
}

TEST_CASE("Parser parses enum identifier initializer") {
  auto result = parse(R"(
    enum Color {
      RED,
      GREEN = RED
    };
  )");

  auto *stmt = dynamic_cast<EnumDeclarationStmt *>(result.program.statements[0].get());

  auto &member = stmt->declaration->members[1];

  REQUIRE(member.value != nullptr);
  REQUIRE(member.value->expr_type() == ExprType::IDENTIFIER);

  auto *id = dynamic_cast<IdentifierExpr *>(member.value.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "RED");
}

TEST_CASE("Parser parses enum binary initializer") {
  auto result = parse(R"(
    enum Foo {
      A = 1 + 2
    };
  )");

  auto *stmt = dynamic_cast<EnumDeclarationStmt *>(result.program.statements[0].get());
  auto &member = stmt->declaration->members[0];

  REQUIRE(member.value->expr_type() == ExprType::BINARY);

  auto *binary = dynamic_cast<BinaryExpr *>(member.value.get());

  REQUIRE(binary != nullptr);
  REQUIRE(binary->op == TokenType::PLUS);
}

TEST_CASE("Parser parses enum unary initializer") {
  auto result = parse(R"(
    enum Foo {
      A = -1
    };
  )");

  auto *stmt = dynamic_cast<EnumDeclarationStmt *>(result.program.statements[0].get());

  auto &member = stmt->declaration->members[0];

  REQUIRE(member.value->expr_type() == ExprType::UNARY);
}

TEST_CASE("Parser parses enum cast initializer") {
  auto result = parse(R"(
    enum Foo {
      A = (int)5
    };
  )");

  auto *stmt = dynamic_cast<EnumDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt->declaration->members[0].value->expr_type() == ExprType::CAST);
}

TEST_CASE("Parser parses enum sizeof initializer") {
  auto result = parse(R"(
    enum Foo {
      A = sizeof(int)
    };
  )");

  auto *stmt = dynamic_cast<EnumDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt->declaration->members[0].value->expr_type() == ExprType::SIZEOF);
}

TEST_CASE("Parser parses enum conditional initializer") {
  auto result = parse(R"(
    enum Foo {
      A = x ? y : z
    };
  )");

  auto *stmt = dynamic_cast<EnumDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt->declaration->members[0].value->expr_type() == ExprType::CONDITIONAL);
}

TEST_CASE("Parser parses enum function call initializer") {
  auto result = parse(R"(
    enum Foo {
      A = foo()
    };
  )");

  auto *stmt = dynamic_cast<EnumDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt->declaration->members[0].value->expr_type() == ExprType::FUNCTION_CALL);
}

TEST_CASE("Parser parses enum trailing comma") {
  auto result = parse(R"(
    enum Foo {
      A,
      B,
      C,
    };
  )");

  auto *stmt = dynamic_cast<EnumDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->members.size() == 3);
}

TEST_CASE("Parser parses enum mixed initializers") {
  auto result = parse(R"(
    enum Foo {
      A,
      B = 5,
      C,
      D = 1 + 2,
      E = sizeof(int),
      F
    };
  )");

  auto *stmt = dynamic_cast<EnumDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->members.size() == 6);
  REQUIRE(stmt->declaration->members[0].value == nullptr);
  REQUIRE(stmt->declaration->members[1].value != nullptr);
  REQUIRE(stmt->declaration->members[2].value == nullptr);
  REQUIRE(stmt->declaration->members[3].value != nullptr);
  REQUIRE(stmt->declaration->members[4].value != nullptr);
  REQUIRE(stmt->declaration->members[5].value == nullptr);
}