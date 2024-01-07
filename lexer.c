#include "debut.h"
#include <assert.h>
#include <string.h>

static TokenKind figure_out_kind (const char);
static uint16_t get_string_literal (const char*, uint16_t*, const uint16_t);

static long double get_number_literal (const char*, uint16_t*);
static void* get_reference (const Spread*, const char*, uint16_t*);

static TokenKind get_function_kind (const char*, const uint16_t, uint16_t*);

void lexer_lex (Spread* spread, Cell* cuC)
{
    const uint16_t len = cuC->fxch;
    if (!len) {
        cuC->as.number = 0;
        cuC->kind = cell_kind_empty;
        return;
    }

    const char* fxs[] = {"sin", "cos", "asin", "acos", "atan", "sqrt", "pi", "e"};

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
                token.as.reference = (Cell*) get_reference(spread, cuC->fx_txt, &i);
                break;

            case token_kind_function:
                token.kind = get_function_kind(cuC->fx_txt + i, len, &i);
                (token.kind) ? printf("function of: %s\n",  fxs[token.kind - 2]) : printf("uknown function\n");
                break;

            default: break;
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


    /* TODO: Add error to stack. */
    if (ch != '"') {
        puts("no terminated string");
        exit(1);
    }

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
    /* Do not forget skip & character.
     * */
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

typedef struct MathFxs {
    char* name;
    uint16_t len;
    TokenKind kind;
} MathFxs;

static TokenKind get_function_kind (const char* src, const uint16_t maxlen, uint16_t *pos)
{
    static const uint16_t nfxs = 8;

    static const MathFxs fxs[] = {
        { .name = "@ASin", .len = 5, .kind = token_kind_asin},
        { .name = "@ACos", .len = 5, .kind = token_kind_acos},
        { .name = "@ATan", .len = 5, .kind = token_kind_atan},
        { .name = "@Sqrt", .len = 5, .kind = token_kind_sqrt},
        { .name = "@Sin",  .len = 4, .kind = token_kind_sin},
        { .name = "@Cos",  .len = 4, .kind = token_kind_cos},
        { .name = "@Pi",   .len = 3, .kind = token_kind_pi},
        { .name = "@E",    .len = 2, .kind = token_kind_e}
    };

    for (uint16_t i = 0; i < nfxs; i++) {
        const MathFxs* fx = &fxs[i];
        const bool inbounds = (fx->len + (*pos)) < maxlen;

        if (inbounds && !strncmp(fx->name, src, fx->len)) {
            *pos += fx->len - 1;
            return fx->kind;
        }
    }

    return token_kind_unknown;
}

#define TEXT "&A4 34 4454 \"dwehfwkhfoiewh\" &AAAAA88 @Sin @Cos @Pi @E 33 @jskiwhd"

int main ()
{
    Cell c;
    snprintf(c.fx_txt, strlen(TEXT) + 1, "%s", TEXT);
    c.fxch = strlen(TEXT) + 1;

    lexer_lex(NULL, &c);
    return 0;
}

