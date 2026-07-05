#pragma once

#include "../types/ParsedTypes.h"
#include "exprs.h"
#include <vector>

struct StructField {
  ParsedType type;
  std::string name;
};

struct StructDecl {
  std::string name;
  std::vector<StructField> fields;
  bool is_definition = true;

  StructDecl(std::string name_, std::vector<StructField> fields_) : name{std::move(name_)}, fields{std::move(fields_)} {
  }

  void show_struct() const {
    std::cout << "StructDecl\n";
    std::cout << "  Name\n";
    if (name.empty()) {
      std::cout << "    <anonymous>\n";
    } else {
      std::cout << "    " << name << '\n';
    }
    std::cout << "  Fields\n";
    for (const auto &field : fields) {
      std::cout << "    Field(" << field.type.to_string() << " " << field.name << ")\n";
    }
  }
};

struct UnionField {
  ParsedType type;
  std::string name;
};

struct UnionDecl {
  std::string name;
  std::vector<UnionField> fields;
  bool is_definition = true;

  UnionDecl(std::string name_, std::vector<UnionField> fields_) : name{std::move(name_)}, fields{std::move(fields_)} {
  }

  void show_union() const {
    std::cout << "UnionDecl\n";
    std::cout << "  Name\n";
    if (name.empty()) {
      std::cout << "    <anonymous>\n";
    } else {
      std::cout << "    " << name << '\n';
    }
    std::cout << "  Fields\n";
    for (const auto &field : fields) {
      std::cout << "    Field(" << field.type.to_string() << " " << field.name << ")\n";
    }
  }
};

struct EnumMember {
  std::string name;
  std::unique_ptr<Expr> value;
};

struct EnumDecl {
  std::string name;
  std::vector<EnumMember> members;
  bool is_definition = true;

  EnumDecl(std::string n, std::vector<EnumMember> m) : name(std::move(n)), members(std::move(m)) {
  }

  inline void print_indent(int indent) const {
    for (int i = 0; i < indent; i++) {
      std::cout << "  ";
    }
  }

  void show_enum(int indent = 0) const {
    print_indent(indent);
    std::cout << "Enum: \n";
    if (name.empty()) {
      std::cout << "    <anonymous>\n";
    } else {
      std::cout << "    " << name << '\n';
    }

    for (const auto &member : members) {
      print_indent(indent + 1);
      std::cout << "  " << member.name;

      if (member.value) {
        std::cout << '\n';
        member.value->show_expr(indent + 2);
      }
    }
  }
};