#include "utilites.h"

int inputString(FILE* fp, size_t size, int skip_spaces, int* retsize, char** retstring) {

	*retsize = 0;

	if (size == 0) {
		return 1;
	}

	if (fp == NULL) {
		return 1;
	}
	/* The size is extended by the input with the value of the provisional */
	char* str;

	/* If we have encountered a large block of spaces */
	int if_space = 0;

	/* The last character we found */
	char last_ch = '\0';

	/* Current length of expression */
	unsigned long len = 0;

	/* Initalize the buffer */
	str = realloc(NULL, size); // size is start size

	/* if failed to initalize */
	if (!str)
		return 1;

	/* Loop over the stdin input */
	char ch = '\0';
	while (EOF != (ch = fgetc(fp)) && ch != '\n') {

		if (skip_spaces == 1) {

			if (ch == ' ' && last_ch == ' ') {
				if_space = 1;

			}
			else {
				if_space = 0;
			}

			if (if_space == 1) {
				continue;
			}
		}

		str[len++] = ch;
		if (len == size) {
			str = realloc(str, sizeof(*str) * (size += 16));
			if (!str)
				return 1;
		}

		/* Record the last character we found */
		last_ch = ch;
		++(*(retsize));
	}
	str[len++] = '\0';

	*retstring = realloc(str, sizeof(*str) * len);

	return 0;
}

int isC_alpha(int _C) {

	if (isalpha(_C) != 0 || _C == '_') {
		return 0;
	}
	else {
		return 1;
	}
}
