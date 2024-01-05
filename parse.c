#include "parse.h"
#include <math.h>

typedef struct ASTNode {
    struct ASTNode* left, *right;
    Token* value;
    bool within_parentheses;
} ASTNode;

static ASTNode* build_node (Cell*, Formula*, uint16_t *, ASTNode*);

void parse_set_error (Cell* cc, uint16_t which, ...)
{
    va_list args;
    va_start(args, which);

    static const char* errfmts[] = {
        "maximum token capacity reached; max = %d Tokens.",
        "bad use of word type, use strings instead."
    };

    snprintf(cc->as.text, 8, "!#ERROR");
    vsnprintf(cc->as_error, DEBUT_CELL_ERROR_LEN, errfmts[which], args); /* @TODO: REPLACE THIS FOR A STACK */
    cc->type = cell_is_errr;
    va_end(args);
}

void parse_solve_cell (Cell* cc)
{
    Formula* fx = &cc->fx;
    ASTNode* tree = NULL;

    for (uint16_t x = 1; x < fx->nth_token; x++) {
        tree = build_node(cc, fx, &x, tree);
    }

    printf("OP: %d\n", tree->value->type);
    printf("R1: %Lf\n", tree->left->value->as.number);
    printf("R2: %Lf\n", tree->right->value->as.number);
}

static ASTNode* build_node (Cell* cc, Formula* fx, uint16_t *pos, ASTNode* __left)
{
    ASTNode* node = (ASTNode*) calloc(1, sizeof(ASTNode));

    Token* token = &fx->tokens[*pos];

    // CHECK PARENTHESES
    if (!__left) {
        if (!IS_IT_LIT_TOKEN(token->type)) exit(1);
        *pos += 1;

        node->left = (ASTNode*) calloc(1, sizeof(ASTNode));
        node->left->value = token;
    } else {

        node->left = __left;
    }

    node->value = &fx->tokens[*pos];

    *pos += 1;
    if (!IS_IT_LIT_TOKEN(fx->tokens[*pos].type)) exit(2);
    node->right = (ASTNode*) calloc(1, sizeof(ASTNode));
    node->right->value = &fx->tokens[*pos];

    return node;
}
