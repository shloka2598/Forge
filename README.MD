# Forge

Forge is a C compiler written from scratch in C++20. I created this project with the goal of understanding every stage of the compilation pipeline, from lexical analysis to machine code generation.

## Current Status

The frontend of the compiler supports a substantial subset of features of the C language and includes lexical analysis, parsing, semantic analysis, symbol resolution, and a fully typed abstract syntax tree (AST). The next stage of development focuses on AST optimizations followed by intermediate representation (IR) generation and backend implementation.

## Features

### Frontend

- Lexical analysis
- Recursive descent parser
- Abstract Syntax Tree (AST)
- Typed AST
- Symbol table
- Semantic analysis
- Type checking
- Scope management

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
- Semantic analysis
- Type system
- Symbol tables
- Typed AST
- Basic Optimizations

### In Progress

- Constant folding
- Dead code elimination
- AST simplification

## License

This project is licensed under the MIT License.
