#pragma once

#include "../types/Types.h"
#include <string>
#include <unordered_map>

enum class SymbolKind {
  VARIABLE,
  PARAMETER,
  FUNCTION,
  STRUCT,
  UNION,
  ENUM,
  ENUM_CONSTANT,
  TYPEDEF
};

struct Symbol {
  SymbolKind kind;
  std::string name;
  Type *type;
  bool is_defined = false;
  const void *decl_node = nullptr;
  std::optional<int64_t> enum_value;
};

class Scope {
private:
  std::unordered_map<std::string, Symbol> ordinary_symbols;
  std::unordered_map<std::string, Symbol> tag_symbols; // struct, enum and union tags

  Scope *parent;

public:
  explicit Scope(Scope *_parent = nullptr) : parent{_parent} {
  }

  bool insert(const Symbol &symbol);

  Symbol *insert_or_get_tag(SymbolKind kind, const std::string &name, Type *type);

  Symbol *lookup_local_identifier(const std::string &);
  Symbol *lookup_identifier(const std::string &);

  Symbol *lookup_local_tag(const std::string &name);
  Symbol *lookup_tag(const std::string &name);

  Scope *get_parent() const {
    return parent;
  }
};