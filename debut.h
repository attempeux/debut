#ifndef DEBUT_DEBUT_H
#define DEBUT_DEBUT_H
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <ncurses.h>

typedef struct WindInfo {
    uint16_t maxx, maxy;
    uint16_t nRows, nCols;
    uint16_t leftpadding;
} WindInfo;

typedef struct Spread {
    WindInfo winf;
} Spread;

#endif
