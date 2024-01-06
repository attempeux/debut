#ifndef DEBUT_DEBUT_H
#define DEBUT_DEBUT_H
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <ncurses.h>

#define DEBUT_FORMULA_LENGTH    512

typedef enum CellKind {
    cell_kind_error  = 0,
    cell_kind_text,
    cell_kind_number,
    cell_kind_empty
} CellKind;

typedef struct WindInfo {
    uint16_t maxx, maxy;
    uint16_t nRows, nCols;
    uint16_t leftpadding;
} WindInfo;

typedef struct Cell {
    char fx_txt[DEBUT_FORMULA_LENGTH];
    uint16_t fxch;
    CellKind kind;
} Cell;

typedef struct Spread {
    Cell* cells;
    WindInfo winf;
} Spread;

#endif
