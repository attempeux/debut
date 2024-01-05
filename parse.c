#include "parse.h"
#include <math.h>

typedef struct ASTNode {
    struct ASTNode* left, *right;
    Token* value;
    bool within_parentheses;
} ASTNode;

static ASTNode* build_expression (Cell*, Formula*, ASTNode*);
static ASTNode* get_child (Cell*, Formula*);

static void print_tree (ASTNode*);

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

    /* It starts at one since the first token works to determinate
     * that an operation is gonna be perform, it does not count as
     * a value.
     * */
    fx->inspecting_n = 1;
    while (fx->inspecting_n < fx->nth_token) {
        tree = build_expression(cc, fx, tree);
    }

    printf("OP: %d\n", tree->value->type);
    printf("R1: %Lf\n", tree->left->value->as.number);
    printf("R2: %Lf\n", tree->right->value->as.number);

    print_tree(tree);
}

static ASTNode* build_expression (Cell* cc, Formula* fx, ASTNode* __left)
{
    if (fx->tokens[fx->inspecting_n].type == token_is_ripr) { // IS THERE LEFT ONE???
        fx->inspecting_n++;
        return __left;
    }

    ASTNode* node = (ASTNode*) calloc(1, sizeof(ASTNode));

    if (__left) node->left = __left;
    else node->left = get_child(cc, fx); // ALWAYS SUCCESS?

    const Token* op = &fx->tokens[fx->inspecting_n++];

    if (!IS_IT_OPERATOR(op->type)) exit(2);
    node->value = (Token*) op;

    node->right = get_child(cc, fx);
    return node;
}

static ASTNode* get_child (Cell* cc, Formula* fx) // CHECK BOUNDS OF TOKENS HAVENT RAN OUT YET
{
    const Token* c = &fx->tokens[fx->inspecting_n++];

    if (c->type == token_is_lfpr) {
        return build_expression(cc, fx, NULL);
    }

    if (!IS_IT_LIT_TOKEN(c->type)) exit(1);

    ASTNode* node = (ASTNode*) calloc(1, sizeof(ASTNode));
    node->value = (Token*) c;

    return node;
}

static void print_tree (ASTNode* a)
{
    if (!a->left && !a->right) return;
    static const char* ops = "-+*/";

        printf("[%Lf] ", a->left->value->as.number);
    printf("%c", ops[a->value->type - token_is_mins]);

        printf(" [%Lf]\n", a->right->value->as.number);


    print_tree(a->right);
    print_tree(a->left);
    puts("-*-");
}
