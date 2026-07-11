#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "common/test_utils.h"

TEST_CASE("Parser parses primitive function parameter types") {
  auto [source, expected] = GENERATE(
      table<std::string, DataType>({
          {"int", DataType::INT},
          {"unsigned", DataType::UINT},
          {"short", DataType::SHORT},
          {"unsigned short", DataType::USHORT},
          {"long", DataType::LONG},
          {"unsigned long", DataType::ULONG},
          {"long long", DataType::LONGLONG},
          {"unsigned long long", DataType::ULONGLONG},
          {"char", DataType::CHAR},
          {"unsigned char", DataType::UCHAR},
          {"float", DataType::FLOAT},
          {"double", DataType::DOUBLE},
          {"long double", DataType::LONGDOUBLE},
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        source + std::string(" foo(") +
        source +
        " value);");

    REQUIRE(result.program.statements.size() == 1);

    auto *fn = get_function_decl(result);

    REQUIRE(fn != nullptr);

    REQUIRE(fn->declaration->parameters.size() == 1);

    REQUIRE(fn->declaration->parameters[0].type.datatype == expected);
  }
}

TEST_CASE("Parser parses primitive function return types") {
  auto [source, expected] = GENERATE(
      table<std::string, DataType>({
          {"void", DataType::VOID},
          {"int", DataType::INT},
          {"unsigned", DataType::UINT},
          {"short", DataType::SHORT},
          {"unsigned short", DataType::USHORT},
          {"long", DataType::LONG},
          {"unsigned long", DataType::ULONG},
          {"long long", DataType::LONGLONG},
          {"unsigned long long", DataType::ULONGLONG},
          {"char", DataType::CHAR},
          {"unsigned char", DataType::UCHAR},
          {"float", DataType::FLOAT},
          {"double", DataType::DOUBLE},
          {"long double", DataType::LONGDOUBLE},
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        source + std::string(" foo();"));

    REQUIRE(result.program.statements.size() == 1);

    auto *fn = get_function_decl(result);

    REQUIRE(fn != nullptr);

    REQUIRE(fn->declaration->return_type.datatype == expected);
  }
}

TEST_CASE("Parser parses multiple primitive parameters") {
  auto result = parse(R"(
      int foo(
          int a,
          float b,
          double c,
          char d,
          unsigned long e
      );
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->parameters.size() == 5);

  REQUIRE(fn->declaration->parameters[0].type.datatype == DataType::INT);
  REQUIRE(fn->declaration->parameters[1].type.datatype == DataType::FLOAT);
  REQUIRE(fn->declaration->parameters[2].type.datatype == DataType::DOUBLE);
  REQUIRE(fn->declaration->parameters[3].type.datatype == DataType::CHAR);
  REQUIRE(fn->declaration->parameters[4].type.datatype == DataType::ULONG);
}

TEST_CASE("Parser parses primitive return type with primitive parameters") {
  auto result = parse(R"(
      unsigned long foo(
          int a,
          float b,
          char c
      );
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->return_type.datatype == DataType::ULONG);

  REQUIRE(fn->declaration->parameters.size() == 3);

  REQUIRE(fn->declaration->parameters[0].type.datatype == DataType::INT);
  REQUIRE(fn->declaration->parameters[1].type.datatype == DataType::FLOAT);
  REQUIRE(fn->declaration->parameters[2].type.datatype == DataType::CHAR);
}

TEST_CASE("Parser parses pointer function parameter types") {
  auto [source, datatype, depth] = GENERATE(
      table<std::string, DataType, size_t>({
          {"int *", DataType::INT, 1},
          {"int **", DataType::INT, 2},
          {"int ***", DataType::INT, 3},

          {"char *", DataType::CHAR, 1},
          {"float *", DataType::FLOAT, 1},
          {"double **", DataType::DOUBLE, 2},

          {"unsigned long *", DataType::ULONG, 1},
          {"unsigned long **", DataType::ULONG, 2},
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        source + std::string(" foo(") +
        source +
        " value);");

    auto *fn = get_function_decl(result);

    REQUIRE(fn != nullptr);
    REQUIRE(fn->declaration->parameters.size() == 1);

    auto &param = fn->declaration->parameters[0];

    REQUIRE(param.type.datatype == datatype);
    REQUIRE(param.type.pointer_depth == depth);
  }
}

TEST_CASE("Parser parses pointer return types") {
  auto [source, datatype, depth] = GENERATE(
      table<std::string, DataType, size_t>({
          {"int *", DataType::INT, 1},
          {"int **", DataType::INT, 2},
          {"int ***", DataType::INT, 3},

          {"char *", DataType::CHAR, 1},
          {"float *", DataType::FLOAT, 1},
          {"double **", DataType::DOUBLE, 2},

          {"unsigned long *", DataType::ULONG, 1},
          {"unsigned long **", DataType::ULONG, 2},
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        source + std::string(" foo();"));

    auto *fn = get_function_decl(result);

    REQUIRE(fn != nullptr);

    REQUIRE(fn->declaration->return_type.datatype == datatype);
    REQUIRE(fn->declaration->return_type.pointer_depth == depth);
  }
}

TEST_CASE("Parser parses multiple pointer parameters") {
  auto result = parse(R"(
      int foo(
          int *a,
          int **b,
          int ***c,
          char *d,
          unsigned long **e
      );
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->parameters.size() == 5);

  REQUIRE(fn->declaration->parameters[0].type.pointer_depth == 1);
  REQUIRE(fn->declaration->parameters[1].type.pointer_depth == 2);
  REQUIRE(fn->declaration->parameters[2].type.pointer_depth == 3);
  REQUIRE(fn->declaration->parameters[3].type.pointer_depth == 1);
  REQUIRE(fn->declaration->parameters[4].type.pointer_depth == 2);
}

TEST_CASE("Parser parses pointer return with pointer parameters") {
  auto result = parse(R"(
      int **foo(
          int *a,
          int **b,
          int ***c
      );
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->return_type.pointer_depth == 2);

  REQUIRE(fn->declaration->parameters.size() == 3);

  REQUIRE(fn->declaration->parameters[0].type.pointer_depth == 1);
  REQUIRE(fn->declaration->parameters[1].type.pointer_depth == 2);
  REQUIRE(fn->declaration->parameters[2].type.pointer_depth == 3);
}

TEST_CASE("Parser parses qualified pointer parameters") {
  auto [source, qualifiers, depth] = GENERATE(
      table<std::string, uint8_t, size_t>({
          {"const int *",
           static_cast<uint8_t>(TypeQualifier::CONST),
           1},
          {"volatile int **",
           static_cast<uint8_t>(TypeQualifier::VOLATILE),
           2},
          {"const volatile unsigned long ***",
           static_cast<uint8_t>(TypeQualifier::CONST) |
               static_cast<uint8_t>(TypeQualifier::VOLATILE),
           3},
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        std::string("void foo(") +
        source +
        " p);");

    auto *fn = get_function_decl(result);

    REQUIRE(fn != nullptr);

    auto &param = fn->declaration->parameters[0];

    REQUIRE(param.type.pointer_depth == depth);
    REQUIRE(param.type.qualifiers == qualifiers);
  }
}

TEST_CASE("Parser parses typedef parameter") {
  auto result = parse(R"(
      typedef int INT;

      void foo(INT value);
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->parameters.size() == 1);

  auto &param = fn->declaration->parameters[0];

  REQUIRE(param.type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(param.type.custom_name == "INT");
}

TEST_CASE("Parser parses typedef return type") {
  auto result = parse(R"(
      typedef int INT;

      INT foo();
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->return_type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(fn->declaration->return_type.custom_name == "INT");
}

TEST_CASE("Parser parses typedef pointer parameter") {
  auto [source, depth] = GENERATE(
      table<std::string, size_t>({
          {"INT *", 1},
          {"INT **", 2},
          {"INT ***", 3},
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "typedef int INT;\n" + source +
        " foo(" + source +
        " value);");

    auto *fn = get_function_decl(result);

    REQUIRE(fn != nullptr);

    auto &param = fn->declaration->parameters[0];

    REQUIRE(param.type.datatype == DataType::TYPEDEF_NAME);
    REQUIRE(param.type.custom_name == "INT");
    REQUIRE(param.type.pointer_depth == depth);

    REQUIRE(fn->declaration->return_type.pointer_depth == depth);
  }
}

TEST_CASE("Parser parses typedef array parameter") {
  auto result = parse(R"(
      typedef int INT;

      void foo(INT arr[10]);
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  auto &param = fn->declaration->parameters[0];

  REQUIRE(param.type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(param.type.custom_name == "INT");

  REQUIRE(param.type.dimensions == std::vector<size_t>{10});
}

TEST_CASE("Parser parses multiple typedef parameters") {
  auto result = parse(R"(
      typedef int INT;

      void foo(
          INT a,
          INT *b,
          INT **c,
          INT d
      );
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->parameters.size() == 4);

  REQUIRE(fn->declaration->parameters[0].type.pointer_depth == 0);
  REQUIRE(fn->declaration->parameters[1].type.pointer_depth == 1);
  REQUIRE(fn->declaration->parameters[2].type.pointer_depth == 2);
  REQUIRE(fn->declaration->parameters[3].type.pointer_depth == 0);
}

TEST_CASE("Parser parses qualified typedef parameter") {
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
        "void foo(" +
        source +
        " value);");

    auto *fn = get_function_decl(result);

    REQUIRE(fn != nullptr);

    auto &param = fn->declaration->parameters[0];

    REQUIRE(param.type.datatype == DataType::TYPEDEF_NAME);
    REQUIRE(param.type.custom_name == "INT");
    REQUIRE(param.type.qualifiers == qualifiers);
  }
}

TEST_CASE("Parser parses typedef mixed with primitive parameters") {
  auto result = parse(R"(
      typedef int INT;

      INT foo(
          INT a,
          int b,
          INT *c,
          float d,
          unsigned long e
      );
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->return_type.datatype == DataType::TYPEDEF_NAME);

  REQUIRE(fn->declaration->parameters.size() == 5);

  REQUIRE(fn->declaration->parameters[0].type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(fn->declaration->parameters[1].type.datatype == DataType::INT);
  REQUIRE(fn->declaration->parameters[2].type.pointer_depth == 1);
  REQUIRE(fn->declaration->parameters[3].type.datatype == DataType::FLOAT);
  REQUIRE(fn->declaration->parameters[4].type.datatype == DataType::ULONG);
}

TEST_CASE("Parser parses struct parameter") {
  auto result = parse(R"(
      struct Point {
          int x;
          int y;
      };

      void foo(struct Point p);
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->parameters.size() == 1);

  auto &param = fn->declaration->parameters[0];

  REQUIRE(param.type.datatype == DataType::STRUCT);
  REQUIRE(param.type.custom_name == "Point");
}

TEST_CASE("Parser parses struct pointer parameters") {
  auto [source, depth] = GENERATE(
      table<std::string, size_t>({
          {"struct Point *", 1},
          {"struct Point **", 2},
          {"struct Point ***", 3},
      }));

  DYNAMIC_SECTION(source) {
    auto result = parse(
        "struct Point { int x; };\n" + source +
        " foo(" + source +
        " value);");

    auto *fn = get_function_decl(result);

    REQUIRE(fn != nullptr);

    auto &param = fn->declaration->parameters[0];

    REQUIRE(param.type.datatype == DataType::STRUCT);
    REQUIRE(param.type.custom_name == "Point");
    REQUIRE(param.type.pointer_depth == depth);

    REQUIRE(fn->declaration->return_type.pointer_depth == depth);
  }
}

TEST_CASE("Parser parses struct array parameter") {
  auto result = parse(R"(
      struct Point { int x; };

      void foo(struct Point arr[10]);
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  auto &param = fn->declaration->parameters[0];

  REQUIRE(param.type.datatype == DataType::STRUCT);
  REQUIRE(param.type.custom_name == "Point");

  REQUIRE(param.type.dimensions == std::vector<size_t>{10});
}

TEST_CASE("Parser parses struct return type") {
  auto result = parse(R"(
      struct Point { int x; };

      struct Point foo();
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->return_type.datatype == DataType::STRUCT);
  REQUIRE(fn->declaration->return_type.custom_name == "Point");
}

TEST_CASE("Parser parses multiple struct parameters") {
  auto result = parse(R"(
      struct Point { int x; };

      void foo(
          struct Point a,
          struct Point *b,
          struct Point **c,
          struct Point d
      );
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->parameters.size() == 4);

  REQUIRE(fn->declaration->parameters[0].type.pointer_depth == 0);
  REQUIRE(fn->declaration->parameters[1].type.pointer_depth == 1);
  REQUIRE(fn->declaration->parameters[2].type.pointer_depth == 2);
  REQUIRE(fn->declaration->parameters[3].type.pointer_depth == 0);
}

TEST_CASE("Parser parses struct mixed with primitive parameters") {
  auto result = parse(R"(
      struct Point {
          int x;
      };

      struct Point foo(
          int a,
          struct Point b,
          float c,
          struct Point *d,
          unsigned long e
      );
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->return_type.datatype == DataType::STRUCT);
  REQUIRE(fn->declaration->return_type.custom_name == "Point");

  REQUIRE(fn->declaration->parameters.size() == 5);

  REQUIRE(fn->declaration->parameters[0].type.datatype == DataType::INT);
  REQUIRE(fn->declaration->parameters[1].type.datatype == DataType::STRUCT);
  REQUIRE(fn->declaration->parameters[2].type.datatype == DataType::FLOAT);
  REQUIRE(fn->declaration->parameters[3].type.pointer_depth == 1);
  REQUIRE(fn->declaration->parameters[4].type.datatype == DataType::ULONG);
}

TEST_CASE("Parser parses union parameter") {
  auto result = parse(R"(
      union Value {
          int i;
          float f;
      };

      void foo(union Value value);
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  auto &param = fn->declaration->parameters[0];

  REQUIRE(param.type.datatype == DataType::UNION);
  REQUIRE(param.type.custom_name == "Value");
}

TEST_CASE("Parser parses union return type") {
  auto result = parse(R"(
      union Value {
          int i;
      };

      union Value foo();
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->return_type.datatype == DataType::UNION);
  REQUIRE(fn->declaration->return_type.custom_name == "Value");
}

TEST_CASE("Parser parses enum parameter") {
  auto result = parse(R"(
      enum Color {
          RED,
          GREEN
      };

      void foo(enum Color c);
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  auto &param = fn->declaration->parameters[0];

  REQUIRE(param.type.datatype == DataType::ENUM);
  REQUIRE(param.type.custom_name == "Color");
}

TEST_CASE("Parser parses enum pointer parameter") {
  auto result = parse(R"(
      enum Color {
          RED
      };

      void foo(enum Color *c);
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  auto &param = fn->declaration->parameters[0];

  REQUIRE(param.type.datatype == DataType::ENUM);
  REQUIRE(param.type.pointer_depth == 1);
}

TEST_CASE("Parser parses enum return type") {
  auto result = parse(R"(
      enum Color {
          RED
      };

      enum Color foo();
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->return_type.datatype == DataType::ENUM);
  REQUIRE(fn->declaration->return_type.custom_name == "Color");
}

TEST_CASE("Parser parses typedef'd struct parameter") {
  auto result = parse(R"(
      typedef struct {
          int x;
          int y;
      } Point;

      void foo(Point p);
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  auto &param = fn->declaration->parameters[0];

  REQUIRE(param.type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(param.type.custom_name == "Point");
}

TEST_CASE("Parser parses typedef'd struct return type") {
  auto result = parse(R"(
      typedef struct {
          int x;
      } Point;

      Point foo();
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->return_type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(fn->declaration->return_type.custom_name == "Point");
}

TEST_CASE("Parser parses typedef'd struct pointers") {
  auto [source, depth] = GENERATE(
      table<std::string, size_t>({
          {"Point *", 1},
          {"Point **", 2},
          {"Point ***", 3},
      }));

  DYNAMIC_SECTION(source) {

    auto result = parse(
        "typedef struct { int x; } Point;\n" + source +
        " foo(" + source +
        " value);");

    auto *fn = get_function_decl(result);

    REQUIRE(fn != nullptr);

    auto &param = fn->declaration->parameters[0];

    REQUIRE(param.type.datatype == DataType::TYPEDEF_NAME);
    REQUIRE(param.type.custom_name == "Point");
    REQUIRE(param.type.pointer_depth == depth);

    REQUIRE(fn->declaration->return_type.pointer_depth == depth);
  }
}

TEST_CASE("Parser parses typedef'd union parameter") {
  auto result = parse(R"(
      typedef union {
          int i;
          float f;
      } Value;

      void foo(Value value);
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  auto &param = fn->declaration->parameters[0];

  REQUIRE(param.type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(param.type.custom_name == "Value");
}

TEST_CASE("Parser parses typedef'd union pointer") {
  auto result = parse(R"(
      typedef union {
          int i;
      } Value;

      Value *foo(Value *value);
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->return_type.pointer_depth == 1);

  auto &param = fn->declaration->parameters[0];

  REQUIRE(param.type.pointer_depth == 1);
}

TEST_CASE("Parser parses typedef'd enum parameter") {
  auto result = parse(R"(
      typedef enum {
          RED,
          GREEN
      } Color;

      void foo(Color c);
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  auto &param = fn->declaration->parameters[0];

  REQUIRE(param.type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(param.type.custom_name == "Color");
}

TEST_CASE("Parser parses typedef'd enum pointer") {
  auto result = parse(R"(
      typedef enum {
          RED
      } Color;

      Color *foo(Color *c);
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->return_type.pointer_depth == 1);

  auto &param = fn->declaration->parameters[0];

  REQUIRE(param.type.pointer_depth == 1);
}

TEST_CASE("Parser parses mixed typedef parameter list") {
  auto result = parse(R"(
      typedef struct {
          int x;
      } Point;

      typedef union {
          int i;
      } Value;

      typedef enum {
          RED
      } Color;

      void foo(
          Point p,
          Value v,
          Color c,
          Point *pp,
          Value **vp,
          Color *cp,
          int x,
          float y,
          unsigned long z
      );
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->parameters.size() == 9);

  REQUIRE(fn->declaration->parameters[0].type.custom_name == "Point");
  REQUIRE(fn->declaration->parameters[1].type.custom_name == "Value");
  REQUIRE(fn->declaration->parameters[2].type.custom_name == "Color");

  REQUIRE(fn->declaration->parameters[3].type.pointer_depth == 1);
  REQUIRE(fn->declaration->parameters[4].type.pointer_depth == 2);
  REQUIRE(fn->declaration->parameters[5].type.pointer_depth == 1);

  REQUIRE(fn->declaration->parameters[6].type.datatype == DataType::INT);
  REQUIRE(fn->declaration->parameters[7].type.datatype == DataType::FLOAT);
  REQUIRE(fn->declaration->parameters[8].type.datatype == DataType::ULONG);
}

TEST_CASE("Parser parses mixed typedef return type") {
  auto result = parse(R"(
      typedef struct {
          int x;
      } Point;

      typedef union {
          int i;
      } Value;

      typedef enum {
          RED
      } Color;

      Point *foo(
          Value value,
          Color color,
          Point *parent
      );
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->return_type.custom_name == "Point");
  REQUIRE(fn->declaration->return_type.pointer_depth == 1);

  REQUIRE(fn->declaration->parameters.size() == 3);

  REQUIRE(fn->declaration->parameters[0].type.custom_name == "Value");
  REQUIRE(fn->declaration->parameters[1].type.custom_name == "Color");
  REQUIRE(fn->declaration->parameters[2].type.custom_name == "Point");
  REQUIRE(fn->declaration->parameters[2].type.pointer_depth == 1);
}

TEST_CASE("Parser parses typedef chains in function declarations") {
  auto result = parse(R"(
      typedef int INT;
      typedef INT INT2;
      typedef INT2 INT3;
      typedef INT3 INT4;

      INT4 foo(INT3 a, INT2 b, INT c);
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->return_type.custom_name == "INT4");

  REQUIRE(fn->declaration->parameters.size() == 3);

  REQUIRE(fn->declaration->parameters[0].type.custom_name == "INT3");
  REQUIRE(fn->declaration->parameters[1].type.custom_name == "INT2");
  REQUIRE(fn->declaration->parameters[2].type.custom_name == "INT");
}

TEST_CASE("Parser parses many mixed parameter types") {
  auto result = parse(R"(
      typedef int INT;

      typedef struct {
          int x;
      } Point;

      typedef union {
          int value;
      } Value;

      typedef enum {
          RED,
          BLUE
      } Color;

      void foo(
          INT a,
          Point p,
          Value v,
          Color c,
          int *p1,
          float *p2,
          Point *p3,
          Value **p4,
          Color *p5,
          unsigned long x,
          char y,
          double z
      );
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->parameters.size() == 12);
}

TEST_CASE("Parser parses deeply nested pointer declarations") {
  auto result = parse(R"(
      typedef struct {
          int x;
      } Point;

      Point ***foo(
          Point ***a,
          Point **b,
          Point *c
      );
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->return_type.pointer_depth == 3);

  REQUIRE(fn->declaration->parameters[0].type.pointer_depth == 3);
  REQUIRE(fn->declaration->parameters[1].type.pointer_depth == 2);
  REQUIRE(fn->declaration->parameters[2].type.pointer_depth == 1);
}

TEST_CASE("Parser parses array parameters with pointer parameters") {
  auto result = parse(R"(
      typedef int INT;

      void foo(
          INT values[10],
          INT *ptr,
          INT matrix[5][6],
          INT **ptr2
      );
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->parameters.size() == 4);
  REQUIRE(fn->declaration->parameters[0].type.dimensions == std::vector<size_t>{10});
  REQUIRE(fn->declaration->parameters[1].type.pointer_depth == 1);
  REQUIRE(fn->declaration->parameters[2].type.dimensions == std::vector<size_t>{5, 6});
  REQUIRE(fn->declaration->parameters[3].type.pointer_depth == 2);
}

TEST_CASE("Parser parses every user defined type together") {
  auto result = parse(R"(
      typedef struct {
          int x;
      } Point;

      typedef union {
          int value;
      } Value;

      typedef enum {
          RED,
          BLUE
      } Color;

      Point foo(
          Point p,
          Value v,
          Color c,
          Point *pp,
          Value *vp,
          Color *cp
      );
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->return_type.custom_name == "Point");

  REQUIRE(fn->declaration->parameters.size() == 6);
}

TEST_CASE("Parser parses qualified function parameters") {
  auto result = parse(R"(
      typedef int INT;

      void foo(
          const INT a,
          volatile INT *b,
          const unsigned long c,
          volatile double *d
      );
  )");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->parameters.size() == 4);
  REQUIRE(fn->declaration->parameters[0].type.qualifiers & static_cast<uint8_t>(TypeQualifier::CONST));
  REQUIRE(fn->declaration->parameters[1].type.qualifiers & static_cast<uint8_t>(TypeQualifier::VOLATILE));
  REQUIRE(fn->declaration->parameters[2].type.qualifiers & static_cast<uint8_t>(TypeQualifier::CONST));
  REQUIRE(fn->declaration->parameters[3].type.qualifiers & static_cast<uint8_t>(TypeQualifier::VOLATILE));
}

TEST_CASE("Parser parses const pointer") {
  auto result = parse(R"(
        int main() {
            int * const ptr;
        }
    )");

  auto *fn = get_function_decl(result);
  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt != nullptr);

  REQUIRE(stmt->type.pointer_depth == 1);
  REQUIRE(stmt->type.pointer_qualifiers.size() == 1);
  REQUIRE(stmt->type.pointer_qualifiers[0] & static_cast<uint8_t>(TypeQualifier::CONST));
}

TEST_CASE("Parser parses volatile pointer") {
  auto result = parse(R"(
        int main() {
            int * volatile ptr;
        }
    )");

  auto *fn = get_function_decl(result);
  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.pointer_depth == 1);
  REQUIRE(stmt->type.pointer_qualifiers[0] & static_cast<uint8_t>(TypeQualifier::VOLATILE));
}

TEST_CASE("Parser parses restrict pointer") {
  auto result = parse(R"(
        int main() {
            int * restrict ptr;
        }
    )");

  auto *fn = get_function_decl(result);
  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.pointer_depth == 1);
  REQUIRE(stmt->type.pointer_qualifiers[0] & static_cast<uint8_t>(TypeQualifier::RESTRICT));
}

TEST_CASE("Parser parses pointer with all qualifiers") {
  auto result = parse(R"(
        int main() {
            int * const volatile restrict ptr;
        }
    )");

  auto *fn = get_function_decl(result);
  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  auto flags = stmt->type.pointer_qualifiers[0];

  REQUIRE(flags & static_cast<uint8_t>(TypeQualifier::CONST));
  REQUIRE(flags & static_cast<uint8_t>(TypeQualifier::VOLATILE));
  REQUIRE(flags & static_cast<uint8_t>(TypeQualifier::RESTRICT));
}

TEST_CASE("Parser parses multiple qualified pointers") {
  auto result = parse(R"(
        int main() {
            int * const * volatile ptr;
        }
    )");

  auto *fn = get_function_decl(result);
  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.pointer_depth == 2);
  REQUIRE(stmt->type.pointer_qualifiers.size() == 2);
  REQUIRE(stmt->type.pointer_qualifiers[0] & static_cast<uint8_t>(TypeQualifier::CONST));
  REQUIRE(stmt->type.pointer_qualifiers[1] & static_cast<uint8_t>(TypeQualifier::VOLATILE));
}

TEST_CASE("Parser parses typedef qualified pointer") {
  auto result = parse(R"(
        typedef int INT;

        int main() {
            INT * const value;
        }
    )");

  auto *fn = get_function_decl(result);

  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(stmt->type.custom_name == "INT");
  REQUIRE(stmt->type.pointer_depth == 1);
  REQUIRE(stmt->type.pointer_qualifiers[0] & static_cast<uint8_t>(TypeQualifier::CONST));
}

TEST_CASE("Parser parses struct qualified pointer") {
  auto result = parse(R"(
        struct Node {
            int x;
        };

        int main() {
            struct Node * const ptr;
        }
    )");

  auto *fn = get_function_decl(result);
  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.datatype == DataType::STRUCT);
  REQUIRE(stmt->type.pointer_depth == 1);
  REQUIRE(stmt->type.pointer_qualifiers[0] & static_cast<uint8_t>(TypeQualifier::CONST));
}

TEST_CASE("Parser parses array of qualified pointers") {
  auto result = parse(R"(
        int main() {
            int * const arr[10];
        }
    )");

  auto *fn = get_function_decl(result);
  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.pointer_depth == 1);
  REQUIRE(stmt->type.dimensions == std::vector<size_t>{10});
  REQUIRE(stmt->type.pointer_qualifiers[0] & static_cast<uint8_t>(TypeQualifier::CONST));
}

TEST_CASE("Parser parses double pointer to typedef") {
  auto result = parse(R"(
        typedef int INT;

        int main() {
            INT ** value;
        }
    )");

  auto *fn = get_function_decl(result);
  auto *stmt = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[0].get());

  REQUIRE(stmt->type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(stmt->type.pointer_depth == 2);
}

TEST_CASE("Parser parses typedef cast") {
  auto result = parse(R"(
      typedef int INT;

      int main() {
          return (INT)x;
      }
  )");

  auto *ret = get_return_stmt(result);

  auto *cast = dynamic_cast<CastExpr *>(ret->expr_ptr.get());

  REQUIRE(cast != nullptr);
  REQUIRE(cast->target_type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(cast->target_type.custom_name == "INT");

  REQUIRE(dynamic_cast<IdentifierExpr *>(cast->expr.get()) != nullptr);
}

TEST_CASE("Parser parses typedef pointer cast") {
  auto result = parse(R"(
      typedef int INT;

      int main() {
          return (INT *)ptr;
      }
  )");

  auto *ret = get_return_stmt(result);

  auto *cast = dynamic_cast<CastExpr *>(ret->expr_ptr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->target_type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(cast->target_type.custom_name == "INT");
  REQUIRE(cast->target_type.pointer_depth == 1);

  auto *id = dynamic_cast<IdentifierExpr *>(cast->expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "ptr");
}

TEST_CASE("Parser parses typedef double pointer cast") {
  auto result = parse(R"(
      typedef int INT;

      int main() {
          return (INT **)ptr;
      }
  )");

  auto *ret = get_return_stmt(result);

  auto *cast = dynamic_cast<CastExpr *>(ret->expr_ptr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->target_type.datatype == DataType::TYPEDEF_NAME);
  REQUIRE(cast->target_type.custom_name == "INT");
  REQUIRE(cast->target_type.pointer_depth == 2);

  auto *id = dynamic_cast<IdentifierExpr *>(cast->expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "ptr");
}

TEST_CASE("Parser parses struct cast") {
  auto result = parse(R"(
      struct Point {
          int x;
      };

      int main() {
          return (struct Point)value;
      }
  )");

  auto *ret = get_return_stmt(result);

  auto *cast = dynamic_cast<CastExpr *>(ret->expr_ptr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->target_type.datatype == DataType::STRUCT);
  REQUIRE(cast->target_type.custom_name == "Point");

  auto *id = dynamic_cast<IdentifierExpr *>(cast->expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "value");
}

TEST_CASE("Parser parses struct pointer cast") {
  auto result = parse(R"(
      struct Point {
          int x;
      };

      int main() {
          return (struct Point *)ptr;
      }
  )");

  auto *ret = get_return_stmt(result);

  auto *cast = dynamic_cast<CastExpr *>(ret->expr_ptr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->target_type.datatype == DataType::STRUCT);
  REQUIRE(cast->target_type.custom_name == "Point");
  REQUIRE(cast->target_type.pointer_depth == 1);

  auto *id = dynamic_cast<IdentifierExpr *>(cast->expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "ptr");
}

TEST_CASE("Parser parses union pointer cast") {
  auto result = parse(R"(
      union Value {
          int x;
      };

      int main() {
          return (union Value *)ptr;
      }
  )");

  auto *ret = get_return_stmt(result);

  auto *cast = dynamic_cast<CastExpr *>(ret->expr_ptr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->target_type.datatype == DataType::UNION);
  REQUIRE(cast->target_type.custom_name == "Value");
  REQUIRE(cast->target_type.pointer_depth == 1);

  auto *id = dynamic_cast<IdentifierExpr *>(cast->expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "ptr");
}

TEST_CASE("Parser parses enum cast") {
  auto result = parse(R"(
      enum Color {
          RED
      };

      int main() {
          return (enum Color)x;
      }
  )");

  auto *ret = get_return_stmt(result);

  auto *cast = dynamic_cast<CastExpr *>(ret->expr_ptr.get());

  REQUIRE(cast != nullptr);

  REQUIRE(cast->target_type.datatype == DataType::ENUM);
  REQUIRE(cast->target_type.custom_name == "Color");

  auto *id = dynamic_cast<IdentifierExpr *>(cast->expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "x");
}

TEST_CASE("Parser parses sizeof typedef") {
  auto result = parse(R"(
      typedef int INT;

      int main() {
          return sizeof(INT);
      }
  )");

  auto *ret = get_return_stmt(result);

  auto *expr = dynamic_cast<SizeofExpr *>(ret->expr_ptr.get());

  REQUIRE(expr != nullptr);

  REQUIRE(expr->parsed_type.has_value());
  REQUIRE(expr->parsed_type->datatype == DataType::TYPEDEF_NAME);
  REQUIRE(expr->parsed_type->custom_name == "INT");
}

TEST_CASE("Parser parses sizeof struct type 2") {
  auto result = parse(R"(
      struct Point {
          int x;
      };

      int main() {
          return sizeof(struct Point);
      }
  )");

  auto *ret = get_return_stmt(result);

  auto *expr = dynamic_cast<SizeofExpr *>(ret->expr_ptr.get());

  REQUIRE(expr != nullptr);

  REQUIRE(expr->parsed_type.has_value());
  REQUIRE(expr->parsed_type->datatype == DataType::STRUCT);
  REQUIRE(expr->parsed_type->custom_name == "Point");
}

TEST_CASE("Parser parses sizeof union type 2") {
  auto result = parse(R"(
      union Value {
          int x;
      };

      int main() {
          return sizeof(union Value);
      }
  )");

  auto *ret = get_return_stmt(result);

  auto *expr = dynamic_cast<SizeofExpr *>(ret->expr_ptr.get());

  REQUIRE(expr != nullptr);

  REQUIRE(expr->parsed_type.has_value());
  REQUIRE(expr->parsed_type->datatype == DataType::UNION);
  REQUIRE(expr->parsed_type->custom_name == "Value");
}

TEST_CASE("Parser parses sizeof enum type 2") {
  auto result = parse(R"(
      enum Color {
          RED
      };

      int main() {
          return sizeof(enum Color);
      }
  )");

  auto *ret = get_return_stmt(result);

  auto *expr = dynamic_cast<SizeofExpr *>(ret->expr_ptr.get());

  REQUIRE(expr != nullptr);

  REQUIRE(expr->parsed_type.has_value());
  REQUIRE(expr->parsed_type->datatype == DataType::ENUM);
  REQUIRE(expr->parsed_type->custom_name == "Color");
}

TEST_CASE("Parser parses sizeof identifier expression 2") {
  auto result = parse(R"(
      int main() {
          return sizeof(x);
      }
  )");

  auto *ret = get_return_stmt(result);

  auto *expr = dynamic_cast<SizeofExpr *>(ret->expr_ptr.get());

  REQUIRE(expr != nullptr);

  REQUIRE_FALSE(expr->parsed_type.has_value());

  auto *id = dynamic_cast<IdentifierExpr *>(expr->expr.get());

  REQUIRE(id != nullptr);
  REQUIRE(id->identifier_name == "x");
}

TEST_CASE("Parser parses sizeof binary expression 2") {
  auto result = parse(R"(
      int main() {
          return sizeof(a + b);
      }
  )");

  auto *ret = get_return_stmt(result);

  auto *expr = dynamic_cast<SizeofExpr *>(ret->expr_ptr.get());

  REQUIRE(expr != nullptr);

  REQUIRE_FALSE(expr->parsed_type.has_value());

  auto *binary = dynamic_cast<BinaryExpr *>(expr->expr.get());

  REQUIRE(binary != nullptr);
}

TEST_CASE("Parser parses nested sizeof") {
  auto result = parse(R"(
      int main() {
          return sizeof(sizeof(int));
      }
  )");

  auto *ret = get_return_stmt(result);

  auto *outer = dynamic_cast<SizeofExpr *>(ret->expr_ptr.get());

  REQUIRE(outer != nullptr);

  REQUIRE_FALSE(outer->parsed_type.has_value());

  auto *inner = dynamic_cast<SizeofExpr *>(outer->expr.get());

  REQUIRE(inner != nullptr);
  REQUIRE(inner->parsed_type.has_value());
  REQUIRE(inner->parsed_type->datatype == DataType::INT);
}

TEST_CASE("Parser parses sizeof cast expression 2") {
  auto result = parse(R"(
      int main() {
          return sizeof((int)x);
      }
  )");

  auto *ret = get_return_stmt(result);

  auto *expr = dynamic_cast<SizeofExpr *>(ret->expr_ptr.get());

  REQUIRE(expr != nullptr);

  REQUIRE_FALSE(expr->parsed_type.has_value());

  auto *cast = dynamic_cast<CastExpr *>(expr->expr.get());

  REQUIRE(cast != nullptr);
  REQUIRE(cast->target_type.datatype == DataType::INT);
}