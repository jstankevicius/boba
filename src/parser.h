#pragma once

#include <deque>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "environment.h"
#include "token.h"

std::shared_ptr<ConsCell> parse_expr(std::deque<std::shared_ptr<Token>>& tokens);
