#include "debut.h"
#include <assert.h>
#include <string.h>

static TokenKind figure_out_kind (const char);
static uint16_t get_string_literal (const char*, uint16_t*, const uint16_t);

static long double get_number_literal (const char*, uint16_t*);
static void* get_reference (const Spread*, const char*, uint16_t*);

void lexer_lex (Spread* spread, Cell* cuC)
{
    const uint16_t len = cuC->fxch;
    if (!len) {
        cuC->as.number = 0;
        cuC->kind = cell_kind_empty;
        return;
    }

    for (uint16_t i = 0; i < len; i++) {
        const char a = cuC->fx_txt[i];
        if (isspace(a)) continue;

        Token token = { .kind = figure_out_kind(a) };

        switch (token.kind) {
            case token_kind_number:
                token.as.number = get_number_literal(cuC->fx_txt, &i);
                printf("Number: <%Lf>\n", token.as.number);
                break;

            case token_kind_string:
                token.as.string  = cuC->fx_txt + i + 1;
                token.len_as_str = get_string_literal(cuC->fx_txt, &i, len);
                printf("String: <%.*s>\n", token.len_as_str, token.as.string);
                break;

            case token_kind_reference:
                get_reference(spread, cuC->fx_txt, &i);
                break;
        }


    }
}

static TokenKind figure_out_kind (const char a)
{
    switch (a) {
        case '"': case '&': case '(': case ')':
        case '@': case '+': case '-': case '*':
        case '/': return a;
    }

    return isdigit(a) ? token_kind_number : token_kind_unknown;
}

static uint16_t get_string_literal (const char* src, uint16_t* pos, const uint16_t len)
{
    uint16_t token_length = 0;
    char ch;

    do {
        token_length++;
        *pos += 1;
        ch = src[*pos];

        /* "This is an \"string\" type"...
         *             ~       ~
         *         '\' counts as character within the string
         *         but gotta skip the string character since
         *         if we dont it will finish and it will incomplete.
         * */
        if (ch == '\\' && ((*pos + 1) < len) && src[*pos + 1] == '"') {
            *pos += 1;
            token_length++;
        }
    } while (ch != '"' && *pos < len);

    if (ch != '"') {} // TODO: add error to the stack.
    return --token_length;
}

static long double get_number_literal (const char* src, uint16_t* pos)
{
    char* init = (char*) &src[*pos], *final;
    long double a = strtold(src + *pos, &final);

    *pos += (uint16_t) (final - init);
    return a;
}

static void* get_reference (const Spread* spread, const char* src, uint16_t* pos)
{
    *pos += 1;
    uint16_t Col = 0, Row = 0, Hilfsvariable = 0;
    uint16_t colseg = (uint16_t) strcspn(src + *pos, "1234567890 ");

    /* TODO: Add error to stack. */
    if (!colseg) {
        puts("no column name");
        exit(1);
    }

    for (uint16_t i = 0; i < colseg; i++) {
        Col += ((Hilfsvariable++) * 26) + (toupper(src[*pos]) - 'A');
        *pos += 1;
    } 

    /* TODO: Add error to stack. */
    if (!isdigit(src[*pos])) {
        puts("no row number");
        exit(1);
    }

    Row = atoi(src + *pos);
    while (isdigit(src[*pos])) *pos += 1;

    /* TODO: Add error to stack.
     * @note: not available while testing:
     * if (Row >= spread->winf.nRows || Col >= spread->winf.nCols) {
     * puts("cell outta bounds");
     * exit(1);
     * }
     * */

    /* @note: not available while testing: &spread->cells[Row * spread->winf.nCols + Col]; */
    printf("reference: (%d, %d)\n", Row, Col);
    return NULL; 
}


#define TEXT "&A4 34"

int main ()
{
    Cell c;
    memcpy(&c.fx_txt, &TEXT, strlen(TEXT));
    c.fxch = strlen(TEXT);

    lexer_lex(NULL, &c);
    return 0;
}

