#include "calculator_methods.h"

#define NULL_ERROR "null values detected"


int add_number(struct number* number1, struct number* number2, struct number* result, char** error_message) {


	/* Get the size of the first number */
	int number1_nsize = strlen(number1->malloced_number);
	int number1_decsize = 0;

	/* Get the size of the second number */
	int number2_nsize = strlen(number2->malloced_number);
	int number2_decsize = 0;


	if (number1->is_decimal != 0) {
		number1_decsize = strlen(number1->malloced_decimal_number);
		result->is_decimal = 1;
	}

	if (number2->is_decimal != 0) {
		number2_decsize = strlen(number2->malloced_decimal_number);
		result->is_decimal = 1;
	}

	/* The carry to be applied to the next number */
	char carry = 0;

	int dec_number_size = 0;

	char* main_number_ = NULL;
	int main_number_size = 0;

	char* slav_number_ = NULL;
	int slav_number_size = 0;

	int malloced_write_pos = strlen(result->malloced_number) - 1;

	/* If one of the numbers are a decimal number then add the decimal numbers up */
	if (result->is_decimal == 1) {
		int malloced_dec_write_pos = strlen(result->malloced_decimal_number) - 1;
		/* Add trailing zeros */
		if (number2_decsize > number1_decsize) {

			number1->decimal_number_size += number2_decsize - number1_decsize + 1;
			number1->malloced_decimal_number = realloc(number1->malloced_decimal_number, number1->decimal_number_size);

			memset(number1->malloced_decimal_number + number1_decsize,'0', number2_decsize - number1_decsize);
			number1->is_decimal = 1;

			*(number1->malloced_decimal_number + number2_decsize) = '\0';

		     
			dec_number_size = number2_decsize;
			number1_decsize = number2_decsize;
		}

		/* Add trailing zeros */
		else if (number1_decsize > number2_decsize) {
			
			number2->decimal_number_size += number1_decsize - number2_decsize + 1;
			number2->malloced_decimal_number = realloc(number2->malloced_decimal_number, number2->decimal_number_size);

			memset(number2->malloced_decimal_number + number2_decsize, '0', number1_decsize - number2_decsize);
			number2->is_decimal = 1;

			*(number2->malloced_decimal_number + number1_decsize) = '\0';
			
			dec_number_size = number1_decsize;
			number2_decsize = number1_decsize;
		}


		/* Add loop for the decimal number */
		for (int index = dec_number_size - 1;;--index) {

			char current_number_one = (*(number1->malloced_decimal_number + index) - 48);
			char current_number_two = (*(number2->malloced_decimal_number + index) - 48);
			
			char result_add = current_number_one + current_number_two + carry;

			carry = 0;

			char add_carry = (result_add / 10);

			result_add -= (add_carry * 10);

			carry += add_carry;

			result_add += 48;

			*(result->malloced_decimal_number + malloced_dec_write_pos) = result_add;
			malloced_dec_write_pos--;

			if (index == 0) {
				break;
			}

		}

	}

	char* malloced_leading_zeros = NULL;
	int leading_zeros_num = 0;
	int was_leading_zeros_malloced = 0;

	int in_leading_zeros = 0;

	/* If there should be leading zeros on number2 */
	if (number1_nsize > number2_nsize) {

		malloced_leading_zeros = calloc(number1_nsize - number2_nsize, sizeof(char));
		leading_zeros_num = number1_nsize - number2_nsize;

		was_leading_zeros_malloced = 1;

		main_number_ = number1->malloced_number;
		main_number_size = number1_nsize;

		slav_number_ = number2->malloced_number;
		slav_number_size = number2_nsize;

	}

	/* If there should be leading zeros on number1 */
	else if (number2_nsize > number1_nsize) {
		
		malloced_leading_zeros = calloc(number2_nsize - number1_nsize, sizeof(char));
		leading_zeros_num = number2_nsize - number1_nsize;

		was_leading_zeros_malloced = 1;

		main_number_ = number2->malloced_number;
		main_number_size = number2_nsize;

		slav_number_ = number1->malloced_number;
		slav_number_size = number1_nsize;

	}
	else {

		main_number_ = number2->malloced_number;
		main_number_size = number2_nsize;

		slav_number_ = number1->malloced_number;
		slav_number_size = number1_nsize;

	}

	/* Number add loop */

	for (int index = main_number_size - 1;;--index) {

		char current_number_one = (*(main_number_ + index) - 48);
		char current_number_two = (*(slav_number_ + index - leading_zeros_num) - 48);

		char result_add = current_number_one + current_number_two + carry;

		carry = 0;

		char add_carry = (result_add / 10);

		result_add -= (add_carry * 10);

		carry += add_carry;

		result_add += 48;

		if (index == leading_zeros_num && was_leading_zeros_malloced == 1) {
			in_leading_zeros = 1;

			*(result->malloced_number + malloced_write_pos) = result_add;
			malloced_write_pos--;

			break;
		}

		/* If we are on the last index */
		if (index == 0) {


			if (add_carry * 10 != 0) {

				*(result->malloced_number + malloced_write_pos) = result_add;
				*(result->malloced_number + malloced_write_pos - 1) = (add_carry * 10);
				malloced_write_pos -= 2;

			}
			else {
				*(result->malloced_number + malloced_write_pos) = result_add;
				malloced_write_pos -= 1;
			}
			


			break;
		}

		*(result->malloced_number + malloced_write_pos) = result_add;
		malloced_write_pos--;
	}

	if (in_leading_zeros == 1) {

		for (int zeros_index = leading_zeros_num - 1;;--zeros_index) {
			
			char current_number_one = (*(main_number_ + zeros_index) - 48);
			char current_number_two = 0;

			char result_add = current_number_one + current_number_two + carry;

			carry = 0;

			char add_carry = (result_add / 10);

			result_add -= (add_carry * 10);

			carry += add_carry;

			result_add += 48;

			if (zeros_index == 0) {


				if (add_carry * 10 != 0) {

					*(result->malloced_number + malloced_write_pos) = result_add;
					*(result->malloced_number + malloced_write_pos - 1) = (add_carry * 10);
					malloced_write_pos -= 2;

				}
				else {
					*(result->malloced_number + malloced_write_pos) = result_add;
					malloced_write_pos -= 1;
				}
			
				break;
			}
			else {
				*(result->malloced_number + malloced_write_pos) = result_add;
				malloced_write_pos -= 1;
			}

		}
	}

	/* Add leading zeros if needed */
	if ((malloced_write_pos + 1) != 0) {
		memset(result->malloced_number, '0', malloced_write_pos + 1);
	}

	/* Free malloced memory if it was allocated */
	if (was_leading_zeros_malloced == 1) {
		free(malloced_leading_zeros);
	}

}