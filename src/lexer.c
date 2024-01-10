#include "lexer.h"

static TokenKind find_out_token_kind (const char);
static void set_token_as_number (const char*, uint16_t*, long double*);

static void set_token_as_string (const char*, uint16_t*, Token*);
static void set_token_as_reference (const Spreadsheet*, const char*, uint16_t*, Cell*);

void lexer_lexer (const Spreadsheet* sp, Cell* ths_cell)
{
    const uint16_t nchs = ths_cell->fx_ntch;
    if (!nchs) {
        ths_cell->as.number = 0;
        ths_cell->kind = cell_kind_empty;
        return;
    }

    for (uint16_t i = 0; i < nchs; i++) {
        const char a = ths_cell->formula_txt[i];
        Token token = { .kind = find_out_token_kind(a) };

        switch (token.kind) {
            case tok_kind_number:
                set_token_as_number(ths_cell->formula_txt, &i, &token.as.number);
                fprintf(stderr, "number: %Lf\n", token.as.number);
                break;
            case tok_kind_string:
                set_token_as_string(ths_cell->formula_txt, &i, &token);
                fprintf(stderr, "string: %s\n", token.as.string);
                break;
            case tok_kind_reference:
                set_token_as_reference(sp, ths_cell->formula_txt, &i, token.as.ref);
            default:
                break;
        }

    }
}

static TokenKind find_out_token_kind (const char a)
{
    switch (a) {
        case '"': case '&': case '(': case ')': case '@':
        case '+': case '-': case '*': case '/': return a;
    }

    return isdigit(a) ? tok_kind_number : tok_kind_unknown;
}

static void set_token_as_number (const char* src, uint16_t* _i, long double* as_number)
{
    char* ends_at = NULL;
    *as_number = strtod(src + *_i, &ends_at);
    *_i += (ends_at - (src + *_i)) - 1;
}

static void set_token_as_string (const char* src, uint16_t* _i, Token* token)
{
    char a;
    do {
        a = src[*_i];
        token->as.string[token->len_as_str++] = a;
        *_i += 1;
    } while (a != '"' && token->len_as_str < DEBUT_TOKEN_TEXT_CAP);
}

static void set_token_as_reference (const Spreadsheet* sp, const char* src, uint16_t* _i, Cell* as_ref)
{
    uint16_t column = 0, hilfsvar = 0;
    long double row_help;

    *_i += 1;
    const size_t nletters = strcspn(src + *_i, "1234567890");

    for (; *_i < nletters; *_i += 1) {
        column += 26 * hilfsvar + (tolower(src[*_i] - 'A'));
        hilfsvar++;
    }

    set_token_as_number(src, _i, &row_help);
    uint16_t pos = (uint16_t) row_help * column;

    if (pos >= DEBUT_TOTAL_CELLS) {
        fprintf(stderr, "%d, %Lf, %d\n", column, row_help, pos);
        abort();
    }

    as_ref = &sp->cells[pos];
}


