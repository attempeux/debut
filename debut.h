#ifndef _DEBUT_H
#define _DEBUT_H
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#define DEBUT_CELL_WIDTH        10
#define DEBUT_CELL_LENGTH       512
#define DEBUT_CELL_TOKEN_CAP    64
#define DEBUT_ERR_MSG_LENGTH    64

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
        void*  reference;                           /* This is actually a Cell* */
        char*  word;
        double number;
    } as;                                           /* Different literal values a token can be. */
    uint16_t length_as_word;
    uint16_t byte_definition;
    TokenType type;
} Token;

typedef struct Cell {
    Token tokens[DEBUT_CELL_TOKEN_CAP];
    char data[DEBUT_CELL_LENGTH];
    char as_error[DEBUT_ERR_MSG_LENGTH];
    uint16_t nth_token, nth_ch;
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
