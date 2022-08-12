#pragma once
#include <ctype.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

char *inputString(FILE *fp, size_t size, int skip_spaces, int *retsize);
