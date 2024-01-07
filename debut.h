#ifndef DEBUT_DEBUT_H
#define DEBUT_DEBUT_H
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define DEBUT_FORMULA_LENGTH    512
#define DEBUT_CELL_DATA_LENGTH  64

typedef enum CellKind {
    cell_kind_error  = 0,
    cell_kind_text,
    cell_kind_number,
    cell_kind_empty
} CellKind;

typedef enum TokenKind {
    token_kind_unknown   = 0,
    token_kind_number    = 1,
    token_kind_sin       = 2,
    token_kind_cos       = 3,
    token_kind_asin      = 4,
    token_kind_acos      = 5,
    token_kind_atan      = 6,
    token_kind_sqrt      = 7,
    token_kind_pi        = 8,
    token_kind_e         = 9,

    token_kind_string    = '"',
    token_kind_reference = '&',
    token_kind_lf_paren  = '(',
    token_kind_rg_paren  = ')',
    token_kind_function  = '@',
    token_kind_add       = '+',
    token_kind_sub       = '-',
    token_kind_mul       = '*',
    token_kind_div       = '/',
} TokenKind;

typedef struct WindInfo {
    uint16_t maxx, maxy;
    uint16_t nRows, nCols;
    uint16_t cur_row, cur_col;
    uint16_t leftpadding;
} WindInfo;

typedef struct Token {
    union {
        long double number;
        void* reference;
        char* string;
    } as;

    uint16_t len_as_str;
    TokenKind kind;
} Token;

typedef struct Cell {
    char fx_txt[DEBUT_FORMULA_LENGTH];
    union {
        char text[DEBUT_CELL_DATA_LENGTH];
        long double number;
    } as;

    uint16_t fxch;
    CellKind kind;
} Cell;

typedef struct Spread {
    Cell* cells;
    WindInfo winf;
} Spread;

#endif
