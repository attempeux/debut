#ifndef DEBUT_DEBUT_H
#define DEBUT_DEBUT_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define DEBUT_CELL_TEXT_CAP     64

typedef enum CellKind {
    cell_kind_error  = 0,
    cell_kind_number,
    cell_kind_string,
    cell_kind_empty
} CellKind;

typedef struct WindowInfo {
    uint16_t win_rows, win_columns;
    uint16_t cur_row, cur_col;
    uint16_t row_offset, col_offset;
    uint16_t displayrows, displaycols;
    uint16_t rel_row_pos, rel_col_pos;
} WindowInfo;

typedef struct Cell {
    char formula_txt[DEBUT_CELL_TEXT_CAP];
    uint16_t fx_ntch;
    CellKind kind;
} Cell;

typedef struct Spreadsheet {
    WindowInfo win_info;
    Cell* cells;
    char* filename;
    size_t flnamesz;
} Spreadsheet;

#endif
