#include "parse.h"

#define OPERATORS_STARTS_AT  DEBUT_CELL_TOKEN_CAP / 2
#define IS_VALID_SYMBOL(a)   ((a >= token_is_mins) && (a <= token_is_modd))

/* Since the formulas are given in a hard way to evaluate such as: 4 + 21 * 3 the program
 * does not really know and does not care the order they must be performed but we do, so
 * if the formula of the cell looks like that we want to organizate it in a faster way to
 * read and solve.
 * */
typedef struct SimplerFx {
    Token fx[DEBUT_CELL_TOKEN_CAP];
    uint16_t nth_operand, nth_operator;
} SimplerFx;

static const struct {
    uint16_t precedence;
    void* (*fx) (const void*, const void*);
} Ops[] = {
    {'-', NULL},
    {'+', NULL},
    {'*', NULL},
    {'/', NULL},
    {'%', NULL},
};

static void deal_with_operators (Cell*, SimplerFx*, Token*);
static bool perform_exchage (const TokenType, const TokenType);

void parse_set_error (Cell* cc, uint16_t err, ...)
{
    va_list args;
    va_start(args, err);

    static const char* errfmts[] = {
        "maximum token capacity; which is %d Tokens.",
        "unknown math/logical operator at %d byte.",
        "missing opening parentheses at %d byte."
    };

    snprintf(cc->as.text, 8, "!#ERROR");
    vsnprintf(cc->as_error, DEBUT_CELL_ERROR_LEN, errfmts[err], args); /* @TODO: REPLACE THIS FOR A STACK */
    cc->type = cell_is_errr;
    va_end(args);
}

void parse_eval_expr (const Spread* spread, Cell* cc)
{
    static SimplerFx sFx = {
        .nth_operand = 0,
        .nth_operator = OPERATORS_STARTS_AT
    };

    Formula* fx = &cc->fx;

    for (uint16_t i = 1; i < fx->nth_token; i++) {
        Token* token = &fx->tokens[i];
        
        if (token->type == token_is_numb)
            memcpy(&sFx.fx[sFx.nth_operand++], token, sizeof(Token));
    }
}


static void deal_with_operators (Cell* cc, SimplerFx* sFx, Token* token)
{
    static uint16_t within_pars = 0;
    const TokenType type = token->type;

    if (type == token_is_lfpr) {
        within_pars++;
        memcpy(&sFx->fx[sFx->nth_operator++], token, sizeof(Token));
        return;
    }

    if (type == token_is_ripr) {
        if (!within_pars) parse_set_error(cc, SET_ERROR_MISSING_LEFT_PAR, token->byte_definition);
        else within_pars--;

        // TODO
        return;
    }

    if (!IS_VALID_SYMBOL(type)) {
        parse_set_error(cc, SET_ERROR_UNKNOWN_SYM, token->byte_definition);
        return;
    }

    while (!within_pars && (sFx->nth_operator > OPERATORS_STARTS_AT)) {
        Token *prev = &sFx->fx[sFx->nth_operator - 1];
        if (!perform_exchage(token->type, prev->type))
            break;

        memcpy(&sFx->fx[sFx->nth_operand++], prev, sizeof(Token));
        sFx->nth_operator--;
    }

    memcpy(&sFx->fx[sFx->nth_operator++], token, sizeof(Token));
}

static bool perform_exchage (const TokenType a, const TokenType b)
{
    return Ops[a - token_is_mins].precedence <= Ops[b - token_is_mins].precedence;
}
