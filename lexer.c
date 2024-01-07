#include "debut.h"
#include <assert.h>
#include <string.h>
#include <errno.h>

static TokenKind figure_out_kind (const char);

static uint16_t get_string_literal (const char*, uint16_t*, const uint16_t);
static long double get_number_literal (const char*, uint16_t*, const uint16_t);

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
        if (token.kind == token_kind_number) {
            printf("number: %Lf\n", get_number_literal(cuC->fx_txt, &i, len));
        }
        if (token.kind == token_kind_string) {
            uint16_t a = i, ss = get_string_literal(cuC->fx_txt, &i, len);
            printf("string: <%.*s> %d\n", ss, cuC->fx_txt + a + 1, ss);
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

    assert(ch == '"' && "incomplete string.");
    return --token_length;
}

static long double get_number_literal (const char* src, uint16_t* pos, const uint16_t len)
{
    char* init = (char*) &src[*pos], *final;
    long double a = strtold(src + *pos, &final);

    if (errno) {
        assert("ERROR");
    }

    *pos += (uint16_t) (final - init);
    return a;
}


#define TEXT "3345 3.34535 \"Hola como estas\" \"Bonjour\"34345'"

int main ()
{
    Cell c;
    memcpy(&c.fx_txt, &TEXT, strlen(TEXT));
    c.fxch = strlen(TEXT);

    lexer_lex(NULL, &c);
    return 0;
}
