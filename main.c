#include "debut.h"

/* There are five rows that are not being used:
 * 1. Cell information.
 * 2. Formula writing space.
 * 3. Command line.
 * 4. Row to display the column names.
 * 5. Error messages.
 * */
#define DEBUT_MAIN_UNUSED_ROWS  5

/* This is the number of columns that takes reach the ZZ column.
 * */
#define DEBUT_MAIN_MAX_COLUMNS  650
#define DEBUT_MAIN_CELL_WIDTH   10

static void init_window (void);
static void refresh_labels (WindInfo*);

static void refresh_grid (WindInfo*);
static uint16_t number_of_digits (uint16_t);

static void get_column_name (char*, const uint16_t);

int main (void)
{
    Spread spread = {0};
    init_window();

    refresh_labels(&spread.winf);

    getch();
    endwin();
    return 0;
}

static void init_window (void)
{
    initscr();
    cbreak();
    noecho();
    start_color();

    init_color(COLOR_BLACK, 0, 0, 0);
    init_color(COLOR_MAGENTA, 890, 420, 470);

    init_pair(1, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(2, COLOR_MAGENTA, COLOR_BLACK);

    attron(COLOR_PAIR(1));
}

static void refresh_labels (WindInfo* winf)
{
    getmaxyx(curscr, winf->maxy, winf->maxx);
    mvprintw(0, 0, "A0 (E): %*c", winf->maxx - 8, ' ');
    mvprintw(1, 0, "%*c", winf->maxx, ' ');
    mvprintw(2, 0, "%*c", winf->maxx, ' ');
    mvprintw(winf->maxy - 1, 0, "!ERRS: %*c", winf->maxx - 7, ' ');

    refresh_grid(winf);
}

static void refresh_grid (WindInfo* winf)
{        
    winf->nRows = winf->maxy - DEBUT_MAIN_UNUSED_ROWS;
    winf->leftpadding = number_of_digits(winf->nRows) + 1;

    mvprintw(3, 0, "%*c", winf->leftpadding, ' ');
    uint16_t print_at_y = 4, i;

    for (i = 0; i < winf->nRows; i++)
        mvprintw(print_at_y++, 0, "%-*d", winf->leftpadding, i);
    move(3, winf->leftpadding);

    winf->nCols = (winf->maxx - winf->leftpadding) / DEBUT_MAIN_CELL_WIDTH;
    if (winf->nCols >= DEBUT_MAIN_MAX_COLUMNS) {
        endwin();
        fprintf(stderr, "debut: error: No gonna work with %d columns, use excel instead.\n", winf->nCols);
        exit(EXIT_FAILURE);
    }

    char col[2];
    for (i = 0; i < winf->nCols; i++) {
        get_column_name(col, i);
        printw("    %c%c    ", col[0], (col[1]) ? col[1] : ' ');
    }
}

/* This function is used to know how many left padding use:
 * padding |    A    |    B    |
 * 0       |         |         |
 * 1       |         |         |
 * ...     |         |         |
 * nRows   |         |         |
 * */
static uint16_t number_of_digits (uint16_t a)
{
    uint16_t n = 1;
    while (a > 9) { a *= .1; n++; }
    return n;
}

/* Gets the name of the column based on the number of the column.
 * 26 is not a magic number, it is the number of letters in the alphabet.
 * */
static void get_column_name (char* name, const uint16_t a)
{
    if (a < 26) {
        name[0] = 'A' + a;
        return;
    }

    const uint16_t nthtime = a / 26;
    name[0] = nthtime + 'A' - 1;
    name[1] = 'A' + (a - 26 * nthtime);
}
