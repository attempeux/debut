#ifndef _PARSE_H
#define _PARSE_H
#include "debut.h"
#include <stdio.h>              /* REMOVE WHEN NEEDED. */
#include <stdarg.h>

#define SET_ERROR_MAX_CAPACITY      0
#define SET_ERROR_UNKNOWN_SYM       1
#define SET_ERROR_MISSING_LEFT_PAR  2

void parse_set_error (Cell*, uint16_t, ...);
void parse_eval_expr (const Spread*, Cell*);
void update_ou_solve (Cell*);

#endif
