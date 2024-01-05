#include "parse.h"
#include <math.h>

void parse_set_error (Cell* cc, uint16_t which, ...)
{
    va_list args;
    va_start(args, which);

    static const char* errfmts[] = {
        "maximum token capacity reached; max = %d Tokens.",
    };

    snprintf(cc->as.text, 8, "!#ERROR");
    vsnprintf(cc->as_error, DEBUT_CELL_ERROR_LEN, errfmts[which], args); /* @TODO: REPLACE THIS FOR A STACK */
    cc->type = cell_is_errr;
    va_end(args);
}

