#ifndef _DEBUT_H
#define _DEBUT_H
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#define DEBUT_CELL_WIDTH        10              /* Width of each cell on the table. */
#define DEBUT_CELL_TOKEN_CAP    128             /* Maximum number of tokens per cell. */
#define DEBUT_CELL_FORMULA_LEN  256             /* Maximum number of characters while writing the formla. */
#define DEBUT_CELL_ERROR_LEN    64              /* Maximum number of characters to write an error. */
#define DEBUT_CELL_VALUE_LEN    64              /* Maximum number of characters to write the answer of a cell. */

#define DEBUT_WRT_FM_AT         1, 4
#define DEBUT_WRT_CC_AT         0, 4

typedef enum TokenType {
    token_is_unkn = 0,
    token_is_word = 1,
    token_is_numb = 2,
    token_is_sqrt = 3,
    token_is_coss = 4,
    token_is_sinn = 5,
    token_is_acos = 6,
    token_is_asin = 7,
    token_is_atan = 8,
    token_is_minn = 9,
    token_is_maxx = 10,
    token_is_eulr = 11,
    token_is_pi   = 12,

    token_is_func = '@',
    token_is_refc = '&',
    token_is_cmma = ',',
    token_is_mins = '-',
    token_is_plus = '+',
    token_is_tims = '*',
    token_is_dvsn = '/',
    token_is_modd = '%',
    token_is_dolr = '$',
    token_is_qust = '?',
    token_is_lfpr = '(',
    token_is_ripr = ')',
    token_is_lfbr = '{',
    token_is_ribr = '}',
} TokenType;

typedef enum CellType {
    cell_is_empt,
    cell_is_errr,
    cell_is_text,
    cell_is_numb
} CellType;

typedef struct Token {
    union {
        void*  reference;                           /* This is actually a Cell pointer. */
        char*  word;
        double number;
    } as;                                           /* Different literal values a token can be. */
    uint16_t length_as_word;                        /* Length of the word in case the token is type word. */
    uint16_t byte_definition;                       /* Position in the formula where the byte was defined. */
    TokenType type;
} Token;

typedef struct Formula {
    Token tokens[DEBUT_CELL_TOKEN_CAP];
    uint16_t nth_token;
} Formula;

typedef struct Cell {
    Formula fx;
    char as_formula[DEBUT_CELL_FORMULA_LEN];
    char as_error[DEBUT_CELL_ERROR_LEN];

    union {
        double number;
        char text[DEBUT_CELL_VALUE_LEN];
    } as;

    uint16_t nth_fx_ch;
    CellType type;
    bool solved;
} Cell;

typedef struct Grid {
    uint32_t nXbytes, nYbytes;                      /* Total number of bytes available to display the table. */
    uint32_t ncolumns, nrows;                       /* How many columns and rows are available. */
    uint32_t left_padding;                          /* Number of bytes used to print the rows number column. */
    uint32_t c_row, c_col;                          /* Number of row and column as the user moves around. */
} Grid;

typedef struct Spread {
    Grid grid;
    Cell* cells;
} Spread;

#endif
