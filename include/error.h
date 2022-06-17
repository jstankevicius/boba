#pragma once
#include <token.h>

void err_token(Token* token, std::string message);
void err_line(int line_num, std::string message);