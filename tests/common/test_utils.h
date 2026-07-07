#pragma once

#include <string_view>
#include <vector>

#include "lexer/lexer.h"

std::vector<Token> tokenize(std::string_view source);
