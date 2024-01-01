#ifndef _DEBUT_H
#define _DEBUT_H
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <ncurses.h>
#include <stdbool.h>

#define CELL_MAX_LENGTH     128
#define CELL_MAX_TOKENS     128

typedef enum TokenType {
    token_types_here
} TokenType;

typedef enum CellType {
    cell_is_errr,
    cell_is_empt,
    cell_is_text,
    cell_is_numb,
} CellType;

typedef struct Token {
    char*     data;
    uint16_t  len;
    TokenType type;
} Token;

typedef struct Cell {
    Token    tokens[CELL_MAX_TOKENS];
    char     data[CELL_MAX_LENGTH];
    uint16_t nth_chr, nth_token;
    CellType type;
} Cell;

typedef struct FileContent {
    char*   src;
    size_t  len;
} FileContent;

typedef struct TableInfo {
    uint16_t max_rows, max_cols, max_cells;
} TableInfo;

/* this struct works to know where the new rows
 * begins in a unfixed size table, sadly this must
 * be implemented since a table can look like this:
 *  | aaa | bb
 *  | c
 *  | dd
 * and the program would not know where is @B0 for
 * instance, so marks' gonna save the positions where
 * new cells are defined.
 *
 * note: this is only used when -p argument is used.
 * */
typedef struct RowOffsets {
    uint16_t *marks, nth_mark, nth_cell;
} RowOffsets;

typedef struct Spread {
    FileContent src;
    Cell*       spread;
    RowOffsets  offsets;
    TableInfo   info;
    char*       filename_r, *filename_w;
    bool        ui_mode;
} Spread;

#endif
