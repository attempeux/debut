#include "debut.h"
#include <ncurses.h>

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

#define DEBUT_MAIN_CELL_WIDTH           10
#define DEBUT_MAIN_IS_ENTER(a)          ((a == KEY_ENTER) || (a == '\r') || (a == '\n'))
#define DEBUT_MAIN_IS_BCKSPC(a)         ((a == KEY_BACKSPACE) || (a == '\b') || (a == 127))
#define DEBUT_MAIN_IS_ARROW_KEY(a)      ((a == KEY_UP) || (a == KEY_DOWN) || (a == KEY_LEFT) || (a == KEY_RIGHT))

static void init_window (void);
static void refresh_labels (WindInfo*);

static void refresh_grid (WindInfo*);
static uint16_t number_of_digits (uint16_t);

static void get_column_name (char*, const uint16_t);
static void start (Spread*);

static void move_cursor_to_current_cell (const WindInfo*);
static void update_cells_capacity (Spread*);

static Cell* is_it_within_the_bounds (const Spread*, WindInfo*, const uint32_t);

int _main (void)
{
    Spread spread = {0};
    init_window();

    refresh_labels(&spread.winf);
    start(&spread);

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

/* This function is called when the program is started
 * and when the window is resized, it prints once again
 * all the layout to fit with the new window size.
 * */
static void refresh_labels (WindInfo* winf)
{
    clear();
    getmaxyx(curscr, winf->maxy, winf->maxx);
    mvprintw(0, 0, "A0 (E): %*c", winf->maxx - 8, ' ');
    mvprintw(1, 0, "%*c", winf->maxx, ' ');
    mvprintw(2, 0, "%*c", winf->maxx, ' ');
    mvprintw(winf->maxy - 1, 0, "!ERRS: %*c", winf->maxx - 7, ' ');

    refresh_grid(winf);
    refresh();
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

static void start (Spread* spread)
{
    move_cursor_to_current_cell(&spread->winf);
    keypad(stdscr, TRUE);
    curs_set(2);

    update_cells_capacity(spread);

    Cell* cuC = &spread->cells[0];
    uint32_t keypressed;

    while ((keypressed = wgetch(stdscr)) != KEY_F(1)) {
        if (keypressed == KEY_RESIZE) {
            refresh_labels(&spread->winf);
            move_cursor_to_current_cell(&spread->winf);
            update_cells_capacity(spread);
        }
        else if (DEBUT_MAIN_IS_ENTER(keypressed)) {
            fprintf(stderr, "content: (%d, %d): %s\n", spread->winf.cur_row, spread->winf.cur_col, cuC->fx_txt);
        }
        else if (DEBUT_MAIN_IS_BCKSPC(keypressed) && cuC->fxch) {
            cuC->fx_txt[--cuC->fxch] = 0;
        }
        else if (isprint(keypressed) && cuC->fxch < DEBUT_FORMULA_LENGTH) {
            cuC->fx_txt[cuC->fxch++] = keypressed;
        }
        else if (DEBUT_MAIN_IS_ARROW_KEY(keypressed)) {
            cuC = is_it_within_the_bounds(spread, &spread->winf, keypressed);
        }

        mvprintw(1, 0, "%-*.*s", spread->winf.maxx, spread->winf.maxx, cuC->fx_txt);
        move_cursor_to_current_cell(&spread->winf);
    }
}

static void move_cursor_to_current_cell (const WindInfo* winf)
{
    move(DEBUT_MAIN_UNUSED_ROWS - 1 + winf->cur_row, winf->leftpadding + DEBUT_MAIN_CELL_WIDTH * winf->cur_col);
}

static void update_cells_capacity (Spread* spread)
{
    static uint32_t prevnCells = 0;
    uint32_t currnCells = spread->winf.nRows * spread->winf.nCols;

    if (currnCells < prevnCells)
        return;

    if (!spread->cells) {
        spread->cells = (Cell*) calloc(currnCells, sizeof(Cell));
    }

    else if (!(spread->cells = (Cell*) realloc(spread->cells, currnCells))) {
        endwin();
        fprintf(stderr, "debut: error: no memory enough to hold %d cells :( use excel instead...\n", currnCells);
        exit(EXIT_FAILURE);
    }
    prevnCells = currnCells;
}

static Cell* is_it_within_the_bounds (const Spread* spread, WindInfo* winf, const uint32_t kp)
{
    const uint16_t rowbound = winf->nRows - 1;
    const uint16_t colbound = winf->nCols - 1;

    switch (kp) {
        case KEY_LEFT:  winf->cur_col += (winf->cur_col) ? -1 : 0; break;
        case KEY_UP:    winf->cur_row += (winf->cur_row) ? -1 : 0; break;
        case KEY_DOWN:  winf->cur_row += (winf->cur_row < rowbound) ? 1 : 0; break;
        case KEY_RIGHT: winf->cur_col += (winf->cur_col < colbound) ? 1 : 0; break;
    }

    return &spread->cells[winf->cur_row * winf->nCols + winf->cur_col];
}



