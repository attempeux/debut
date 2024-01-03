#include "lexer.h"
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#define IS_IT_CONSTANT(a)       ((a == token_is_numb) || (a == token_is_word))
#define IS_IT_LIT_TOKEN(a)      (IS_IT_CONSTANT(a)    || (a == token_is_refc))
#define IS_IT_FORMULA(a)        ((a == token_is_dolr) || (a == token_is_qust))

#define ERROR_MAX_CAPACITY      0

static TokenType find_type_of (const char);
static void get_literal (const char*, uint16_t*, Token*);

static uint16_t get_function (const char*, const uint16_t, Token*);
static void set_error_on_cc (Cell*, const uint16_t, ...);

void lexer_lex (const Spread* spread, Cell* cc)
{
    if (!cc->nth_fx_ch) {
        cc->type = cell_is_empt;
        return;
    }

    Formula* fx   = &cc->fx;
    fx->nth_token = 0;
    Token token   = {0};

    for (uint16_t i = 0; i < cc->nth_fx_ch; i++) {
        const char a = cc->as_formula[i];
        if (isspace(a)) continue;
        token.type = find_type_of(a);


        if (IS_IT_LIT_TOKEN(token.type))
            get_literal(cc->as_formula, &i, &token);

        else if (token.type == token_is_func)
            i += get_function(cc->as_formula + i, cc->nth_fx_ch - i, &token);

        /* If the very first token is a constant one, it
         * means no formula it is gonna be applied here.
         *
         * If the first token is nor constant nor formula
         * token there is not reason to keep getting tokens
         * since the cell is just plain text.
         * */
        if (!fx->nth_token && (IS_IT_CONSTANT(token.type) || !IS_IT_LIT_TOKEN(token.type))) {
            if (token.type != token_is_numb) {
                snprintf(cc->as.text, cc->nth_fx_ch + 1, "%.*s", DEBUT_CELL_VALUE_LEN, token.as.word);
                cc->type = cell_is_text;
            }
            else {
                cc->as.number = token.as.number;
                cc->type = cell_is_numb;
            }

            return;
        }

        if (fx->nth_token == DEBUT_CELL_TOKEN_CAP) {
            set_error_on_cc(cc, ERROR_MAX_CAPACITY, DEBUT_CELL_TOKEN_CAP);
            return;
        }

        memcpy(&fx->tokens[fx->nth_token++], &token, sizeof(Token));
    }
}

static TokenType find_type_of (const char a)
{
    switch (a) {
        case '@': case '&':
        case ',': case '+':
        case '*': case '/':
        case '%': case '$':
        case '?': case '(':
        case ')': case '{':
        case '}': case '-':
            return a;
    }

    return isdigit(a) ? token_is_numb : token_is_word;
}

static void get_literal (const char* src, uint16_t* pos, Token* token)
{
    typedef int (*defines) (const int);
    defines fx = (token->type == token_is_numb) ? isdigit : isalnum;

    const uint16_t prev_pos = *pos;
    do { *pos += 1; } while (fx(src[*pos]));

    switch (token->type) {
        case token_is_word: {
            token->as.word = (char*) src + prev_pos;
            token->length_as_word = *pos - prev_pos;
            return;
        }

        case token_is_numb: {
            token->as.number = strtod(src, NULL);
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

static void set_error_on_cc (Cell* cc, const uint16_t err, ...)
{
    va_list args;
    va_start(args, err);

    static const char* errfmts[] = {
        "maximum token capacity; which is %d Tokens.",
    };

    vsnprintf(cc->as_error, DEBUT_CELL_ERROR_LEN, errfmts[err], args);
    cc->type = cell_is_errr;
    va_end(args);
}

