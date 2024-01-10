#ifndef DEBUT_DEBUT_H
#define DEBUT_DEBUT_H
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct WindowInfo {
    uint16_t win_rows, win_columns;
    uint16_t cur_row, cur_col;
    uint16_t row_offset, col_offset;
    uint16_t displayrows, displaycols;
    uint16_t rel_row_pos, rel_col_pos;
} WindowInfo;

typedef struct Spreadsheet {
    WindowInfo win_info;
    char* filename;
    size_t flnamesz;
} Spreadsheet;

#endif
