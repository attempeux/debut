#ifndef DEBUT_DEBUT_H
#define DEBUT_DEBUT_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* Since we do not wanna make the next excel but just a spreadsheet engine
 * for fun we do not need to much cells. 77 is the number of columns that
 * takes to reach the 'BZ' column starting from 0 index.
 * */
#define DEBUT_MAX_ROWS          150
#define DEBUT_MAX_COLS          77
#define DEBUT_TOTAL_CELLS       DEBUT_MAX_ROWS * DEBUT_MAX_COLS
#define DEBUT_CELL_WIDTH        10
#define DEBUT_CELL_TEXT_CAP     64
#define DEBUT_TOKEN_TEXT_CAP    16

typedef enum CellKind {
    cell_kind_error  = 0,
    cell_kind_number,
    cell_kind_string,
    cell_kind_empty
} CellKind;

typedef enum TokenKind {
    tok_kind_unknown       = 0,
    tok_kind_number        = 1,
    tok_kind_sin_fx        = 2,
    tok_kind_cos_fx        = 3,
    tok_kind_asin_fx       = 4,
    tok_kind_acos_fx       = 5,
    tok_kind_atan_fx       = 6,
    tok_kind_sqrt          = 7,
    tok_kind_string        = '"',
    tok_kind_reference     = '&',
    tok_kind_lf_par        = '(',
    tok_kind_rg_par        = ')',
    tok_kind_build_in_func = '@',
    tok_kind_add_fx        = '+',
    tok_kind_sub_fx        = '-',
    tok_kind_mul_fxu       = '*',
    tok_kind_div_fx        = '/',
    tok_kind_mod_fx        = '-',
} TokenKind;

typedef struct Cell Cell; 

typedef struct Token {
    union {
        Cell* ref;
        char string[DEBUT_TOKEN_TEXT_CAP];
        long double number;
    } as;

    uint16_t len_as_str;
    TokenKind kind;
} Token;

typedef struct Cell {
    char formula_txt[DEBUT_CELL_TEXT_CAP];

    union {
        long double number;
        char text[DEBUT_CELL_TEXT_CAP];
    } as;

    uint16_t fx_ntch;
    CellKind kind;
} Cell;

typedef struct WindowInfo {
    uint16_t win_rows, win_columns;
    uint16_t cur_row, cur_col;
    uint16_t row_offset, col_offset;
    uint16_t displayrows, displaycols;
    uint16_t rel_row_pos, rel_col_pos;
} WindowInfo;

typedef struct Spreadsheet {
    WindowInfo win_info;
    Cell* cells;
    char* filename;
    size_t flnamesz;
} Spreadsheet;

#endif
