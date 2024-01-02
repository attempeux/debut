#include "debut.h"

#define IS_IT_BCKSP(c)  ((c == KEY_BACKSPACE) || (c == 127) || (c == '\b'))

static void init_grid (void);
static void print_labels (Grid*);

static void print_coords (Grid*);
static const uint16_t count_digits (uint32_t);

static void get_column_name (char*, const uint32_t);
static void start_moving (Spread*);

static void check_bounds (Grid*, const uint32_t);
static void update_formula (const Grid*, const Cell*);

int main (void)
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
    init_color(COLOR_MAGENTA, 218, 132, 245);

    init_pair(1, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(2, COLOR_MAGENTA, COLOR_BLACK);

    attron(COLOR_PAIR(1));
}

static void print_labels (Grid* grid)
{
    /* The spreadsheet looks like this:
     * CC: <pos where you are>
     * FM: <content of the cell>
     * < COMMAND LINE >             -> This is all white space
     *                                 until a command is excuted.
     * ...
     * ER: <error messages>
     * */
    const uint32_t padd = grid->nXbytes - 4;

    mvprintw(0, 0, "CC: A0%-*.s", padd, " ");
    mvprintw(1, 0, "FM: %-*.s", padd, " ");
    mvprintw(2, 0, "%-*.s", grid->nXbytes, " ");
    mvprintw(grid->nYbytes - 1, 0, "ER: %-*.s", padd, " ");
}

static void print_coords (Grid* grid)
{
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
        printw("      %c%c      ", columname[0], columname[1]);
    }
}

static uint16_t count_digits (uint32_t n)
{
    uint16_t a = 1;
    while (n >= 10) {
        n *= .1;
        a++;
    }

    return a;
}

static void get_column_name (char* name, const uint32_t at)
{
    if (at < 26) {
        name[0] = 'A' + at;
        name[1] = ' ';
        return;
    }

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

    Cell* cc   = &spread->cells[0];
    uint32_t K = 0;

    while ((K = getch()) != KEY_F(1)) {

        if ((K == KEY_UP) || (K == KEY_DOWN) || (K == KEY_LEFT) || (K == KEY_RIGHT)) {
            check_bounds(grid, K);
        }

        if (isprint(K) && (cc->nth_ch < DEBUT_CELL_LENGHT))
            cc->data[cc->nth_ch++] = K;

        if (IS_IT_BCKSP(K) && cc->nth_ch)
            cc->data[--cc->nth_ch] = 0;

        update_formula(grid, cc);
    }
}

static void check_bounds (Grid* grid, const uint32_t K)
{
    if ((K == KEY_UP) && (grid->c_row))   grid->c_row--;
    else if ((K == KEY_LEFT) && (grid->c_col)) grid->c_col--;

    else if ((K == KEY_DOWN) && (grid->c_row < grid->nrows - 1)) grid->c_row++;
    else if ((K == KEY_RIGHT) && (grid->c_col < grid->ncolumns - 1)) grid->c_col++;
    move(4 + grid->c_row, grid->left_padding + DEBUT_CELL_WIDTH * grid->c_col);
}

static void update_formula (const Grid* grid, const Cell* cc)
{
    fprintf(stderr, "getting '%s' for (%d %d)\n", cc->data, grid->c_row, grid->c_col);
}
