#include "utilites.h"

/* These macros are used to indentify the different kind of operators a user can
 * do */
#define ADD 0xDEA

#define SUBTRACT 0xDEB

#define MULTILPLY 0xDEC

#define DIVIDE 0xDED

/* This structure contains the number implementation, it represents a number */
struct number {
  char *malloced_number;         /* The malloced number */
  unsigned int number_size;      /* The malloced number's size */
  int is_decimal;            /* If the number is a decimal number */
  int is_number_dig;            /* Does this number have any non-decimal digit*/
  char *malloced_decimal_number; /* The malloced decimal part of the number */
  unsigned int decimal_number_size; /* The decimal number size */
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

struct number *copy_number(struct number *, int* );

void reuse_number(struct number**);

void free_number(struct number *);
