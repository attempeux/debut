#ifndef _PARSE_H
#define _PARSE_H
#include "debut.h"
#include <stdio.h>              /* REMOVE WHEN NEEDED. */
#include <stdarg.h>

#define DEBUT_ERR_MAX_CAP_REACHED   0

void parse_set_error (Cell*, uint16_t, ...);

#endif
