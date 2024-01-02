#ifndef _DEBUT_H
#define _DEBUT_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ncurses.h>
#include <stdbool.h>

#define DEBUT_CELL_WIDTH    14

typedef struct Grid {
    uint32_t nXbytes, nYbytes;
    uint32_t ncolumns, nrows;
    uint32_t left_padding;
} Grid;

typedef struct Spread {
    Grid grid;
} Spread;

#endif
