#pragma once
#include "token.h"
#include <memory>

void err_token(std::shared_ptr<Token> token, std::string message);
void err_line(int line_num, std::string message);