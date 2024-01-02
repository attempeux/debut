#ifndef _DEBUT_H
#define _DEBUT_H
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ncurses.h>
#include <stdbool.h>

#define DEBUT_CELL_WIDTH        14
#define DEBUT_CELL_LENGHT       512
#define DEBUT_CELL_TOKEN_CAP    64

typedef enum TokenType {
    token_is_number,
    token_is_unknown,
} TokenType;

typedef enum CellType {
    cell_is_empt,
    cell_is_errr,
    cell_is_text,
    cell_is_numb
} CellType;

typedef struct Token {
    char* data;
    uint16_t len;
    TokenType type;
} Token;

typedef struct Cell {
    Token tokens[DEBUT_CELL_TOKEN_CAP];
    char data[DEBUT_CELL_LENGHT];
    uint16_t nth_token, nth_ch;
    CellType type;
} Cell;

typedef struct Grid {
    uint32_t nXbytes, nYbytes;
    uint32_t ncolumns, nrows;
    uint32_t left_padding;
    uint32_t c_row, c_col;
} Grid;

typedef struct Spread {
    Grid grid;
    Cell* cells;
} Spread;

#endif
