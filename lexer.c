#include "lexer.h"
#define MUST_DEFINE_TOKEN(a)    ((a == token_is_numb) || (a == token_is_word) || (a == token_is_refc))

static TokenType find_type_of (const char, const char);
static uint16_t define_token (const char*, uint16_t*, const TokenType);

static void define_function (const char*, const uint16_t, Token*);

void lexer_lex (const Spread* spread, Cell* cc)
{
    if (!cc->modified) return;

    bool isconst = false;
    for (uint16_t i = 0; i < cc->nth_ch && !isconst; i++) {
        const char a = cc->data[i];

        if (isspace(a)) continue;
        Token token = {
            .data = cc->data + i,
            .len  = 1,
            .type = find_type_of(a, ((i + 1) < cc->nth_ch) ? cc->data[i + 1] : 0)
        };


        if (MUST_DEFINE_TOKEN(token.type))
            token.len = define_token(cc->data, &i, token.type);

        else if (token.type == token_is_func) {
            define_function(cc->data + i, cc->nth_ch - i, &token);
            i += token.len - 1;
        }

        fprintf(stderr, "token: <%.*s %d>\n", token.len, token.data, token.len);
    }

    cc->modified = false;
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

static uint16_t define_token (const char* src, uint16_t* pos, const TokenType is)
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

static void define_function (const char* src, const uint16_t left, Token* t)
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
        if ((left > fx->len) && !strncmp(src, fx->name, fx->len)) {
            t->len  = fx->len;
            t->type = fx->token;
            return;
        }
    }

    t->type = token_is_unkn;
}

