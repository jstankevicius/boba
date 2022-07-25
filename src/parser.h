#pragma once

#include <deque>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "token.h"

std::unique_ptr<AST>
parse_expr(std::deque<std::shared_ptr<Token>>& tokens);
