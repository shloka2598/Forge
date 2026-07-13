#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "common/test_utils.h"

static void require_error(const ParserResult &result, std::string_view message) {
  for (const auto &diag : result.diagnostics.get_diagnostics()) {
    if (diag.level == DiagnosticLevel::Error && diag.message == message) {
      SUCCEED();
      return;
    }
  }

  FAIL("Expected parser error not emitted.");
}

TEST_CASE("Parser reports missing primary expression") {
  auto result = parse(R"(
int main() {
    int x = ;
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing assignment expression") {
  auto result = parse(R"(
int main() {
    x = ;
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing opening parenthesis after if") {
  auto result = parse(R"(
int main() {
    if x) {
    }
}
)");

  require_error(result, "Expected '('");
}

TEST_CASE("Parser reports missing closing parenthesis after if condition") {
  auto result = parse(R"(
int main() {
    if (x + 5 {
    }
}
)");

  require_error(result, "Expected ')'");
}

TEST_CASE("Parser reports missing semicolon after return") {
  auto result = parse(R"(
int main() {
    return 5
}
)");

  require_error(result, "Expected ';'");
}

TEST_CASE("Parser rejects assignment to literal") {
  auto result = parse(R"(
int main() {
    5 = x;
}
)");

  require_error(result, "Left-hand side of assignment must be assignable.");
}

TEST_CASE("Parser rejects postfix increment on rvalue") {
  auto result = parse(R"(
int main() {
    (a + b)++;
}
)");

  require_error(result, "Operand of postfix '++' must be assignable.");
}

TEST_CASE("Parser rejects prefix increment on rvalue") {
  auto result = parse(R"(
int main() {
    ++(a + b);
}
)");

  require_error(result, "Operand of '++' must be assignable.");
}

TEST_CASE("Parser rejects postfix decrement on rvalue") {
  auto result = parse(R"(
int main() {
    (a + b)--;
}
)");

  require_error(result, "Operand of postfix '--' must be assignable.");
}

TEST_CASE("Parser rejects prefix decrement on rvalue") {
  auto result = parse(R"(
int main() {
    --(a + b);
}
)");

  require_error(result, "Operand of '--' must be assignable.");
}

TEST_CASE("Parser reports missing expression after unary plus") {
  auto result = parse(R"(
int main() {
    +;
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing expression after unary minus") {
  auto result = parse(R"(
int main() {
    -;
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing expression after logical not") {
  auto result = parse(R"(
int main() {
    !;
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing expression after bitwise not") {
  auto result = parse(R"(
int main() {
    ~;
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing expression after dereference") {
  auto result = parse(R"(
int main() {
    *;
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing expression after address-of") {
  auto result = parse(R"(
int main() {
    &;
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing expression after sizeof") {
  auto result = parse(R"(
int main() {
    sizeof;
}
)");

  require_error(result, "Expected an expression after 'sizeof'.");
}

TEST_CASE("Parser reports missing expression after prefix increment") {
  auto result = parse(R"(
int main() {
    ++;
}
)");

  require_error(result, "Expected an expression after '++'.");
}

TEST_CASE("Parser reports missing expression after prefix decrement") {
  auto result = parse(R"(
int main() {
    --;
}
)");

  require_error(result, "Expected an expression after '--'.");
}

TEST_CASE("Parser reports missing expression after ternary question mark") {
  auto result = parse(R"(
int main() {
    x ? : y;
}
)");

  require_error(result, "Expected an expression after '?'.");
}

TEST_CASE("Parser reports missing expression after ternary colon") {
  auto result = parse(R"(
int main() {
    x ? y : ;
}
)");

  require_error(result, "Expected an expression after ':'.");
}

TEST_CASE("Parser reports missing expression after compound assignment") {
  auto result = parse(R"(
int main() {
    x += ;
}
)");

  require_error(result, "Expected an expression after compound assignment operator.");
}

TEST_CASE("Parser reports missing closing parenthesis after function call") {
  auto result = parse(R"(
int main() {
    foo(1, 2;
}
)");

  require_error(result, "Expected ')'");
}

TEST_CASE("Parser reports missing closing bracket after array access") {
  auto result = parse(R"(
int main() {
    arr[5;
}
)");

  require_error(result, "Expected ']'");
}

TEST_CASE("Parser reports missing member name after dot") {
  auto result = parse(R"(
int main() {
    obj.;
}
)");

  require_error(result, "Expected a member name after '.'.");
}

TEST_CASE("Parser reports missing member name after arrow") {
  auto result = parse(R"(
int main() {
    ptr->;
}
)");

  require_error(result, "Expected a member name after '->'.");
}

TEST_CASE("Parser rejects function call on non identifier") {
  auto result = parse(R"(
int main() {
    (a + b)(1);
}
)");

  require_error(result, "Expected an identifier before '('.");
}

TEST_CASE("Parser reports missing cast expression operand") {
  auto result = parse(R"(
int main() {
    (int);
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing function call argument") {
  auto result = parse(R"(
int main() {
    foo(1, );
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing array index expression") {
  auto result = parse(R"(
int main() {
    arr[];
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing variable name") {
  auto result = parse(R"(
int main() {
    int;
}
)");

  require_error(result, "Expected a variable name.");
}

TEST_CASE("Parser reports missing typedef name") {
  auto result = parse(R"(
typedef int;
)");

  require_error(result, "Expected a typedef name.");
}

TEST_CASE("Parser rejects typedef initializer") {
  auto result = parse(R"(
typedef int MyInt = 5;
)");

  require_error(result, "A typedef declaration cannot have an initializer.");
}

TEST_CASE("Parser reports missing global variable name") {
  auto result = parse(R"(
int;
int *;
)");

  require_error(result, "Expected a global variable name.");
}

TEST_CASE("Parser reports missing struct field name") {
  auto result = parse(R"(
struct S {
    int;
};
)");

  require_error(result, "Expected a struct field name.");
}

TEST_CASE("Parser reports missing union field name") {
  auto result = parse(R"(
union U {
    int;
};
)");

  require_error(result, "Expected a union field name.");
}

TEST_CASE("Parser reports missing enum member name") {
  auto result = parse(R"(
enum E {
    = 5
};
)");

  require_error(result, "Expected an enum member name.");
}

TEST_CASE("Parser reports missing enum initializer") {
  auto result = parse(R"(
enum E {
    A =
};
)");

  require_error(result, "Expected an enumerator initializer after '='.");
}

TEST_CASE("Parser reports missing array size") {
  auto result = parse(R"(
int main() {
    int arr[abc];
}
)");

  require_error(result, "Expected an integer array size.");
}

TEST_CASE("Parser reports missing closing array bracket") {
  auto result = parse(R"(
int main() {
    int arr[5;
}
)");

  require_error(result, "Expected ']'");
}

TEST_CASE("Parser reports missing array initializer") {
  auto result = parse(R"(
int arr[3] = 5;
)");

  require_error(result, "Expected an array initializer after '='.");
}

TEST_CASE("Parser reports missing initializer expression") {
  auto result = parse(R"(
int main() {
    int x = ;
}
)");

  require_error(result, "Expected an initializer expression after '='.");
}

TEST_CASE("Parser rejects duplicate const qualifier") {
  auto result = parse(R"(
int main() {
    const const int x;
}
)");

  require_error(result, "Duplicate 'const' qualifier.");
}

TEST_CASE("Parser rejects duplicate volatile qualifier") {
  auto result = parse(R"(
int main() {
    volatile volatile int x;
}
)");

  require_error(result, "Duplicate 'volatile' qualifier.");
}

TEST_CASE("Parser rejects duplicate restrict qualifier") {
  auto result = parse(R"(
int main() {
    restrict restrict int *p;
}
)");

  require_error(result, "Duplicate 'restrict' qualifier.");
}

TEST_CASE("Parser rejects multiple storage classes") {
  auto result = parse(R"(
static extern int x;
)");

  require_error(result, "Multiple storage class specifiers are not allowed.");
}

TEST_CASE("Parser rejects duplicate inline specifier") {
  auto result = parse(R"(
inline inline int foo();
)");

  require_error(result, "Duplicate 'inline' specifier.");
}

TEST_CASE("Parser rejects duplicate signed specifier") {
  auto result = parse(R"(
signed signed int x;
)");

  require_error(result, "Duplicate 'signed' specifier.");
}

TEST_CASE("Parser rejects duplicate unsigned specifier") {
  auto result = parse(R"(
unsigned unsigned int x;
)");

  require_error(result, "Duplicate 'unsigned' specifier.");
}

TEST_CASE("Parser rejects duplicate short specifier") {
  auto result = parse(R"(
short short int x;
)");

  require_error(result, "Duplicate 'short' specifier.");
}

TEST_CASE("Parser rejects too many long specifiers") {
  auto result = parse(R"(
long long long int x;
)");

  require_error(result, "Too many 'long' specifiers.");
}

TEST_CASE("Parser rejects multiple base types") {
  auto result = parse(R"(
int float x;
)");

  require_error(result, "Multiple base types specified.");
}

TEST_CASE("Parser rejects signed unsigned combination") {
  auto result = parse(R"(
signed unsigned int x;
)");

  require_error(result, "Type cannot be both 'signed' and 'unsigned'.");
}

TEST_CASE("Parser rejects short long combination") {
  auto result = parse(R"(
short long int x;
)");

  require_error(result, "Type cannot be both 'short' and 'long'.");
}

TEST_CASE("Parser rejects illegal modifiers for char") {
  auto result = parse(R"(
long char c;
)");

  require_error(result, "Illegal type modifiers for 'char'.");
}

TEST_CASE("Parser rejects signed char") {
  auto result = parse(R"(
signed char c;
)");

  require_error(result, "'signed char' is not supported yet.");
}

TEST_CASE("Parser rejects illegal modifiers for float") {
  auto result = parse(R"(
unsigned float f;
)");

  require_error(result, "Illegal modifiers for 'float'.");
}

TEST_CASE("Parser rejects illegal modifiers for double") {
  auto result = parse(R"(
unsigned double d;
)");

  require_error(result, "Illegal modifiers for 'double'.");
}

TEST_CASE("Parser rejects long long double") {
  auto result = parse(R"(
long long double d;
)");

  require_error(result, "'long long double' is not a valid type specifier.");
}

TEST_CASE("Parser rejects illegal modifiers for void") {
  auto result = parse(R"(
unsigned void *p;
)");

  require_error(result, "Illegal modifiers for 'void'.");
}

TEST_CASE("Parser rejects illegal modifiers for struct") {
  auto result = parse(R"(
unsigned struct S { int x; } s;
)");

  require_error(result, "Illegal modifiers for 'struct'.");
}

TEST_CASE("Parser rejects illegal modifiers for union") {
  auto result = parse(R"(
unsigned union U { int x; } u;
)");

  require_error(result, "Illegal modifiers for 'union'.");
}

TEST_CASE("Parser rejects illegal modifiers for enum") {
  auto result = parse(R"(
unsigned enum E { A } e;
)");

  require_error(result, "Illegal modifiers for 'enum'.");
}

TEST_CASE("Parser reports expected datatype") {
  auto result = parse(R"(
int main() {
    const;
}
)");

  require_error(result, "Expected a datatype.");
}

TEST_CASE("Parser recovers after invalid local variable initializer") {
  auto result = parse(R"(
int main() {
    int x = ;
    int y = 42;
}
)");

  require_error(result, "Expected a primary expression.");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 2);

  REQUIRE(dynamic_cast<EmptyStmt *>(fn->declaration->body->statements[0].get()));
  auto *decl = dynamic_cast<VariableDeclarationStmt *>(fn->declaration->body->statements[1].get());

  REQUIRE(decl != nullptr);
  REQUIRE(decl->var_name == "y");
}

TEST_CASE("Parser recovers after invalid expression statement") {
  auto result = parse(R"(
int main() {
    x = ;
    y = 10;
}
)");

  require_error(result, "Expected a primary expression.");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 2);
  REQUIRE(dynamic_cast<EmptyStmt *>(fn->declaration->body->statements[0].get()));
  REQUIRE(dynamic_cast<ExpressionStmt *>(fn->declaration->body->statements[1].get()));
}

TEST_CASE("Parser recovers after invalid return statement") {
  auto result = parse(R"(
int main() {
    return +;
    return 5;
}
)");

  require_error(result, "Expected a primary expression.");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->body->statements.size() == 2);
  REQUIRE(dynamic_cast<EmptyStmt *>(fn->declaration->body->statements[0].get()));
  REQUIRE(dynamic_cast<ReturnStmt *>(fn->declaration->body->statements[1].get()));
}

TEST_CASE("Parser recovers after malformed function argument") {
  auto result = parse(R"(
int main() {
    foo(1, );
    bar();
}
)");

  require_error(result, "Expected a primary expression.");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);

  REQUIRE(fn->declaration->body->statements.size() == 2);

  REQUIRE(dynamic_cast<ExpressionStmt *>(fn->declaration->body->statements[0].get()));
  REQUIRE(dynamic_cast<ExpressionStmt *>(fn->declaration->body->statements[1].get()));
}

TEST_CASE("Parser recovers after malformed if condition") {
  auto result = parse(R"(
int main() {
    if (x + ) {
    }

    return 5;
}
)");

  require_error(result, "Expected a primary expression.");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 1);
}

TEST_CASE("Parser recovers after malformed for initializer") {
  auto result = parse(R"(
int main() {
    for (int x = ; ; ) {
    }

    return 5;
}
)");

  require_error(result, "Expected a primary expression.");

  auto *fn = get_function_decl(result);

  REQUIRE(fn != nullptr);
  REQUIRE(fn->declaration->body->statements.size() == 2);

  REQUIRE(dynamic_cast<ForStmt *>(fn->declaration->body->statements[0].get()));
  REQUIRE(dynamic_cast<ReturnStmt *>(fn->declaration->body->statements[1].get()));
}

TEST_CASE("Parser reports missing while condition") {
  auto result = parse(R"(
int main() {
    while () {
    }
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing closing parenthesis after while condition") {
  auto result = parse(R"(
int main() {
    while (x + 1 {
    }
}
)");

  require_error(result, "Expected ')'");
}

TEST_CASE("Parser reports missing do-while condition") {
  auto result = parse(R"(
int main() {
    do {
    } while ();
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing semicolon after do-while") {
  auto result = parse(R"(
int main() {
    do {
    } while (1)
}
)");

  require_error(result, "Expected ';'");
}

TEST_CASE("Parser reports missing switch condition") {
  auto result = parse(R"(
int main() {
    switch () {
    }
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing colon after case label") {
  auto result = parse(R"(
int main() {
    switch (x) {
        case 1
            break;
    }
}
)");

  require_error(result, "Expected ':'");
}

TEST_CASE("Parser reports duplicate default label") {
  auto result = parse(R"(
int main() {
    switch (x) {
        default:
            break;
        default:
            break;
    }
}
)");

  require_error(result, "Multiple 'default' labels are not allowed.");
}

TEST_CASE("Parser reports invalid switch label") {
  auto result = parse(R"(
int main() {
    switch (x) {
        foo:
            break;
    }
}
)");

  require_error(result, "Expected 'case' or 'default' label.");
}

TEST_CASE("Parser reports malformed case expression") {
  auto result = parse(R"(
int main() {
    switch (x) {
        case :
            break;
    }
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports malformed for condition") {
  auto result = parse(R"(
int main() {
    for (;; +) {
    }
}
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing parameter datatype") {
  auto result = parse(R"(
int foo(x) {
}
)");

  require_error(result, "Expected a parameter datatype.");
}

TEST_CASE("Parser reports invalid parameter datatype") {
  auto result = parse(R"(
int foo(int int x) {
}
)");

  require_error(result, "Multiple base types specified.");
}

TEST_CASE("Parser reports missing closing parenthesis in parameter list") {
  auto result = parse(R"(
int foo(int x {
}
)");

  require_error(result, "Expected ')'");
}

TEST_CASE("Parser reports missing comma between parameters") {
  auto result = parse(R"(
int foo(int x int y) {
}
)");

  require_error(result, "Expected ')'");
}

TEST_CASE("Parser reports malformed array parameter") {
  auto result = parse(R"(
int foo(int arr[abc]) {
}
)");

  require_error(result, "Expected an integer array size.");
}

TEST_CASE("Parser reports missing array bracket in parameter") {
  auto result = parse(R"(
int foo(int arr[10) {
}
)");

  require_error(result, "Expected ']'");
}

TEST_CASE("Parser reports missing function body") {
  auto result = parse(R"(
int foo()
)");

  require_error(result, "Unexpected end of file while parsing");
}

TEST_CASE("Parser reports missing opening brace in function body") {
  auto result = parse(R"(
int foo();
int bar()
)");

  require_error(result, "Unexpected end of file while parsing");
}

TEST_CASE("Parser reports malformed function declaration after parameter list") {
  auto result = parse(R"(
int foo(int x) xyz
)");

  require_error(result, "Expected '{'");
}

TEST_CASE("Parser rejects duplicate const qualifier 2") {
  auto result = parse(R"(
const const int x;
)");

  require_error(result, "Duplicate 'const' qualifier.");
}

TEST_CASE("Parser rejects duplicate volatile qualifier 2") {
  auto result = parse(R"(
volatile volatile int x;
)");

  require_error(result, "Duplicate 'volatile' qualifier.");
}

TEST_CASE("Parser rejects duplicate restrict qualifier 2") {
  auto result = parse(R"(
restrict restrict int *p;
)");

  require_error(result, "Duplicate 'restrict' qualifier.");
}

TEST_CASE("Parser rejects duplicate signed specifier 2") {
  auto result = parse(R"(
signed signed int x;
)");

  require_error(result, "Duplicate 'signed' specifier.");
}

TEST_CASE("Parser rejects duplicate unsigned specifier 2") {
  auto result = parse(R"(
unsigned unsigned int x;
)");

  require_error(result, "Duplicate 'unsigned' specifier.");
}

TEST_CASE("Parser rejects duplicate short specifier 2") {
  auto result = parse(R"(
short short int x;
)");

  require_error(result, "Duplicate 'short' specifier.");
}

TEST_CASE("Parser rejects too many long specifiers 2") {
  auto result = parse(R"(
long long long int x;
)");

  require_error(result, "Too many 'long' specifiers.");
}

TEST_CASE("Parser rejects duplicate inline specifier 2") {
  auto result = parse(R"(
inline inline int foo() {
}
)");

  require_error(result, "Duplicate 'inline' specifier.");
}

TEST_CASE("Parser rejects duplicate storage class 2") {
  auto result = parse(R"(
static extern int x;
)");

  require_error(result, "Multiple storage class specifiers are not allowed.");
}

TEST_CASE("Parser rejects multiple base types 2") {
  auto result = parse(R"(
int double x;
)");

  require_error(result, "Multiple base types specified.");
}

TEST_CASE("Parser rejects signed unsigned combination 2") {
  auto result = parse(R"(
signed unsigned int x;
)");

  require_error(result, "Type cannot be both 'signed' and 'unsigned'.");
}

TEST_CASE("Parser rejects short long combination 2") {
  auto result = parse(R"(
short long int x;
)");

  require_error(result, "Type cannot be both 'short' and 'long'.");
}

TEST_CASE("Parser rejects illegal modifiers for char 2") {
  auto result = parse(R"(
long char x;
)");

  require_error(result, "Illegal type modifiers for 'char'.");
}

TEST_CASE("Parser rejects signed char 2") {
  auto result = parse(R"(
signed char x;
)");

  require_error(result, "'signed char' is not supported yet.");
}

TEST_CASE("Parser rejects illegal modifiers for float 2") {
  auto result = parse(R"(
unsigned float x;
)");

  require_error(result, "Illegal modifiers for 'float'.");
}

TEST_CASE("Parser rejects illegal modifiers for double 2") {
  auto result = parse(R"(
short double x;
)");

  require_error(result, "Illegal modifiers for 'double'.");
}

TEST_CASE("Parser rejects long long double 2") {
  auto result = parse(R"(
long long double x;
)");

  require_error(result, "'long long double' is not a valid type specifier.");
}

TEST_CASE("Parser rejects illegal modifiers for void 2") {
  auto result = parse(R"(
unsigned void *p;
)");

  require_error(result, "Illegal modifiers for 'void'.");
}

TEST_CASE("Parser rejects illegal modifiers for struct 2") {
  auto result = parse(R"(
unsigned struct S {
    int x;
} s;
)");

  require_error(result, "Illegal modifiers for 'struct'.");
}

TEST_CASE("Parser rejects illegal modifiers for union 2") {
  auto result = parse(R"(
short union U {
    int x;
} u;
)");

  require_error(result, "Illegal modifiers for 'union'.");
}

TEST_CASE("Parser rejects illegal modifiers for enum 2") {
  auto result = parse(R"(
long enum E {
    A
} e;
)");

  require_error(result, "Illegal modifiers for 'enum'.");
}

TEST_CASE("Parser reports missing datatype 2") {
  auto result = parse(R"(
const x;
)");

  require_error(result, "Expected a datatype.");
}

TEST_CASE("Parser reports missing datatype after storage class 2") {
  auto result = parse(R"(
static x;
)");

  require_error(result, "Expected a datatype.");
}

TEST_CASE("Parser rejects typedef with initializer") {
  auto result = parse(R"(
typedef int MyInt = 5;
)");

  require_error(result, "A typedef declaration cannot have an initializer.");
}

TEST_CASE("Parser reports invalid array size") {
  auto result = parse(R"(
int arr[foo];
)");

  require_error(result, "Expected an integer array size.");
}

TEST_CASE("Parser reports missing initializer after equals") {
  auto result = parse(R"(
int x = ;
)");

  require_error(result, "Expected an initializer expression after '='.");
}

TEST_CASE("Parser reports missing variable initializer expression") {
  auto result = parse(R"(
int x = +;
)");

  require_error(result, "Expected a primary expression.");
}

TEST_CASE("Parser reports missing variable semicolon") {
  auto result = parse(R"(
int x = 5
)");

  require_error(result, "Unexpected end of file while parsing");
}

TEST_CASE("Parser reports missing break semicolon") {
  auto result = parse(R"(
int main() {
    break
}
)");

  require_error(result, "Expected ';'");
}

TEST_CASE("Parser reports missing continue semicolon") {
  auto result = parse(R"(
int main() {
    continue
}
)");

  require_error(result, "Expected ';'");
}