/* Take a look to:
 * imaxdiv
 * remainder
 * */
#include "lexer.h"
#include <ncurses.h>
#include <getopt.h>

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
#define DEBUT_MAIN_CELL_PRINT_WIDTH     9
#define DEBUT_MAIN_IS_ENTER(a)          ((a == KEY_ENTER) || (a == '\r') || (a == '\n'))
#define DEBUT_MAIN_IS_BCKSPC(a)         ((a == KEY_BACKSPACE) || (a == '\b') || (a == 127))
#define DEBUT_MAIN_IS_ARROW_KEY(a)      ((a == KEY_UP) || (a == KEY_DOWN) || (a == KEY_LEFT) || (a == KEY_RIGHT))
#define DEBUT_ABORT(s, ...)             do { endwin(); fprintf(stderr, "debut: error: " s, ##__VA_ARGS__); exit(EXIT_FAILURE); } while (0)

static void init_window (void);
static void refresh_labels (WindInfo*);

static void refresh_grid (WindInfo*);
static uint16_t number_of_digits (uint16_t);

static void get_column_name (char*, const uint16_t);
static void start (Spread*);

static void move_cursor_to_current_cell (const WindInfo*);
static Cell* is_it_within_the_bounds (const Spread*, WindInfo*, const uint32_t);

static void display_current_cell_name (const WindInfo*);
static void display_content_on (const Cell*, const uint16_t, const bool);

int main (void)
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

static void refresh_labels (WindInfo* winf)
{
    clear();
    getmaxyx(curscr, winf->maxy, winf->maxx);
    mvprintw(0, 0, "A0: %*c", winf->maxx - 4, ' ');
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

    /* Some people just got a whole TV for their PC.
     * */
    if (winf->nCols >= DEBUT_MAIN_MAX_COLUMNS)
        DEBUT_ABORT("No gonna work with %d columns, use excel instead.\n", winf->nCols);

    char col[3] = {0};
    for (i = 0; i < winf->nCols; i++) {
        get_column_name(col, i);
        printw("    %-2s    ", col);
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
    WindInfo *cuWinf = &spread->winf;
    move_cursor_to_current_cell(cuWinf);
    keypad(stdscr, TRUE);
    curs_set(2);


    uint16_t numcells = cuWinf->nRows * cuWinf->nCols;
    spread->cells = (Cell*) calloc(numcells, sizeof(Cell));
    if (!spread->cells)
        DEBUT_ABORT("no capacity enough to allocate %d cells.\n", numcells);

    Cell* cuC = &spread->cells[0];
    uint32_t keypressed;

    while ((keypressed = wgetch(stdscr)) != KEY_F(1)) {
        if (keypressed == KEY_RESIZE)
            DEBUT_ABORT("no gonna lie, do not wanna do such thing.\n");

        else if (DEBUT_MAIN_IS_ENTER(keypressed)) {
            lexer_lex(spread, cuC);
            display_content_on(cuC, DEBUT_MAIN_CELL_PRINT_WIDTH, true);
            display_content_on(cuC, DEBUT_CELL_DATA_LENGTH, false);
        }   

        else if (DEBUT_MAIN_IS_BCKSPC(keypressed) && cuC->fxch) {
            cuC->fx_txt[--cuC->fxch] = 0;
        }

        else if (isprint(keypressed) && cuC->fxch < DEBUT_FORMULA_LENGTH) {
            cuC->fx_txt[cuC->fxch++] = keypressed;
        }

        else if (DEBUT_MAIN_IS_ARROW_KEY(keypressed)) {
            cuC = is_it_within_the_bounds(spread, cuWinf, keypressed);
            display_current_cell_name(cuWinf);
            display_content_on(cuC, DEBUT_CELL_DATA_LENGTH, false);
        }

        mvprintw(1, 0, "%-*.*s", cuWinf->maxx, cuWinf->maxx, cuC->fx_txt);
        move_cursor_to_current_cell(cuWinf);
    }
}

static void move_cursor_to_current_cell (const WindInfo* winf)
{
    move(DEBUT_MAIN_UNUSED_ROWS - 1 + winf->cur_row, winf->leftpadding + DEBUT_MAIN_CELL_WIDTH * winf->cur_col);
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

    const size_t cellat = (winf->cur_row * winf->nCols) + (winf->cur_col);
    return &spread->cells[cellat];
}

static void display_current_cell_name (const WindInfo* winf)
{
    char columname[3] = {0};
    get_column_name(columname, winf->cur_col);
    mvprintw(0, 0, "%s%d: ", columname, winf->cur_row);
}

static void display_content_on (const Cell* cuC, const uint16_t width, const bool oncell)
{
    if (!oncell) { move(0, 4); }
    else { attron(COLOR_PAIR(2)); }

    if (cuC->kind == cell_kind_number) printw("%-*.*LG", width, width - 5, cuC->as.number);
    else printw("%-*.*s", width, width, cuC->as.text);
    attron(COLOR_PAIR(1));
}
