#include "expression.h"

struct number* copy_number(struct number* old_number, int* have_encv) {

	/* Create a copy of the current number object */

	struct number* adding_number = malloc(sizeof(struct number));
	memset(adding_number, 0, sizeof(struct number));

		adding_number->malloced_number = malloc(old_number->number_size);
		memcpy(adding_number->malloced_number, old_number->malloced_number,
			old_number->number_size);
		adding_number->number_size = old_number->number_size;


		adding_number->is_negative = old_number->is_negative;
	
	adding_number->is_decimal = old_number->is_decimal;


	if (have_encv == NULL) {

		if (adding_number->is_decimal == 1) {
			adding_number->decimal_number_size = old_number->decimal_number_size;
			adding_number->malloced_decimal_number =
				malloc(old_number->decimal_number_size);

			memcpy(adding_number->malloced_decimal_number,
				old_number->malloced_decimal_number,
				old_number->decimal_number_size);
		}

	}
	else {

		if (adding_number->is_decimal == 1 && *have_encv == 1) {
			adding_number->decimal_number_size = old_number->decimal_number_size;
			adding_number->malloced_decimal_number =
				malloc(old_number->decimal_number_size);

			memcpy(adding_number->malloced_decimal_number,
				old_number->malloced_decimal_number,
				old_number->decimal_number_size);
		}
		else {
			adding_number->is_decimal = 0;
		}

	}

	return adding_number;
}

void reuse_number(struct number** result)
{
	(*(result))->is_decimal = 0;
	(*(result))->is_negative = 0;
	(*(result))->is_number_dig = 0;
	*(*(result))->malloced_decimal_number = '\0';
	*(*(result))->malloced_number = '\0';
}

void free_number(struct number* number) {

	/* Free */
	free(number->malloced_decimal_number);
	free(number->malloced_number);
	free(number);

	return;
}

void free_last_value_buff(struct last_value_buffer* ptr)
{
	free(ptr->buffer);
	free(ptr);
}

int create_number(char* num, int size, struct number* result)
{
	
	int FoundNegativeSign = 0;
	int FoundDecimalSign = 0;
	int InValue = 1;
	int InDecimalValue = 0;

	int result_write_pos = 0;
	int result_dec_write_pos = 0;

	int size_ = 0;

	for (char* loopnum = num; size_ < size; loopnum++,size_++) {

		if (*loopnum == '-') {

			if (FoundNegativeSign == 1) {
				return 1;
			}

			FoundNegativeSign = 1;
			result->is_negative = 1;

			continue;
		}

		if (*loopnum == '.') {

			if (FoundDecimalSign == 1) {
				return 1;
			}

			FoundDecimalSign = 1;
			result->is_decimal = 1;
			InValue = 0;
			InDecimalValue = 1;


			continue;
		}

		if (InValue == 1) {

			if (!isdigit(*loopnum)) {
				return -1;
			}

			*(result->malloced_number + result_write_pos) = *loopnum;
			result_write_pos++;

			continue;
		}

		else if (InDecimalValue == 1) {

			if (!isdigit(*loopnum)) {
				return -1;
			}

			*(result->malloced_decimal_number + result_dec_write_pos) = *loopnum;
			result_dec_write_pos++;

			continue;
		}


	}

}

