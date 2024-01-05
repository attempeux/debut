#include "lexer.h"
#include <ncurses.h>

#define IS_IT_BCKSP(c)  ((c == KEY_BACKSPACE) || (c == 127) || (c == '\b'))
#define IS_IT_ENTER(c)  ((c == KEY_ENTER) || (c == '\r') || (c == '\n'))

static void init_grid (void);
static void print_labels (Grid*);

static void print_coords (Grid*);
static uint16_t count_digits (uint32_t);

static void get_column_name (char*, const uint32_t);
static void start_moving (Spread*);

static void check_bounds (Grid*, const uint32_t);
static void update_formula (const Grid*, const Cell*, const uint32_t);

static Cell* update_cell (const Spread*, const Grid*);
static void evaluate_cell (const Spread*, Cell*);

int main___ (void)
{
    Spread spread = {0};
    init_grid();

    getmaxyx(curscr, spread.grid.nYbytes, spread.grid.nXbytes);
    print_labels(&spread.grid);
    print_coords(&spread.grid);

    spread.cells = (Cell*) calloc(spread.grid.ncolumns * spread.grid.nrows, sizeof(Cell));
    assert(spread.cells && "no memory enough to proceed.");

    start_moving(&spread);
    getch();
    endwin();
    return 0;
}

static void init_grid (void)
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

static void print_labels (Grid* grid)
{
    /* The spreadsheet looks like this:
     * CC: <pos where you are>
     * FM: <formula of the cell>
     *          <command line>
     * ...   A    B
     * 0    ...  ...
     * 1    ...  ...
     * ER: <error messages>
     * */
    const uint32_t padd = grid->nXbytes - 4;

    mvprintw(0, 0, "CC: A0%-*.s", padd, " ");
    mvprintw(1, 0, "FM: %-*.s", padd, " ");
    mvprintw(2, 0, "%*c", grid->nXbytes, ' ');
    mvprintw(grid->nYbytes - 1, 0, "ER: :)%-*.s", padd, " ");
}

static void print_coords (Grid* grid)
{
    /* No magic numbers:
     * grid->nYbytes - 5, 5 since that is the number of rows used to display other kind of info.
     * grid->ncolumns < 650: 650 since that is the number you get when 26 * 25; that is the max number of columns.
     * other numbers in move or mvprintw are used meaning the position of some label.
     * */
    grid->nrows = grid->nYbytes - 5;
    grid->left_padding = count_digits(grid->nYbytes) + 1;

    mvprintw(3, 0, "%*c", grid->left_padding, ' ');
    uint32_t ypos = 4, i;

    for (i = 0; i < grid->nrows; i++)
        mvprintw(ypos++, 0, "%*d", grid->left_padding, i);

    move(3, grid->left_padding);
    grid->ncolumns = (grid->nXbytes - grid->left_padding) / DEBUT_CELL_WIDTH;

    assert(grid->ncolumns < 650 && "no supported number of columns");
    char columname[2] = {0};

    for (i = 0; i < grid->ncolumns; i++) {
        get_column_name(columname, i);
        printw("    %c%c    ", columname[0], columname[1]);
    }
}

static uint16_t count_digits (uint32_t n)
{
    uint16_t a = 1;
    while (n >= 10) { n *= .1; a++; }
    return a;
}

static void get_column_name (char* name, const uint32_t at)
{
    if (at < 26) {
        name[0] = 'A' + at;
        name[1] = ' ';
        return;
    }

    /* 25 are the number of letters in the alphabet
     * so do not think this is magic number it only
     * computes the number of times that 'at' have
     * completed the alphabet, cannot divide by 25
     * since it will skip Z.
     * */
    const uint32_t nth_time = at / 26;
    name[0] = nth_time + 'A' - 1;
    name[1] = 'A' + (at - 26 * nth_time);
}

static void start_moving (Spread* spread)
{
    Grid* grid = &spread->grid;
    move(4, grid->left_padding);

    keypad(stdscr, TRUE);
    curs_set(2);

    /* FM: <Bsleft>xxx|
     *     4th byte  ` Stops when there's 3 bytes left.
     *      ` Starts printing here.
     * */
    const uint32_t Bsleft = grid->nXbytes - 7;

    Cell* cc   = &spread->cells[0];
    uint32_t K = 0;

    while ((K = getch()) != KEY_F(1)) {
        if (IS_IT_ENTER(K)) {
            evaluate_cell(spread, cc);
            mvprintw(spread->grid.nYbytes - 1, 4, "%-*.*s", Bsleft, Bsleft, (!cc->type) ? cc->as_error : ":)"); // XXX: THIS WILL BE REPLACED BY A STACK.
        }

        else if ((K == KEY_UP) || (K == KEY_DOWN) || (K == KEY_LEFT) || (K == KEY_RIGHT)) {
            check_bounds(grid, K);
            cc = update_cell(spread, grid);
        }

        else if (isprint(K) && (cc->nth_fx_ch < DEBUT_CELL_FORMULA_LEN))
            cc->as_formula[cc->nth_fx_ch++] = K;

        else if (IS_IT_BCKSP(K) && cc->nth_fx_ch)
            cc->as_formula[--cc->nth_fx_ch] = 0;

        update_formula(grid, cc, Bsleft);
    }
}

static void check_bounds (Grid* grid, const uint32_t K)
{
    if ((K == KEY_UP) && (grid->c_row))   grid->c_row--;
    else if ((K == KEY_LEFT) && (grid->c_col)) grid->c_col--;

    else if ((K == KEY_DOWN) && (grid->c_row < grid->nrows - 1)) grid->c_row++;
    else if ((K == KEY_RIGHT) && (grid->c_col < grid->ncolumns - 1)) grid->c_col++;
}

/* Every time content is added or deleted from the current cell
 * the formula label must be updated.
 * */
static void update_formula (const Grid* grid, const Cell* cc, const uint32_t left)
{
    mvprintw(DEBUT_WRT_FM_AT, "%-*.*s", left, left, cc->as_formula);
    move(4 + grid->c_row, grid->left_padding + DEBUT_CELL_WIDTH * grid->c_col);
}

/* Once any arrow key is pressed and the new position
 * is within the bounds this function will display the
 * position where the user is and will return the new
 * cell.
 * */
static Cell* update_cell (const Spread* spread, const Grid* grid)
{
    char name[3] = {0};
    get_column_name(name, grid->c_col);
    mvprintw(DEBUT_WRT_CC_AT, "%s%d ", name, grid->c_row);
    return &spread->cells[grid->c_row * grid->ncolumns + grid->c_col];
}

/* Once the cell has been completed and <enter> is
 * pressed this function is called and all the lexer
 * and parser stuff will be done.
 * */
static void evaluate_cell (const Spread* spread, Cell* cc)
{
    static const uint32_t printwidth = DEBUT_CELL_WIDTH - 1,
                          maxndigits = printwidth - 5;

    attron(COLOR_PAIR(2));
    lexer_lex(spread, cc);

    if (cc->type == cell_is_numb)
        printw("%*.*Lg", printwidth, maxndigits, cc->as.number);
    else
        printw("%-*.*s", printwidth, printwidth, cc->as.text);

    attron(COLOR_PAIR(1));
}
