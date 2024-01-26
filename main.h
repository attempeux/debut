/* File created by Attempeux on Jan 24 2024.
 * main.c header file. */
#ifndef TERMSP_TERMSP_H
#define TERMSP_TERMSP_H
#include "parser.h"

#include <ncurses.h>
#include <getopt.h>
#include <string.h>

/* The maxium of characters for a cell name is five since
 * the maximum row is 199 and the maximum column is AZ giving
 * AZ199 as maximum.
 * */
#define     m_cellname_maxlength  5
#define     m_fatal_error(s, ...)     do { fprintf(stderr, "termsp: fatal: " s, ##__VA_ARGS__); exit(1); } while (0)

/*  Defines available types supported by tersmp so far. */
typedef enum SheetType {
    sheet_is_trms_one,
    sheet_is_xlsx_one
} SheetType;

/* Contains information about the window and the screen. */
typedef struct winscr_info {
    uint32_t final_y, final_x;
    uint32_t current_y, current_x;
    uint32_t display_n_rows, display_n_cols;
    char current_name[m_cellname_maxlength];
} WinScrInformation;

/* Contains information about the file given to read/write. */
typedef struct sheet_info {
    char* sheetname;
    size_t sheetname_length;
    SheetType type;
} SheetInformation;

typedef struct Termsp {
    SheetInformation sheet_info;
    WinScrInformation winscr_info;
    WINDOW* panel;
} Termsp;

#endif
