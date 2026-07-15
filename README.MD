# Forge

Forge is a C compiler written from scratch in C++20. I created this project with the goal of understanding every stage of the compilation pipeline, from lexical analysis to machine code generation.

## Current Status

Forge currently implements a complete frontend for a substantial subset of the C language, including lexical analysis, parsing, semantic analysis, type checking, symbol resolution, and a fully typed Abstract Syntax Tree (AST).

The compiler also features source-aware diagnostics with precise error reporting and extensive automated testing.

Development is currently focused on expanding the optimization pipeline, followed by intermediate representation (IR) generation and backend code generation.

## Features

### Frontend

- Lexical analysis
- Recursive descent parser
- Error recovery
- Source-aware diagnostics
- Abstract Syntax Tree (AST)
- Typed AST
- Symbol table
- Semantic analysis
- Type checking
- Scope management

### Optimization

- AST optimization framework

### Supported Language Features

- Primitive C types
- Pointers
- Arrays and multidimensional arrays
- Functions and function prototypes
- Variable declarations
- Type qualifiers and storage classes
- Structures
- Unions
- Enumerations
- Typedefs
- Anonymous structs, unions, and enums
- Forward declarations
- Recursive and mutually recursive structures
- Member access (`.` and `->`)
- Array indexing
- Function calls
- Type casts
- `sizeof`
- `if` / `else`
- `while`
- `do-while`
- `for`
- `switch`
- `break`
- `continue`
- `return`
- Assignment expressions
- Unary operators
- Binary operators
- Conditional (`?:`) operator
- Comma operator
- Implicit arithmetic conversions

## Testing

Forge uses Catch2 for automated testing.

Current test suite includes:

- 910 test cases
- 8392 assertions

Run the test suite with:

```bash
ctest
```

or

```bash
./build/tests/forge_tests
```

## Building

### Requirements

- CMake 3.20 or newer
- A C++20 compatible compiler

### Build

```bash
mkdir build
cd build

cmake ..
cmake --build .
```

The compiler executable will be generated as:

```text
forge
```

### Usage

```bash
./forge <source-file.c>
```

## Roadmap

### Completed

- Lexer
- Parser
- AST construction
- Typed AST
- Symbol tables
- Semantic analysis
- Type system
- Source-aware diagnostics
- Error recovery
- Basic AST optimization framework

### In Progress

- Constant folding
- Dead code elimination
- AST simplification

## License

This project is licensed under the MIT License.
