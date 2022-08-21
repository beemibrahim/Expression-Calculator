#include "utilites.h"

/* These macros are used to indentify the different kind of operators a user can
 * do */
#define ADD 0xDEA

#define SUBTRACT 0xDEB

#define MULTILPLY 0xDEC

#define DIVIDE 0xDED


#define VALUE_START_MALLOC 100
#define VALUE_ADD_MALLOC 100

#define DECIMAL_VALUE_START_MALLOC 100
#define DECIMAL_VALUE_ADD_MALLOC 100

#define ANY_START_MALLOC 100
#define ANY_ADD_MALLOC 100

#define MAX_NUMBER_SIZE 500
#define MAX_DEC_NUMBER_SIZE 50

#define EXPRESS_OP_START_SIZE 100
#define EXPRESS_OP_ADD_SIZE 100

struct off_st {
    int offset;
    int value;
};

/* This structure contains the number implementation, it represents a number */
struct number {
  char *malloced_number_block;         /* The malloced number's block */
  int malloced_number_start_offst;         /* This is the start of the number inside the block */

  unsigned int number_block_size;      /* The malloced number's size */
//  unsigned int number_size;       /* The size that the number uses of the block size */

  int is_decimal;            /* If the number is a decimal number */
  int is_number_dig;            /* Does this number have any non-decimal digit */

  char *malloced_decimal_number_block; /* The malloced decimal part of the number */
  int malloced_decimal_number_start_offst;  /* This is the start of the decimal number inside the block */

  unsigned int decimal_number_block_size; /* The decimal number size */
//  unsigned int decimal_number_size; /* The size that the decimal number uses of the decimal number block size */

  unsigned int last_non_zero_decimal_place_pos; /* the position of the last non-zero decimal number of the decimal number block size */

  int is_negative;              /* If the number is a negative number*/
};

/* This structure defined an operator */
struct op {
  int op; /* This variable one of the 4 macros above */
};

/* This strutcure defines anything ( an operator or a number )*/
struct any {
  int is_number;   /* This number is a boolean that contains whether the current
                      thing is an operator */
  int is_operator; /* This number is a boolean that contains whether the current
                      thing is an operator */

  struct number *the_number; /* This contains the pointer to the number
                                structure if there is one */
  struct op *the_op; /* This contains the pointer to the operator structure if
                        there is one*/

  int is_start_new_exec; /* This is a boolean whether or not the executer should
                            start a new execution*/
  int is_end_new_exec;   /* This is a boolean whether or not the executer should
                            end the current execution and go back to the previous
                            execution is there is once*/
};

// This structure contains the expression;
struct expression {
  struct any *has_retrevied_result;
  int any_write_offset;
  int any_size;
};


struct last_value_buffer {
    struct off_st* buffer;
    int buffer_size;
    int buffer_offset;
};

struct number *copy_number(struct number *, int*);

void reuse_number(struct number**);

void free_number(struct number *);

void free_last_value_buff(struct last_value_buffer*);

int create_number(char* num, int size, struct number* result);
