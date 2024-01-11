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
                fprintf(stdout, "%.1Lf\n", token.as.number);
                break;
            case tok_kind_string:
                set_token_as_string(ths_cell->formula_txt, &i, &token);
                fprintf(stdout, "%.*s\n", token.len_as_str, token.as.string);
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
        puts("bounds!");
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

        default: // TODO: Add error.
            fprintf(stdout, "bounds!\n");
            exit(0);
            break;
    }


    set_token_as_number(src, _i, &row_help);
    const uint16_t pos = (uint16_t) row_help * column;

    if (pos >= DEBUT_TOTAL_CELLS) { // TODO: Add error.
        fprintf(stdout, "bounds!\n");
        exit(0);
    }

    fprintf(stdout, "(%d, %.0Lf)\n", column, row_help);
    //as_ref = &sp->cells[pos];
}


int main (int argc, char** argv)
{
    Cell cell;
    snprintf(cell.formula_txt, DEBUT_CELL_TEXT_CAP - 1, "%s", argv[1]);
    cell.fx_ntch = strlen(argv[1]);

    lexer_lexer(NULL, &cell);

    return 0;
}
