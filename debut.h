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
#define DEBUT_CELL_FORMULA_LEN  256             /* Maximum number of characters while writing the formula. */
#define DEBUT_CELL_ERROR_LEN    64              /* Maximum number of characters to write an error. */

#define DEBUT_WRT_CC_AT         0, 4
#define DEBUT_WRT_FM_AT         1, 4

typedef enum TokenType {
    token_is_unkn,
    token_is_word,
    token_is_numb,
    token_is_refc,

    token_is_func,
    token_is_sqrt,
    token_is_coss,
    token_is_sinn,
    token_is_acos,
    token_is_asin,
    token_is_atan,
    token_is_minn,
    token_is_maxx,
    token_is_eulr,
    token_is_pi,

    token_is_dolr,
    token_is_qust,

    token_is_cmma,
    token_is_mins,
    token_is_plus,
    token_is_tims,
    token_is_dvsn,
    token_is_modd,
    token_is_lfpr,
    token_is_ripr,
} TokenType;

typedef enum CellType {
    cell_is_errr = 0,
    cell_is_empt,
    cell_is_text,
    cell_is_numb
} CellType;

typedef struct Token {
    union {
        long double number;                         /* Value if the token is a number. */
        void*  reference;                           /* Value if the token is a cell reference. */
    } as;
    TokenType type;                                 /* What the token is. */
} Token;

typedef struct Formula {
    Token tokens[DEBUT_CELL_TOKEN_CAP];             /* Formula to be applied. */
    uint16_t nth_token;                             /* Number of tokens. */
    TokenType performs;                             /* It only marks either token_is_dolr if it does math, token_is_qust if conditional. */
} Formula;

typedef struct Cell {
    Formula fx;
    char as_formula[DEBUT_CELL_FORMULA_LEN];
    char as_error[DEBUT_CELL_ERROR_LEN];

    union {
        long double number;
        char* text;
    } as;

    uint16_t nth_fx_ch;
    CellType type;
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
