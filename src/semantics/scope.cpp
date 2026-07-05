#include "scope.h"

bool Scope::insert(const Symbol &symbol) {
  switch (symbol.kind) {
  case SymbolKind::STRUCT:
  case SymbolKind::UNION:
  case SymbolKind::ENUM:
    if (lookup_local_tag(symbol.name)) {
      return false;
    }

    tag_symbols[symbol.name] = symbol;
    return true;
  case SymbolKind::VARIABLE:
  case SymbolKind::PARAMETER:
  case SymbolKind::FUNCTION:
  case SymbolKind::ENUM_CONSTANT:
  case SymbolKind::TYPEDEF:
    if (lookup_local_identifier(symbol.name)) {
      return false;
    }
    ordinary_symbols[symbol.name] = symbol;
    return true;
  }

  return false;
}

Symbol *Scope::insert_or_get_tag(SymbolKind kind, const std::string &name, Type *type) {

  if (name.empty()) {
    Symbol symbol;
    symbol.kind = kind;
    symbol.name = "";
    symbol.type = type;
    symbol.is_defined = false;
    symbol.decl_node = nullptr;
    symbol.enum_value = std::nullopt;

    auto key = "__anonymous_tag_" + std::to_string(tag_symbols.size());

    auto [it, success] = tag_symbols.emplace(std::move(key), std::move(symbol));

    return &it->second;
  }

  auto it = tag_symbols.find(name);

  if (it != tag_symbols.end()) {
    return &it->second;
  }

  Symbol symbol;
  symbol.kind = kind;
  symbol.name = name;
  symbol.type = type;
  symbol.is_defined = false;
  symbol.decl_node = nullptr;
  symbol.enum_value = std::nullopt;

  auto [inserted, success] = tag_symbols.emplace(name, std::move(symbol));

  return &inserted->second;
}

Symbol *Scope::lookup_local_identifier(const std::string &name) {
  auto it = ordinary_symbols.find(name);

  if (it == ordinary_symbols.end()) {
    return nullptr;
  }

  return &it->second;
}

Symbol *Scope::lookup_identifier(const std::string &name) {
  Scope *current = this;
  while (current) {
    auto it = current->ordinary_symbols.find(name);
    if (it != current->ordinary_symbols.end()) {
      return &it->second;
    }
    current = current->parent;
  }
  return nullptr;
}

Symbol *Scope::lookup_local_tag(const std::string &name) {
  auto it = tag_symbols.find(name);
  if (it == tag_symbols.end()) {
    return nullptr;
  }
  return &it->second;
}

Symbol *Scope::lookup_tag(const std::string &name) {
  Scope *current = this;
  while (current) {
    auto it = current->tag_symbols.find(name);
    if (it != current->tag_symbols.end()) {
      return &it->second;
    }
    current = current->parent;
  }

  return nullptr;
}