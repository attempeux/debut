#include "parse.h"

#define IS_VALID_SYMBOL(a)      ((a >= token_is_mins) && (a <= token_is_modd))
#define OPERATORS_STARTS_AT     DEBUT_CELL_TOKEN_CAP / 2
#define NUMBER_STACK_SIZE       16

/* Since the formulas are given in a hard way to evaluate such as: 4 + 21 * 3 the program
 * does not really know and does not care the order they must be performed but we do, so
 * if the formula of the cell looks like that we want to organizate it in a faster way to
 * read and solve.
 * */
typedef struct SimplerFx {
    Token fx[DEBUT_CELL_TOKEN_CAP];
    uint16_t nth_operand, nth_operator;
} SimplerFx;

static double fx_sub (const double a, const double b) { return a - b; }
static double fx_add (const double a, const double b) { return a + b; }
static double fx_mul (const double a, const double b) { return a * b; }
static double fx_div (const double a, const double b) { return a / b; }

static const struct {
    uint16_t precedence;
    double (*eval_math) (const double, const double);
} Ops[] = {
    {1, fx_sub},
    {1, fx_add},
    {2, fx_mul},
    {2, fx_div},
    {2, NULL},
};

static void deal_with_operators (Cell*, SimplerFx*, Token*);
static bool perform_exchage (const TokenType, const TokenType);

static void rewrite_original_formula (Cell*, SimplerFx*);

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
        else
            deal_with_operators(cc, &sFx, token);
    }

    rewrite_original_formula(cc, &sFx);
    update_ou_solve(cc);

    memset(&sFx, 0, sizeof(SimplerFx));
    sFx.nth_operator = OPERATORS_STARTS_AT;
}

void update_ou_solve (Cell* cc) // NOT ONLY MATH
{
    Formula* fx = &cc->fx;

    uint16_t nth_num = 0;
    double numberstack[NUMBER_STACK_SIZE] = {0};

    for (uint16_t i = 0; i < fx->nth_token; i++) {
        Token* token = &fx->tokens[i];

        if (token->type == token_is_numb) {
            numberstack[nth_num++] = token->as.number;
        }
        else {
            // CHECK IT IS ALWAYS GREATER THAN 2
            numberstack[nth_num - 2] = Ops[token->type - token_is_mins].eval_math(
                numberstack[nth_num - 2],
                numberstack[nth_num - 1]
            );
            nth_num--;
        }
    }

    fprintf(stderr, "\nANS: %f\n", numberstack[0]);
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
        if (!within_pars) { parse_set_error(cc, SET_ERROR_MISSING_LEFT_PAR, token->byte_definition); return; }

        // TODO

        while (sFx->fx[--sFx->nth_operator].type != token_is_lfpr)
            memcpy(&sFx->fx[sFx->nth_operand++], &sFx->fx[sFx->nth_operator], sizeof(Token));

        within_pars--;
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

static void rewrite_original_formula (Cell* cc, SimplerFx* sFx)
{
    do {
        --sFx->nth_operator;
        memcpy(&sFx->fx[sFx->nth_operand++], &sFx->fx[sFx->nth_operator], sizeof(Token));
    } while (sFx->nth_operator > OPERATORS_STARTS_AT);

    Formula* fx = &cc->fx;
    memcpy(&fx->tokens, &sFx->fx, sizeof(Token) * DEBUT_CELL_TOKEN_CAP);
    fx->nth_token = sFx->nth_operand;
}
