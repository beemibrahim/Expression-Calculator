#pragma once
#include "expression.h"
#include "errors.h"

int ParseExpression(char* expression, int expression_size, struct expression* result,char** read_only_readable_error);

