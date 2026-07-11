#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

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

TEST_CASE("Parser parses typedef of int") {
  auto result = parse(R"(
    typedef int INT;
  )");

  REQUIRE(result.program.statements.size() == 1);

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::INT);
  REQUIRE(stmt->alias_name == "INT");
}

TEST_CASE("Parser parses typedef of double") {
  auto result = parse(R"(
    typedef double DOUBLE;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::DOUBLE);
  REQUIRE(stmt->alias_name == "DOUBLE");
}

TEST_CASE("Parser parses typedef of void") {
  auto result = parse(R"(
    typedef void VOID;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::VOID);
  REQUIRE(stmt->alias_name == "VOID");
}

TEST_CASE("Parser parses typedef of char") {
  auto result = parse(R"(
    typedef char CHAR;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::CHAR);
  REQUIRE(stmt->alias_name == "CHAR");
}

TEST_CASE("Parser parses typedef of single pointer") {
  auto result = parse(R"(
    typedef int *INT_PTR;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::INT);
  REQUIRE(stmt->aliased_type.pointer_depth == 1);

  REQUIRE(stmt->alias_name == "INT_PTR");
}

TEST_CASE("Parser parses typedef of double pointer") {
  auto result = parse(R"(
    typedef int **INT_PTR2;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::INT);
  REQUIRE(stmt->aliased_type.pointer_depth == 2);

  REQUIRE(stmt->alias_name == "INT_PTR2");
}

TEST_CASE("Parser parses typedef of triple pointer") {
  auto result = parse(R"(
    typedef int ***INT_PTR3;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::INT);
  REQUIRE(stmt->aliased_type.pointer_depth == 3);

  REQUIRE(stmt->alias_name == "INT_PTR3");
}

TEST_CASE("Parser parses typedef of one dimensional array") {
  auto result = parse(R"(
    typedef int INT_ARRAY[10];
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::INT);

  REQUIRE(stmt->aliased_type.dimensions == std::vector<size_t>{10});

  REQUIRE(stmt->alias_name == "INT_ARRAY");
}

TEST_CASE("Parser parses typedef of multidimensional array") {
  auto result = parse(R"(
    typedef int MATRIX[5][10][20];
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::INT);

  REQUIRE(stmt->aliased_type.dimensions == std::vector<size_t>{5, 10, 20});

  REQUIRE(stmt->alias_name == "MATRIX");
}

TEST_CASE("Parser parses typedef of pointer array") {
  auto result = parse(R"(
    typedef int *PTR_ARRAY[10];
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::INT);

  REQUIRE(stmt->aliased_type.pointer_depth == 1);

  REQUIRE(stmt->aliased_type.dimensions == std::vector<size_t>{10});

  REQUIRE(stmt->alias_name == "PTR_ARRAY");
}

TEST_CASE("Parser parses typedef of named struct") {
  auto result = parse(R"(
    struct Foo {
      int x;
    };

    typedef struct Foo Foo;
  )");

  REQUIRE(result.program.statements.size() == 2);

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::STRUCT);
  REQUIRE(stmt->aliased_type.custom_name == "Foo");
  REQUIRE(stmt->alias_name == "Foo");

  REQUIRE(stmt->anonymous_struct == nullptr);
}

TEST_CASE("Parser parses typedef of forward declared struct") {
  auto result = parse(R"(
    struct Foo;

    typedef struct Foo Foo;
  )");

  REQUIRE(result.program.statements.size() == 2);

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::STRUCT);
  REQUIRE(stmt->aliased_type.custom_name == "Foo");

  REQUIRE(stmt->anonymous_struct == nullptr);
}

TEST_CASE("Parser parses typedef of struct pointer") {
  auto result = parse(R"(
    struct Foo;

    typedef struct Foo *FooPtr;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::STRUCT);
  REQUIRE(stmt->aliased_type.custom_name == "Foo");
  REQUIRE(stmt->aliased_type.pointer_depth == 1);

  REQUIRE(stmt->alias_name == "FooPtr");
}

TEST_CASE("Parser parses typedef of anonymous struct") {
  auto result = parse(R"(
    typedef struct {
      int x;
    } Point;
  )");

  REQUIRE(result.program.statements.size() == 1);

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_struct != nullptr);

  REQUIRE(stmt->anonymous_struct->name.empty());

  REQUIRE(stmt->anonymous_struct->fields.size() == 1);

  REQUIRE(stmt->alias_name == "Point");
}

TEST_CASE("Parser parses typedef of anonymous struct with multiple fields") {
  auto result = parse(R"(
    typedef struct {
      int x;
      int y;
      char c;
    } Point;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_struct != nullptr);

  REQUIRE(stmt->anonymous_struct->fields.size() == 3);

  REQUIRE(stmt->anonymous_struct->fields[0].type.datatype == DataType::INT);
  REQUIRE(stmt->anonymous_struct->fields[1].type.datatype == DataType::INT);
  REQUIRE(stmt->anonymous_struct->fields[2].type.datatype == DataType::CHAR);
}

TEST_CASE("Parser parses typedef of anonymous struct with pointer members") {
  auto result = parse(R"(
    typedef struct {
      int *a;
      double **b;
    } Node;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_struct != nullptr);

  REQUIRE(stmt->anonymous_struct->fields[0].type.pointer_depth == 1);
  REQUIRE(stmt->anonymous_struct->fields[1].type.pointer_depth == 2);
}

TEST_CASE("Parser parses typedef of anonymous struct with array members") {
  auto result = parse(R"(
    typedef struct {
      int data[10];
      char buffer[4][8];
    } Buffer;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_struct != nullptr);

  REQUIRE(stmt->anonymous_struct->fields[0].type.dimensions == std::vector<size_t>{10});

  REQUIRE(stmt->anonymous_struct->fields[1].type.dimensions == std::vector<size_t>{4, 8});
}

TEST_CASE("Parser parses typedef of self referential anonymous struct") {
  auto result = parse(R"(
    typedef struct Node {
      struct Node *next;
    } Node;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::STRUCT);

  REQUIRE(stmt->anonymous_struct != nullptr);

  REQUIRE(stmt->anonymous_struct->fields.size() == 1);

  REQUIRE(stmt->anonymous_struct->fields[0].type.datatype == DataType::STRUCT);

  REQUIRE(stmt->anonymous_struct->fields[0].type.custom_name == "Node");

  REQUIRE(stmt->anonymous_struct->fields[0].type.pointer_depth == 1);
}

TEST_CASE("Parser parses typedef of anonymous struct containing another struct") {
  auto result = parse(R"(
    typedef struct {
      struct {
        int x;
      } inner;
    } Outer;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_struct != nullptr);

  REQUIRE(stmt->anonymous_struct->fields.size() == 1);

  REQUIRE(stmt->anonymous_struct->fields[0].type.datatype == DataType::STRUCT);
}

TEST_CASE("Parser parses typedef of anonymous struct containing enum and union") {
  auto result = parse(R"(
    typedef struct {
      enum {
        RED,
        GREEN
      } color;

      union {
        int i;
        float f;
      } value;
    } Variant;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_struct != nullptr);

  REQUIRE(stmt->anonymous_struct->fields.size() == 2);

  REQUIRE(stmt->anonymous_struct->fields[0].type.datatype == DataType::ENUM);

  REQUIRE(stmt->anonymous_struct->fields[1].type.datatype == DataType::UNION);
}

TEST_CASE("Parser parses typedef of named union") {
  auto result = parse(R"(
    union Value {
      int x;
    };

    typedef union Value Value;
  )");

  REQUIRE(result.program.statements.size() == 2);

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::UNION);
  REQUIRE(stmt->aliased_type.custom_name == "Value");
  REQUIRE(stmt->alias_name == "Value");

  REQUIRE(stmt->anonymous_union == nullptr);
}

TEST_CASE("Parser parses typedef of forward declared union") {
  auto result = parse(R"(
    union Value;

    typedef union Value Value;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::UNION);
  REQUIRE(stmt->aliased_type.custom_name == "Value");

  REQUIRE(stmt->anonymous_union == nullptr);
}

TEST_CASE("Parser parses typedef of union pointer") {
  auto result = parse(R"(
    union Value;

    typedef union Value *ValuePtr;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::UNION);
  REQUIRE(stmt->aliased_type.custom_name == "Value");
  REQUIRE(stmt->aliased_type.pointer_depth == 1);

  REQUIRE(stmt->alias_name == "ValuePtr");
}

TEST_CASE("Parser parses typedef of anonymous union") {
  auto result = parse(R"(
    typedef union {
      int i;
      float f;
    } Value;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_union != nullptr);

  REQUIRE(stmt->anonymous_union->name.empty());

  REQUIRE(stmt->anonymous_union->fields.size() == 2);

  REQUIRE(stmt->alias_name == "Value");
}

TEST_CASE("Parser parses typedef of anonymous union with pointer members") {
  auto result = parse(R"(
    typedef union {
      int *a;
      double **b;
    } PtrUnion;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_union != nullptr);

  REQUIRE(stmt->anonymous_union->fields.size() == 2);

  REQUIRE(stmt->anonymous_union->fields[0].type.pointer_depth == 1);

  REQUIRE(stmt->anonymous_union->fields[1].type.pointer_depth == 2);
}

TEST_CASE("Parser parses typedef of anonymous union with array members") {
  auto result = parse(R"(
    typedef union {
      int data[10];
      char buffer[4][8];
    } Buffer;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_union != nullptr);
  REQUIRE(stmt->anonymous_union->fields[0].type.dimensions == std::vector<size_t>{10});
  REQUIRE(stmt->anonymous_union->fields[1].type.dimensions == std::vector<size_t>{4, 8});
}

TEST_CASE("Parser parses typedef of anonymous union containing anonymous struct") {
  auto result = parse(R"(
    typedef union {
      struct {
        int x;
      } s;
    } Value;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_union != nullptr);

  REQUIRE(stmt->anonymous_union->fields.size() == 1);

  REQUIRE(stmt->anonymous_union->fields[0].type.datatype == DataType::STRUCT);
}

TEST_CASE("Parser parses typedef of anonymous union containing anonymous enum") {
  auto result = parse(R"(
    typedef union {
      enum {
        RED,
        GREEN
      } color;
    } Value;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_union != nullptr);

  REQUIRE(stmt->anonymous_union->fields.size() == 1);

  REQUIRE(stmt->anonymous_union->fields[0].type.datatype == DataType::ENUM);
}

TEST_CASE("Parser parses typedef of anonymous union containing anonymous union") {
  auto result = parse(R"(
    typedef union {
      union {
        int x;
        float y;
      } inner;
    } Value;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_union != nullptr);

  REQUIRE(stmt->anonymous_union->fields.size() == 1);

  REQUIRE(stmt->anonymous_union->fields[0].type.datatype == DataType::UNION);
}

TEST_CASE("Parser parses typedef of union with mixed member types") {
  auto result = parse(R"(
    typedef union {
      int i;
      struct {
        int x;
      } s;
      enum {
        RED
      } e;
      char *ptr;
      double values[5];
    } Value;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_union != nullptr);
  REQUIRE(stmt->anonymous_union->fields.size() == 5);
  REQUIRE(stmt->anonymous_union->fields[0].type.datatype == DataType::INT);
  REQUIRE(stmt->anonymous_union->fields[1].type.datatype == DataType::STRUCT);
  REQUIRE(stmt->anonymous_union->fields[2].type.datatype == DataType::ENUM);
  REQUIRE(stmt->anonymous_union->fields[3].type.pointer_depth == 1);
  REQUIRE(stmt->anonymous_union->fields[4].type.dimensions == std::vector<size_t>{5});
}

TEST_CASE("Parser parses typedef of named enum") {
  auto result = parse(R"(
    enum Color {
      RED,
      GREEN
    };

    typedef enum Color Color;
  )");

  REQUIRE(result.program.statements.size() == 2);

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::ENUM);
  REQUIRE(stmt->aliased_type.custom_name == "Color");
  REQUIRE(stmt->alias_name == "Color");

  REQUIRE(stmt->anonymous_enum == nullptr);
}

TEST_CASE("Parser parses typedef of forward declared enum") {
  auto result = parse(R"(
    enum Color;

    typedef enum Color Color;
  )");

  REQUIRE(result.program.statements.size() == 2);

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::ENUM);
  REQUIRE(stmt->aliased_type.custom_name == "Color");

  REQUIRE(stmt->anonymous_enum == nullptr);
}

TEST_CASE("Parser parses typedef of anonymous enum") {
  auto result = parse(R"(
    typedef enum {
      RED,
      GREEN,
      BLUE
    } Color;
  )");

  REQUIRE(result.program.statements.size() == 1);

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_enum != nullptr);

  REQUIRE(stmt->anonymous_enum->name.empty());

  REQUIRE(stmt->anonymous_enum->members.size() == 3);

  REQUIRE(stmt->alias_name == "Color");
}

TEST_CASE("Parser parses typedef of anonymous enum with explicit values") {
  auto result = parse(R"(
    typedef enum {
      RED = 1,
      GREEN = 2,
      BLUE = 3
    } Color;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_enum != nullptr);

  REQUIRE(stmt->anonymous_enum->members.size() == 3);

  for (const auto &member : stmt->anonymous_enum->members) {
    REQUIRE(member.value != nullptr);
    REQUIRE(member.value->expr_type() == ExprType::INT_LITERAL);
  }
}

TEST_CASE("Parser parses typedef of anonymous enum with binary expressions") {
  auto result = parse(R"(
    typedef enum {
      A = 1 + 2,
      B = 3 * 4
    } Numbers;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_enum != nullptr);

  REQUIRE(stmt->anonymous_enum->members.size() == 2);

  REQUIRE(stmt->anonymous_enum->members[0].value->expr_type() == ExprType::BINARY);

  REQUIRE(stmt->anonymous_enum->members[1].value->expr_type() == ExprType::BINARY);
}

TEST_CASE("Parser parses typedef of anonymous enum with sizeof") {
  auto result = parse(R"(
    typedef enum {
      A = sizeof(int),
      B = sizeof(double)
    } Sizes;
  )");

  auto *stmt =
      dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_enum != nullptr);

  REQUIRE(stmt->anonymous_enum->members.size() == 2);

  REQUIRE(stmt->anonymous_enum->members[0].value->expr_type() == ExprType::SIZEOF);

  REQUIRE(stmt->anonymous_enum->members[1].value->expr_type() == ExprType::SIZEOF);
}

TEST_CASE("Parser parses typedef of anonymous enum with cast expressions") {
  auto result = parse(R"(
    typedef enum {
      A = (int)1.5,
      B = (int)2
    } Numbers;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_enum != nullptr);

  REQUIRE(stmt->anonymous_enum->members[0].value->expr_type() == ExprType::CAST);

  REQUIRE(stmt->anonymous_enum->members[1].value->expr_type() == ExprType::CAST);
}

TEST_CASE("Parser parses typedef of anonymous enum with conditional expressions") {
  auto result = parse(R"(
    typedef enum {
      A = x ? 1 : 2
    } Numbers;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_enum != nullptr);

  REQUIRE(stmt->anonymous_enum->members.size() == 1);

  REQUIRE(stmt->anonymous_enum->members[0].value->expr_type() == ExprType::CONDITIONAL);
}

TEST_CASE("Parser parses typedef of anonymous enum with identifier expressions") {
  auto result = parse(R"(
    typedef enum {
      A = RED
    } Numbers;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->anonymous_enum != nullptr);
  REQUIRE(stmt->anonymous_enum->members[0].value->expr_type() == ExprType::IDENTIFIER);
}

TEST_CASE("Parser parses typedef of anonymous enum with mixed expressions") {
  auto result = parse(R"(
    typedef enum {
      A,
      B = 5,
      C = 1 + 2,
      D = sizeof(int),
      E = (int)4.2,
      F = foo(),
      G = x ? 1 : 2
    } Numbers;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->anonymous_enum != nullptr);
  REQUIRE(stmt->anonymous_enum->members.size() == 7);
  REQUIRE(stmt->anonymous_enum->members[0].value == nullptr);
  REQUIRE(stmt->anonymous_enum->members[1].value->expr_type() == ExprType::INT_LITERAL);
  REQUIRE(stmt->anonymous_enum->members[2].value->expr_type() == ExprType::BINARY);
  REQUIRE(stmt->anonymous_enum->members[3].value->expr_type() == ExprType::SIZEOF);
  REQUIRE(stmt->anonymous_enum->members[4].value->expr_type() == ExprType::CAST);
  REQUIRE(stmt->anonymous_enum->members[5].value->expr_type() == ExprType::FUNCTION_CALL);
  REQUIRE(stmt->anonymous_enum->members[6].value->expr_type() == ExprType::CONDITIONAL);
}

TEST_CASE("Parser parses typedef from typedef") {
  auto result = parse(R"(
    typedef int INT;
    typedef INT INTEGER;
  )");

  REQUIRE(result.program.statements.size() == 2);

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(stmt->aliased_type.custom_name == "INT");

  REQUIRE(stmt->alias_name == "INTEGER");
}

TEST_CASE("Parser parses pointer typedef from typedef") {
  auto result = parse(R"(
    typedef int INT;
    typedef INT *INT_PTR;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(stmt->aliased_type.custom_name == "INT");

  REQUIRE(stmt->aliased_type.pointer_depth == 1);

  REQUIRE(stmt->alias_name == "INT_PTR");
}

TEST_CASE("Parser parses chained typedefs") {
  auto result = parse(R"(
    typedef int A;
    typedef A B;
    typedef B C;
  )");

  REQUIRE(result.program.statements.size() == 3);

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[2].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(stmt->aliased_type.custom_name == "B");

  REQUIRE(stmt->alias_name == "C");
}

TEST_CASE("Parser parses array typedef from typedef") {
  auto result = parse(R"(
    typedef int INT;
    typedef INT MATRIX[10][20];
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.custom_name == "INT");

  REQUIRE(stmt->aliased_type.dimensions == std::vector<size_t>{10, 20});
}

TEST_CASE("Parser parses pointer array typedef") {
  auto result = parse(R"(
    typedef int INT;
    typedef INT *TABLE[8];
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.pointer_depth == 1);

  REQUIRE(stmt->aliased_type.dimensions == std::vector<size_t>{8});
}

TEST_CASE("Parser parses chained struct typedef") {
  auto result = parse(R"(
    typedef struct {
      int x;
    } Point;

    typedef Point Point2;
  )");

  REQUIRE(result.program.statements.size() == 2);

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.custom_name == "Point");

  REQUIRE(stmt->alias_name == "Point2");
}

TEST_CASE("Parser parses chained union typedef") {
  auto result = parse(R"(
    typedef union {
      int x;
    } Value;

    typedef Value Value2;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.custom_name == "Value");
}

TEST_CASE("Parser parses chained enum typedef") {
  auto result = parse(R"(
    typedef enum {
      RED
    } Color;

    typedef Color Color2;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.custom_name == "Color");
}

TEST_CASE("Parser parses typedef of self referential pointer") {
  auto result = parse(R"(
    typedef struct Node {
      struct Node *next;
    } Node;

    typedef Node *NodePtr;
  )");

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->aliased_type.custom_name == "Node");

  REQUIRE(stmt->aliased_type.pointer_depth == 1);

  REQUIRE(stmt->alias_name == "NodePtr");
}

TEST_CASE("Parser parses many typedef declarations") {
  auto result = parse(R"(
    typedef int INT;
    typedef float FLOAT;
    typedef double DOUBLE;
    typedef char CHAR;

    typedef INT INTEGER;
    typedef INTEGER NUMBER;

    typedef NUMBER *NUMBER_PTR;

    typedef NUMBER_PTR TABLE[10];
  )");

  REQUIRE(result.program.statements.size() == 8);

  auto *stmt = dynamic_cast<TypedefDeclarationStmt *>(result.program.statements.back().get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->alias_name == "TABLE");

  REQUIRE(stmt->aliased_type.pointer_depth == 0);

  REQUIRE(stmt->aliased_type.dimensions == std::vector<size_t>{10});
}

TEST_CASE("Parser parses variable using typedef type") {
  auto result = parse(R"(
        typedef int INT;
        INT value;
    )");

  REQUIRE(result.program.statements.size() == 2);

  auto *var = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[1].get());

  REQUIRE(var != nullptr);

  REQUIRE(var->declaration->type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(var->declaration->type.custom_name == "INT");

  REQUIRE(var->declaration->name == "value");
}

TEST_CASE("Parser parses struct field using typedef type") {
  auto result = parse(R"(
        typedef int INT;

        struct Foo {
            INT x;
        };
    )");

  REQUIRE(result.program.statements.size() == 2);

  auto *stmt = dynamic_cast<StructDeclarationStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->fields[0].type.datatype == DataType::TYPEDEF_NAME);

  REQUIRE(stmt->declaration->fields[0].type.custom_name == "INT");
}

TEST_CASE("Parser parses int variable declaration") {
  auto result = parse(R"(
    int main() {
      int x;
    }
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn->declaration->body->statements.size() == 1);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->type.datatype == DataType::INT);
  REQUIRE(stmt->var_name == "x");

  REQUIRE(stmt->expr_ptr == nullptr);
  REQUIRE_FALSE(stmt->init.has_value());
}

TEST_CASE("Parser parses initialized int variable") {
  auto result = parse(R"(
    int main() {
      int x = 42;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->expr_ptr != nullptr);

  REQUIRE(stmt->expr_ptr->expr_type() == ExprType::INT_LITERAL);

  auto *lit = dynamic_cast<IntLetExpr *>(stmt->expr_ptr.get());

  REQUIRE(lit != nullptr);
  REQUIRE(lit->value == 42);
}

TEST_CASE("Parser parses double variable") {
  auto result = parse(R"(
    int main() {
      double pi;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.datatype == DataType::DOUBLE);
  REQUIRE(stmt->var_name == "pi");
}

TEST_CASE("Parser parses pointer variable") {
  auto result = parse(R"(
    int main() {
      int *ptr;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.datatype == DataType::INT);
  REQUIRE(stmt->type.pointer_depth == 1);

  REQUIRE(stmt->var_name == "ptr");
}

TEST_CASE("Parser parses double pointer variable") {
  auto result = parse(R"(
    int main() {
      int **ptr;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.pointer_depth == 2);
}

TEST_CASE("Parser parses array variable") {
  auto result = parse(R"(
    int main() {
      int arr[10];
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.datatype == DataType::INT);

  REQUIRE(stmt->type.dimensions == std::vector<size_t>{10});
}

TEST_CASE("Parser parses multidimensional array variable") {
  auto result = parse(R"(
    int main() {
      int matrix[3][4][5];
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.dimensions == std::vector<size_t>{3, 4, 5});
}

TEST_CASE("Parser parses variable initialized with expression") {
  auto result = parse(R"(
    int main() {
      int x = a + b * c;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->expr_ptr != nullptr);

  REQUIRE(stmt->expr_ptr->expr_type() == ExprType::BINARY);
}

TEST_CASE("Parser parses variable initialized with cast") {
  auto result = parse(R"(
    int main() {
      int x = (int)foo;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->expr_ptr->expr_type() == ExprType::CAST);
}

TEST_CASE("Parser parses variable initialized with function call") {
  auto result = parse(R"(
    int main() {
      int x = foo(1, 2);
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->expr_ptr->expr_type() == ExprType::FUNCTION_CALL);
}

TEST_CASE("Parser parses variable of typedef type") {
  auto result = parse(R"(
    typedef int INT;

    int main() {
      INT value;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(stmt->type.custom_name == "INT");

  REQUIRE(stmt->var_name == "value");
}

TEST_CASE("Parser parses initialized typedef variable") {
  auto result = parse(R"(
    typedef int INT;

    int main() {
      INT value = 10;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->type.datatype == DataType::TYPEDEF_NAME);

  REQUIRE(stmt->expr_ptr != nullptr);

  REQUIRE(stmt->expr_ptr->expr_type() == ExprType::INT_LITERAL);
}

TEST_CASE("Parser parses struct variable") {
  auto result = parse(R"(
    struct Point {
      int x;
    };

    int main() {
      struct Point p;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->type.datatype == DataType::STRUCT);
  REQUIRE(stmt->type.custom_name == "Point");
}

TEST_CASE("Parser parses struct pointer variable") {
  auto result = parse(R"(
    struct Point {
      int x;
    };

    int main() {
      struct Point *p;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.datatype == DataType::STRUCT);
  REQUIRE(stmt->type.pointer_depth == 1);
}

TEST_CASE("Parser parses union variable") {
  auto result = parse(R"(
    union Value {
      int x;
    };

    int main() {
      union Value value;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.datatype == DataType::UNION);
  REQUIRE(stmt->type.custom_name == "Value");
}

TEST_CASE("Parser parses enum variable") {
  auto result = parse(R"(
    enum Color {
      RED
    };

    int main() {
      enum Color c;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.datatype == DataType::ENUM);
  REQUIRE(stmt->type.custom_name == "Color");
}

TEST_CASE("Parser parses typedef struct variable") {
  auto result = parse(R"(
    typedef struct {
      int x;
    } Point;

    int main() {
      Point p;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(stmt->type.custom_name == "Point");
}

TEST_CASE("Parser parses typedef union variable") {
  auto result = parse(R"(
    typedef union {
      int x;
    } Value;

    int main() {
      Value v;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(stmt->type.custom_name == "Value");
}

TEST_CASE("Parser parses typedef enum variable") {
  auto result = parse(R"(
    typedef enum {
      RED
    } Color;

    int main() {
      Color c;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(stmt->type.custom_name == "Color");
}

TEST_CASE("Parser parses typedef pointer variable") {
  auto result = parse(R"(
    typedef int *INT_PTR;

    int main() {
      INT_PTR ptr;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(stmt->type.custom_name == "INT_PTR");
}

TEST_CASE("Parser parses typedef array variable") {
  auto result = parse(R"(
    typedef int ARRAY[10];

    int main() {
      ARRAY values;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(stmt->type.custom_name == "ARRAY");
}

TEST_CASE("Parser parses pointer to typedef variable") {
  auto result = parse(R"(
    typedef int INT;

    int main() {
      INT *ptr;
    }
  )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.datatype == DataType::TYPEDEF_NAME);

  REQUIRE(stmt->type.pointer_depth == 1);
}

TEST_CASE("Parser parses integer type specifiers") {
  auto [source, expected] = GENERATE(
      table<std::string, DataType>({
          {"int", DataType::INT},
          {"signed", DataType::INT},
          {"signed int", DataType::INT},
          {"unsigned", DataType::UINT},
          {"unsigned int", DataType::UINT},
          {"short", DataType::SHORT},
          {"short int", DataType::SHORT},
          {"unsigned short", DataType::USHORT},
          {"unsigned short int", DataType::USHORT},
          {"long", DataType::LONG},
          {"long int", DataType::LONG},
          {"unsigned long", DataType::ULONG},
          {"unsigned long int", DataType::ULONG},
          {"long long", DataType::LONGLONG},
          {"long long int", DataType::LONGLONG},
          {"unsigned long long", DataType::ULONGLONG},
          {"unsigned long long int", DataType::ULONGLONG},
      }));

  std::string program = "int main() {\n";
  program += source;
  program += " value;\n}";

  auto result = parse(program);

  auto *fn = get_function_decl(result);

  REQUIRE(fn->declaration->body->statements.size() == 1);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->var_name == "value");
  REQUIRE(stmt->type.datatype == expected);
  REQUIRE(stmt->type.pointer_depth == 0);
  REQUIRE(stmt->type.dimensions.empty());
  REQUIRE(stmt->expr_ptr == nullptr);
  REQUIRE_FALSE(stmt->init.has_value());
}

TEST_CASE("Parser parses primitive type specifiers") {
  auto [source, expected] = GENERATE(
      table<std::string, DataType>({
          {"char", DataType::CHAR},
          {"unsigned char", DataType::UCHAR},
          {"float", DataType::FLOAT},
          {"double", DataType::DOUBLE},
          {"long double", DataType::LONGDOUBLE},
          {"void*", DataType::VOID},
      }));

  std::string program = "int main() {\n";
  program += source;
  program += " value;\n}";

  auto result = parse(program);

  auto *fn = get_function_decl(result);
  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->type.datatype == expected);

  if (expected == DataType::VOID) {
    REQUIRE(stmt->type.pointer_depth == 1);
  } else {
    REQUIRE(stmt->type.pointer_depth == 0);
  }
}

TEST_CASE("Parser parses single type qualifiers") {
  auto [source, qualifier] = GENERATE(
      table<std::string, uint8_t>({
          {"const int", static_cast<uint8_t>(TypeQualifier::CONST)},
          {"volatile int", static_cast<uint8_t>(TypeQualifier::VOLATILE)},
          {"restrict int*", static_cast<uint8_t>(TypeQualifier::RESTRICT)},
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "int main() {\n" + source +
        " value;\n"
        "}");

    auto *fn = get_function_decl(result);
    auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

    REQUIRE(stmt != nullptr);
    REQUIRE(stmt->type.qualifiers == qualifier);
  }
}

TEST_CASE("Parser parses qualifier combinations") {
  auto [source, qualifiers] = GENERATE(
      table<std::string, uint8_t>({
          {
              "const volatile int",
              static_cast<uint8_t>(TypeQualifier::CONST) |
                  static_cast<uint8_t>(TypeQualifier::VOLATILE),
          },
          {
              "const restrict int*",
              static_cast<uint8_t>(TypeQualifier::CONST) |
                  static_cast<uint8_t>(TypeQualifier::RESTRICT),
          },
          {
              "volatile restrict int*",
              static_cast<uint8_t>(TypeQualifier::VOLATILE) |
                  static_cast<uint8_t>(TypeQualifier::RESTRICT),
          },
          {
              "const volatile restrict int*",
              static_cast<uint8_t>(TypeQualifier::CONST) |
                  static_cast<uint8_t>(TypeQualifier::VOLATILE) |
                  static_cast<uint8_t>(TypeQualifier::RESTRICT),
          },
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "int main() {\n" + source +
        " value;\n"
        "}");

    auto *fn = get_function_decl(result);
    auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

    REQUIRE(stmt != nullptr);
    REQUIRE(stmt->type.qualifiers == qualifiers);
  }
}

TEST_CASE("Parser parses qualified pointer declarations") {
  auto [source, qualifiers, ptr_depth] = GENERATE(
      table<std::string, uint8_t, size_t>({
          {
              "const int *",
              static_cast<uint8_t>(TypeQualifier::CONST),
              1,
          },
          {
              "volatile int **",
              static_cast<uint8_t>(TypeQualifier::VOLATILE),
              2,
          },
          {
              "const volatile int ***",
              static_cast<uint8_t>(TypeQualifier::CONST) |
                  static_cast<uint8_t>(TypeQualifier::VOLATILE),
              3,
          },
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "int main() {\n" + source +
        " ptr;\n"
        "}");

    auto *fn = get_function_decl(result);

    auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

    REQUIRE(stmt != nullptr);

    REQUIRE(stmt->type.qualifiers == qualifiers);
    REQUIRE(stmt->type.pointer_depth == ptr_depth);
  }
}

TEST_CASE("Parser parses qualified arrays") {
  auto [source, qualifiers] = GENERATE(
      table<std::string, uint8_t>({
          {
              "const int",
              static_cast<uint8_t>(TypeQualifier::CONST),
          },
          {
              "volatile int",
              static_cast<uint8_t>(TypeQualifier::VOLATILE),
          },
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "int main() {\n" + source +
        " arr[10];\n"
        "}");

    auto *fn = get_function_decl(result);

    auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

    REQUIRE(stmt != nullptr);
    REQUIRE(stmt->type.qualifiers == qualifiers);
    REQUIRE(stmt->type.dimensions == std::vector<size_t>{10});
  }
}

TEST_CASE("Parser parses qualified typedef variables") {
  auto [source, qualifiers] = GENERATE(
      table<std::string, uint8_t>({
          {
              "const INT",
              static_cast<uint8_t>(TypeQualifier::CONST),
          },
          {
              "volatile INT",
              static_cast<uint8_t>(TypeQualifier::VOLATILE),
          },
          {
              "const volatile INT",
              static_cast<uint8_t>(TypeQualifier::CONST) |
                  static_cast<uint8_t>(TypeQualifier::VOLATILE),
          },
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "typedef int INT;\n"
        "int main() {\n" +
        source +
        " value;\n"
        "}");

    auto *fn = get_function_decl(result);

    auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

    REQUIRE(stmt != nullptr);
    REQUIRE(stmt->type.datatype == DataType::TYPEDEF_NAME);
    REQUIRE(stmt->type.custom_name == "INT");
    REQUIRE(stmt->type.qualifiers == qualifiers);
  }
}

TEST_CASE("Parser parses storage class specifiers") {
  auto [source, storage] = GENERATE(
      table<std::string, StorageClass>({
          {"static int", StorageClass::STATIC},
          {"extern int", StorageClass::EXTERN},
          {"register int", StorageClass::REGISTER},
          {"auto int", StorageClass::AUTO},
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "int main() {\n" + source +
        " value;\n"
        "}");

    auto *fn = get_function_decl(result);

    auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

    REQUIRE(stmt != nullptr);
    REQUIRE(stmt->type.storage == storage);
    REQUIRE(stmt->type.datatype == DataType::INT);
  }
}

TEST_CASE("Parser parses storage classes with qualifiers") {
  auto [source, storage, qualifiers] = GENERATE(
      table<std::string, StorageClass, uint8_t>({
          {
              "static const int",
              StorageClass::STATIC,
              static_cast<uint8_t>(TypeQualifier::CONST),
          },
          {
              "extern volatile int",
              StorageClass::EXTERN,
              static_cast<uint8_t>(TypeQualifier::VOLATILE),
          },
          {
              "register const volatile int",
              StorageClass::REGISTER,
              static_cast<uint8_t>(TypeQualifier::CONST) |
                  static_cast<uint8_t>(TypeQualifier::VOLATILE),
          },
          {
              "auto const int",
              StorageClass::AUTO,
              static_cast<uint8_t>(TypeQualifier::CONST),
          },
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "int main() {\n" + source +
        " value;\n"
        "}");

    auto *fn = get_function_decl(result);

    auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

    REQUIRE(stmt != nullptr);
    REQUIRE(stmt->type.storage == storage);
    REQUIRE(stmt->type.qualifiers == qualifiers);
  }
}

TEST_CASE("Parser parses storage class pointer declarations") {
  auto [source, storage, ptr_depth] = GENERATE(
      table<std::string, StorageClass, size_t>({
          {
              "static int *",
              StorageClass::STATIC,
              1,
          },
          {
              "extern int **",
              StorageClass::EXTERN,
              2,
          },
          {
              "register char ***",
              StorageClass::REGISTER,
              3,
          },
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "int main() {\n" + source +
        " ptr;\n"
        "}");

    auto *fn = get_function_decl(result);

    auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

    REQUIRE(stmt != nullptr);
    REQUIRE(stmt->type.storage == storage);
    REQUIRE(stmt->type.pointer_depth == ptr_depth);
  }
}

TEST_CASE("Parser parses storage class array declarations") {
  auto [source, storage] = GENERATE(
      table<std::string, StorageClass>({
          {"static int", StorageClass::STATIC},
          {"extern int", StorageClass::EXTERN},
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "int main() {\n" + source +
        " arr[5][10];\n"
        "}");

    auto *fn = get_function_decl(result);
    auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

    REQUIRE(stmt != nullptr);
    REQUIRE(stmt->type.storage == storage);
    REQUIRE(stmt->type.dimensions == std::vector<size_t>{5, 10});
  }
}

TEST_CASE("Parser parses storage class with typedef type") {
  auto [source, storage] = GENERATE(
      table<std::string, StorageClass>({
          {"static INT", StorageClass::STATIC},
          {"extern INT", StorageClass::EXTERN},
          {"register INT", StorageClass::REGISTER},
          {"auto INT", StorageClass::AUTO},
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "typedef int INT;\n"
        "int main() {\n" +
        source +
        " value;\n"
        "}");

    auto *fn = get_function_decl(result);
    auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

    REQUIRE(stmt != nullptr);
    REQUIRE(stmt->type.storage == storage);
    REQUIRE(stmt->type.datatype == DataType::TYPEDEF_NAME);
    REQUIRE(stmt->type.custom_name == "INT");
  }
}

TEST_CASE("Parser accepts different integer specifier orders") {
  auto source = GENERATE(
      "unsigned long int",
      "long unsigned int",
      "int unsigned long",
      "long int unsigned",
      "unsigned int long");

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "int main() {\n" + std::string(source) +
        " value;\n"
        "}");

    auto *fn = get_function_decl(result);

    auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

    REQUIRE(stmt != nullptr);

    REQUIRE(stmt->type.datatype == DataType::ULONG);
  }
}

TEST_CASE("Parser accepts different long long specifier orders") {
  auto source = GENERATE(
      "unsigned long long",
      "long unsigned long",
      "long long unsigned",
      "unsigned long long int",
      "long unsigned long int");

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "int main() {\n" + std::string(source) +
        " value;\n"
        "}");

    auto *fn = get_function_decl(result);
    auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

    REQUIRE(stmt != nullptr);
    REQUIRE(stmt->type.datatype == DataType::ULONGLONG);
  }
}

TEST_CASE("Parser accepts different qualifier orders") {
  auto source = GENERATE(
      "const volatile int",
      "volatile const int",
      "int const volatile",
      "int volatile const");

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "int main() {\n" + std::string(source) +
        " value;\n"
        "}");

    auto *fn = get_function_decl(result);

    auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

    REQUIRE(stmt != nullptr);

    REQUIRE(stmt->type.qualifiers == (static_cast<uint8_t>(TypeQualifier::CONST) | static_cast<uint8_t>(TypeQualifier::VOLATILE)));
  }
}

TEST_CASE("Parser accepts storage class before or after qualifiers") {
  auto source = GENERATE(
      "static const int",
      "const static int",
      "volatile static int",
      "static volatile int");

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "int main() {\n" + std::string(source) +
        " value;\n"
        "}");

    auto *fn = get_function_decl(result);
    auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

    REQUIRE(stmt != nullptr);
    REQUIRE(stmt->type.storage == StorageClass::STATIC);
  }
}

TEST_CASE("Parser accepts mixed declaration specifier order") {
  auto source = GENERATE(
      "static const unsigned long int",
      "const static unsigned long int",
      "unsigned static const long int",
      "long unsigned static const int",
      "const unsigned static long int");

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "int main() {\n" + std::string(source) +
        " value;\n"
        "}");

    auto *fn = get_function_decl(result);

    auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

    REQUIRE(stmt != nullptr);
    REQUIRE(stmt->type.storage == StorageClass::STATIC);
    REQUIRE(stmt->type.qualifiers == static_cast<uint8_t>(TypeQualifier::CONST));
    REQUIRE(stmt->type.datatype == DataType::ULONG);
  }
}

TEST_CASE("Parser parses global int variable") {
  auto result = parse(R"(
    int x;
  )");

  REQUIRE(result.program.statements.size() == 1);

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->type.datatype == DataType::INT);
  REQUIRE(stmt->declaration->name == "x");

  REQUIRE(stmt->declaration->initializer == nullptr);
  REQUIRE_FALSE(stmt->declaration->array_initializer.has_value());
}

TEST_CASE("Parser parses initialized global variable") {
  auto result = parse(R"(
    int x = 42;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->initializer != nullptr);

  REQUIRE(stmt->declaration->initializer->expr_type() == ExprType::INT_LITERAL);
}

TEST_CASE("Parser parses global pointer") {
  auto result = parse(R"(
    int *ptr;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->type.pointer_depth == 1);
}

TEST_CASE("Parser parses global double pointer") {
  auto result = parse(R"(
    int **ptr;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->type.pointer_depth == 2);
}

TEST_CASE("Parser parses global array") {
  auto result = parse(R"(
    int arr[10];
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->type.dimensions == std::vector<size_t>{10});
}

TEST_CASE("Parser parses multidimensional global array") {
  auto result = parse(R"(
    int matrix[3][4];
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->type.dimensions == std::vector<size_t>{3, 4});
}

TEST_CASE("Parser parses global initialized with binary expression") {
  auto result = parse(R"(
    int x = a + b * c;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->initializer->expr_type() == ExprType::BINARY);
}

TEST_CASE("Parser parses global initialized with function call") {
  auto result = parse(R"(
    int x = foo(1, 2);
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->initializer->expr_type() == ExprType::FUNCTION_CALL);
}

TEST_CASE("Parser parses global initialized with cast") {
  auto result = parse(R"(
    int x = (int)foo;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->initializer->expr_type() == ExprType::CAST);
}

TEST_CASE("Parser parses global initialized with conditional expression") {
  auto result = parse(R"(
    int x = cond ? a : b;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[0].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->initializer->expr_type() == ExprType::CONDITIONAL);
}

TEST_CASE("Parser parses global typedef variable") {
  auto result = parse(R"(
    typedef int INT;

    INT value;
  )");

  REQUIRE(result.program.statements.size() == 2);

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(stmt->declaration->type.custom_name == "INT");

  REQUIRE(stmt->declaration->name == "value");
}

TEST_CASE("Parser parses initialized global typedef variable") {
  auto result = parse(R"(
    typedef int INT;

    INT value = 10;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->declaration->initializer != nullptr);

  REQUIRE(stmt->declaration->initializer->expr_type() == ExprType::INT_LITERAL);
}

TEST_CASE("Parser parses global typedef pointer") {
  auto result = parse(R"(
    typedef int INT;

    INT *ptr;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(stmt->declaration->type.pointer_depth == 1);
}

TEST_CASE("Parser parses global typedef array") {
  auto result = parse(R"(
    typedef int INT;

    INT arr[10];
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->type.dimensions == std::vector<size_t>{10});
}

TEST_CASE("Parser parses global struct variable") {
  auto result = parse(R"(
    struct Point {
      int x;
    };

    struct Point p;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->type.datatype == DataType::STRUCT);
  REQUIRE(stmt->declaration->type.custom_name == "Point");
}

TEST_CASE("Parser parses global struct pointer") {
  auto result = parse(R"(
    struct Point {
      int x;
    };

    struct Point *ptr;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->type.pointer_depth == 1);
}

TEST_CASE("Parser parses global struct array") {
  auto result = parse(R"(
    struct Point {
      int x;
    };

    struct Point points[5];
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->type.dimensions == std::vector<size_t>{5});
}

TEST_CASE("Parser parses global union variable") {
  auto result = parse(R"(
    union Value {
      int x;
    };

    union Value value;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->type.datatype == DataType::UNION);
  REQUIRE(stmt->declaration->type.custom_name == "Value");
}

TEST_CASE("Parser parses global union pointer") {
  auto result = parse(R"(
    union Value {
      int x;
    };

    union Value *ptr;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->type.pointer_depth == 1);
}

TEST_CASE("Parser parses global enum variable") {
  auto result = parse(R"(
    enum Color {
      RED
    };

    enum Color color;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->type.datatype == DataType::ENUM);
  REQUIRE(stmt->declaration->type.custom_name == "Color");
}

TEST_CASE("Parser parses global enum initialized variable") {
  auto result = parse(R"(
    enum Color {
      RED
    };

    enum Color color = RED;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->initializer != nullptr);
  REQUIRE(stmt->declaration->initializer->expr_type() == ExprType::IDENTIFIER);
}

TEST_CASE("Parser parses global typedef struct variable") {
  auto result = parse(R"(
    typedef struct {
      int x;
    } Point;

    Point p;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(stmt->declaration->type.custom_name == "Point");
}

TEST_CASE("Parser parses global typedef union variable") {
  auto result = parse(R"(
    typedef union {
      int x;
    } Value;

    Value v;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->type.custom_name == "Value");
}

TEST_CASE("Parser parses global typedef enum variable") {
  auto result = parse(R"(
    typedef enum {
      RED
    } Color;

    Color c;
  )");

  auto *stmt = dynamic_cast<GlobalVariableDeclStmt *>(result.program.statements[1].get());

  REQUIRE(stmt != nullptr);
  REQUIRE(stmt->declaration->type.custom_name == "Color");
}