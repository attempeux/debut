#ifndef _PARSE_H
#define _PARSE_H
#include "debut.h"
#include <stdio.h>              /* REMOVE WHEN NEEDED. */
#include <stdarg.h>

#define DEBUT_ERR_MAX_CAP_REACHED   0
#define DEBUT_BAD_USE_OF_WORD_TYPE  1

#define IS_IT_CONSTANT(a)       ((a == token_is_numb) || (a == token_is_word))
#define IS_IT_LIT_TOKEN(a)      (IS_IT_CONSTANT(a)    || (a == token_is_refc))
#define IS_IT_FORMULA(a)        ((a == token_is_dolr) || (a == token_is_qust))
#define IS_IT_OPERATOR(a)       ((a >= token_is_mins) || (a == token_is_modd))

void parse_set_error (Cell*, uint16_t, ...);
void parse_solve_cell (Cell*);

#endif
