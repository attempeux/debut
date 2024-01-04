#include "lexer.h"
#include <ctype.h>

#define IS_IT_CONSTANT(a)       ((a == token_is_numb) || (a == token_is_word))
#define IS_IT_LIT_TOKEN(a)      (IS_IT_CONSTANT(a)    || (a == token_is_refc))
#define IS_IT_FORMULA(a)        ((a == token_is_dolr) || (a == token_is_qust))

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
    Token token   = {0};

    for (uint16_t i = 0; i < cc->nth_fx_ch; i++) {
        const char a = cc->as_formula[i];
        if (isspace(a)) continue;

        token.as.word = cc->as_formula + i;
        token.type    = find_type_of(a);

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
                snprintf(cc->as.text, cc->nth_fx_ch + 1, "%.*s", DEBUT_CELL_VALUE_LEN, cc->as_formula);
                cc->type = cell_is_text;
            }
            else {
                cc->as.number = token.as.number;
                cc->type = cell_is_numb;
            }

            return;
        }

        if (fx->nth_token == DEBUT_CELL_TOKEN_CAP) {
            parse_set_error(cc, SET_ERROR_MAX_CAPACITY, DEBUT_CELL_TOKEN_CAP);
            return;
        }

        fprintf(stderr, "Token: %d\n", token.type);
        memcpy(&fx->tokens[fx->nth_token++], &token, sizeof(Token));
    }

    parse_eval_expr(spread, cc);
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
    typedef int (*defines) (const int);
    defines fx = (token->type == token_is_numb) ? isdigit : isalnum;

    const uint16_t prev_pos = *pos;
    do { *pos += 1; } while (fx(src[*pos]));

    *pos -= 1;
    switch (token->type) {
        case token_is_word: {
            token->as.word = (char*) src + prev_pos;
            token->length_as_word = *pos - prev_pos;
            return;
        }

        case token_is_numb: {
            token->as.number = strtod(src + prev_pos, NULL);
            return;
        }

        default: { assert("NO YET"); }
    }
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
        {"@Max",  4, token_is_coss},
        {"@Min",  4, token_is_coss},
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
