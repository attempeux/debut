#include "lexer.h"
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#define IS_IT_CONSTANT(a)       ((a == token_is_numb) || (a == token_is_word))
#define IS_IT_LIT_TOKEN(a)      (IS_IT_CONSTANT(a)    || (a == token_is_refc))

#define ERROR_UNKNOWN_TOKEN     0
#define ERROR_MAX_CAPACITY      1
#define ERROR_UNKNOWN_FORMULA   2

static TokenType find_type_of (const char, const char);
static uint16_t get_literal (const char*, uint16_t*, const TokenType);

static void get_function (const char*, const uint16_t, Token*);
static void set_error_on_cc (Cell*, const uint16_t, ...);

void lexer_lex (const Spread* spread, Cell* cc)
{
    if (!cc->nth_ch) {
        cc->type = cell_is_empt;
        return;
    }

    cc->nth_token = 0;
    const uint32_t row = spread->grid.c_row, col = spread->grid.c_col;

    for (uint16_t i = 0; i < cc->nth_ch; i++) {
        const char a = cc->data[i];

        if (isspace(a)) continue;
        Token token = {
            .data = cc->data + i,
            .len  = 1,
            .type = find_type_of(a, ((i + 1) < cc->nth_ch) ? cc->data[i + 1] : 0)
        };

        if (IS_IT_LIT_TOKEN(token.type))
            token.len = get_literal(cc->data, &i, token.type);

        else if (token.type == token_is_func) {
            get_function(cc->data + i, cc->nth_ch - i, &token);
            i += token.len - 1;
        }

        if (IS_IT_CONSTANT(token.type) && !cc->nth_token) {
            cc->type      = (token.type == token_is_numb) ? cell_is_numb : cell_is_text;
            cc->data[++i] = 0;
            cc->nth_ch    = i;
            return;
        }

        if (token.type == token_is_unkn) {
            set_error_on_cc(cc, ERROR_UNKNOWN_TOKEN, row, col, token.len, token.data);
            return;
        }

        if (cc->nth_token == DEBUT_CELL_TOKEN_CAP) {
            set_error_on_cc(cc, ERROR_MAX_CAPACITY, row, col, DEBUT_CELL_TOKEN_CAP);
            return;
        }

        memcpy(&cc->tokens[cc->nth_token++], &token, sizeof(Token));
    }

    switch (cc->tokens[0].type) {
        case token_is_dolr: cc->type = cell_is_numb; break;
        case token_is_qust: cc->type = cell_is_numb; break;
        default: {
            set_error_on_cc(cc, ERROR_UNKNOWN_FORMULA, row, col);
            return;
        }
    }
}

static TokenType find_type_of (const char a, const char b)
{
    switch (a) {
        case '@': case '&':
        case ',': case '+':
        case '*': case '/':
        case '%': case '$':
        case '?': case '(':
        case ')': case '{':
        case '}': return a;
    }

    if (isalpha(a))
        return token_is_word;

    if ((a == '-'))
        return isdigit(b) ? token_is_numb : token_is_mins;

    return isdigit(a) ? token_is_numb : token_is_unkn;
}

static uint16_t get_literal (const char* src, uint16_t* pos, const TokenType is)
{
    typedef int (*defines) (const int);
    defines fx = (is == token_is_numb) ? isdigit : isalnum;

    uint16_t len = 0;
    do {
        len++;
        *pos += 1;
    } while (fx(src[*pos]));

    *pos -= 1;
    return len;
}

typedef struct SpreadFunction {
    const char* name;
    uint16_t len;
    TokenType token;
} SpreadFunction;

static void get_function (const char* src, const uint16_t left, Token* t)
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
            t->len  = fx->len;
            t->type = fx->token;
            return;
        }
    }

    t->type = token_is_unkn;
}

static void set_error_on_cc (Cell* cc, const uint16_t err, ...)
{
    va_list args;
    va_start(args, err);

    static const char* errfmts[] = {
        "(%d, %d): got unknown token: %.*s.",
        "(%d, %d): have reached maximum token capacity; %d Tokens.",
        "(%d, %d): unknown formula; cannot proceed."
    };

    vsnprintf(cc->as_error, DEBUT_ERR_MSG_LENGTH, errfmts[err], args);
    cc->type = cell_is_errr;
    va_end(args);
}

