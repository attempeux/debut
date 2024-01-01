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

typedef struct Source {
    char* src;
    size_t len;
} Source;

typedef struct Spread {
    Source src;
    char* filename_r, *filename_w;
    bool ui_mode;
} Spread;

#endif
