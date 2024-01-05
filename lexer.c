#include "lexer.h"
#include <ctype.h>

static TokenType find_type_of (const char);
static void get_literal (const char*, uint16_t*, Token*);

static uint16_t get_function (const char*, const uint16_t, Token*);

void lexer_lex (const Spread* spread, Cell* cc)
{
    if (!cc->nth_fx_ch) {
        cc->as.number = 0;
        cc->type = cell_is_empt;
        return;
    }

    Formula* fx   = &cc->fx;
    fx->nth_token = 0;

    for (uint16_t i = 0; i < cc->nth_fx_ch; i++) {
        const char a = cc->as_formula[i];
        if (isspace(a)) continue;
        Token token = { .type = find_type_of(a) };

        if (IS_IT_LIT_TOKEN(token.type))
            get_literal(cc->as_formula, &i, &token);

        else if (token.type == token_is_func)
            i += get_function(cc->as_formula + i, cc->nth_fx_ch - i, &token);

        /* If the first token is constant or non-formula
         * token there is not reason to keep getting tokens
         * since the cell is just plain text.
         * */
        if (!fx->nth_token && (IS_IT_CONSTANT(token.type) || !IS_IT_FORMULA(token.type))) {
            if (token.type != token_is_numb) {
                cc->as.text = cc->as_formula;
                cc->type = cell_is_text;
            }
            else {
                cc->as.number = token.as.number;
                cc->type = cell_is_numb;
            }

            return;
        }

        if (fx->nth_token == DEBUT_CELL_TOKEN_CAP) {
            parse_set_error(cc, DEBUT_ERR_MAX_CAP_REACHED, DEBUT_CELL_TOKEN_CAP);
            return;
        }

        memcpy(&fx->tokens[fx->nth_token++], &token, sizeof(Token));
    }

    parse_solve_cell(cc);
}

static TokenType find_type_of (const char a)
{
    switch (a) {
        case '@': return token_is_func;
        case '&': return token_is_refc;
        case ',': return token_is_cmma;
        case '+': return token_is_plus;
        case '*': return token_is_tims;
        case '/': return token_is_dvsn;
        case '%': return token_is_modd;
        case '$': return token_is_dolr;
        case '?': return token_is_qust;
        case '(': return token_is_lfpr;
        case ')': return token_is_ripr;
        case '-': return token_is_mins;
    }

    return isdigit(a) ? token_is_numb : token_is_word;
}

static void get_literal (const char* src, uint16_t* pos, Token* token)
{
    const TokenType what_is_it = token->type;

    typedef int (*defines) (const int);
    defines fx = (what_is_it == token_is_numb) ? isdigit : isalnum;

    const uint16_t prev_pos = *pos;
    do { *pos += 1; } while (fx(src[*pos]));

    *pos -= 1;
    if (what_is_it == token_is_numb) {
        token->as.number = strtod(src + prev_pos, NULL);
        return;
    }

    // TODO: Get references
}

typedef struct SpreadFunction {
    const char* name;
    uint16_t len;
    TokenType token;
} SpreadFunction;

static uint16_t get_function (const char* src, const uint16_t left, Token* t)
{
    static const uint16_t nfuncs = 10;
    static const SpreadFunction functions[] = {
        {"@E",    2, token_is_eulr},
        {"@PI",   3, token_is_pi  },
        {"@Sin",  4, token_is_sinn},
        {"@Cos",  4, token_is_coss},
        {"@Max",  4, token_is_maxx},
        {"@Min",  4, token_is_minn},
        {"@ASin", 5, token_is_asin},
        {"@ACos", 5, token_is_acos},
        {"@ATan", 5, token_is_atan},
        {"@Sqrt", 5, token_is_sqrt},
    };

    for (uint16_t i = 0; i < nfuncs; i++) {
        const SpreadFunction *fx = &functions[i];
        if ((left >= fx->len) && !strncmp(src, fx->name, fx->len)) {
            t->type = fx->token;
            return fx->len - 1;
        }
    }

    t->type = token_is_unkn;
    return 0;
}

int main () {
    Cell cell;

    char* content = "$3 + 5 + 8";
    size_t siz = strlen(content);
    cell.nth_fx_ch = (uint16_t) siz;

    snprintf(cell.as_formula, siz + 1, "%s", content);
    lexer_lex(NULL, &cell);

    return 0;
}
