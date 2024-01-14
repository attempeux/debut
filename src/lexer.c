#include "lexer.h"

static TokenKind find_out_token_kind (const char);
static void set_token_as_number (const char*, uint16_t*, long double*);

static void set_token_as_string (const char*, uint16_t*, Token*);
static void set_token_as_reference (const Spreadsheet*, const char*, uint16_t*, Cell*);

static TokenKind kind_of_built_in_fx (const char*, uint16_t*, const uint16_t);

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
        if (isspace(a)) continue;

        Token token = { .kind = find_out_token_kind(a) };
        switch (token.kind) {
            case tok_kind_number:
                set_token_as_number(ths_cell->formula_txt, &i, &token.as.number);
                break;

            case tok_kind_string:
                set_token_as_string(ths_cell->formula_txt, &i, &token);
                break;

            case tok_kind_reference:
                set_token_as_reference(sp, ths_cell->formula_txt, &i, token.as.ref);
                break;

            case tok_kind_build_in_func:
                token.kind = kind_of_built_in_fx(ths_cell->formula_txt + i, &i, nchs - i);
                break;

            default:
                break;
        }

#ifdef LEXER_TEST
        const TokenKind k = token.kind;
        switch (k) {
            case tok_kind_number: printf("%.1Lf\n", token.as.number); break;
            case tok_kind_string: printf("%.*s\n", token.len_as_str, token.as.string); break;
            case tok_kind_unknown: printf("unkn\n"); break;
            default:
                if (((k >= tok_kind_sin_fx) && (k <= tok_kind_e_const)) || (k == tok_kind_reference)) break;
                printf("%c\n", a);
                break;
        }
#endif
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

/* This function get the literal strings, it saves the character at the *_ith position
 * as long as the string size does not exceed DEBUT_TOKEN_TEXT_CAP, if it does then the
 * string will be left til that point and the function will reach the closing quote to
 * not interpretet as another string in the next iterations.
 *      "this exceeds here..."
 *                         ~~ ` *_i will be set here.
 * */
static void set_token_as_string (const char* src, uint16_t* _i, Token* token)
{
    do {
        *_i += 1;
        token->as.string[token->len_as_str++] = src[*_i];
    } while (src[*_i] != '"' && token->len_as_str < DEBUT_TOKEN_TEXT_CAP);

    if (token->as.string[token->len_as_str - 1] == '"') {
        token->as.string[token->len_as_str - 1] = 0;
        return;
    }

    if (*_i >= DEBUT_CELL_TEXT_CAP) {
        puts("big string!");
        exit(0);
    }

    while (src[*_i] != '"') *_i += 1;
}

static void set_token_as_reference (const Spreadsheet* sp, const char* src, uint16_t* _i, Cell* as_ref)
{
    uint16_t column = 0;
    long double row_help;

    *_i += 1;
    switch (strcspn(src + *_i, "1234567890")) {
        case 1:
            column = toupper(src[*_i]) - 'A';
            *_i += 1;
            break;

        case 2:
            column += (toupper(src[*_i]) - 'A' + 1) * 26;
            column += toupper(src[*_i + 1]) - 'A';
            *_i += 2;
            break;

        default:
            puts("no-well-defined-ref");
            exit(0);
            break;
    }

    set_token_as_number(src, _i, &row_help);
    const uint16_t pos = (uint16_t) row_help * column;

    if (pos >= DEBUT_TOTAL_CELLS) {
        puts("cell outta bounds");
        exit(0);
    }

#ifdef LEXER_TEST
    printf("(%d, %.0Lf)\n", column, row_help);
#else
    as_ref = &sp->cells[pos];
#endif
}

typedef struct BuiltInFx {
    const char* name;
    const uint16_t len;
    const TokenKind kind;
} BuiltInFx;

static TokenKind kind_of_built_in_fx (const char* src, uint16_t* _i, const uint16_t len)
{
    static const uint16_t nbuiltin = 8;
    static const BuiltInFx fxs[] = {
        {"@asin", 5, tok_kind_asin_fx },
        {"@acos", 5, tok_kind_acos_fx },
        {"@atan", 5, tok_kind_atan_fx },
        {"@sqrt", 5, tok_kind_sqrt_fx },
        {"@sin",  4, tok_kind_sin_fx  },
        {"@cos",  4, tok_kind_cos_fx  },
        {"@pi",   3, tok_kind_pi_const},
        {"@e",    2, tok_kind_e_const },
    };

    for (uint16_t i = 0; i < nbuiltin; i++) {
        const BuiltInFx* bfx = &fxs[i];
        if (len >= bfx->len && !strncmp(bfx->name, src, bfx->len)) {

#ifdef LEXER_TEST
            printf("%s\n", bfx->name);
#endif

            *_i += bfx->len - 1;
            return bfx->kind;
        }
    }

    return tok_kind_unknown;
}


#ifdef LEXER_TEST 
int main (int argc, char** argv)
{
    Cell cell;
    snprintf(cell.formula_txt, DEBUT_CELL_TEXT_CAP - 1, "%s", argv[1]);
    cell.fx_ntch = strlen(argv[1]);

    lexer_lexer(NULL, &cell);
    return 0;
}
#endif
