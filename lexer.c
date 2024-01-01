#include "lexer.h"

/* this function will split all the cells
 * into separated ones to then lexer them
 * one by one.
 *
 * note: this function is only used when
 * -p is used.
 * */
void lexer_cell_by_cell (Spread* spread)
{
    const size_t len = spread->src.len;
    bool within_str  = false;

    RowOffsets* offsts = &spread->offsets;
    uint16_t nchrs  = 0;

    for (size_t i = 0; i < len; i++) {
        const char a = spread->src.src[i];

        if (a == 10) {
            offsts->marks[++offsts->nth_mark] = offsts->nth_cell;
            printf("offset at: %d\n", offsts->nth_cell);
            continue;
        }
        if (a == '|' && !within_str) {
            char* pos = spread->src.src + i - nchrs;
            printf("content of %dth cell: <%*.*s>\n", offsts->nth_cell, nchrs, nchrs, pos);
            offsts->nth_cell++;
            nchrs = 0;
            continue;
        }

        if (a == '"')
            within_str = !within_str;
        nchrs++;
    }
}
