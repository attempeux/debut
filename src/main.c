#include "lexer.h"
#include <ncurses.h>

#define DEBUT_MOVE_KEY(a)       ((a == 'j') || (a == 'k') || (a == 'h') || (a == 'l'))
#define DEBUT_BCKSPC_KEY(a)     ((a == '\b') || (a == KEY_BACKSPACE))
#define DEBUT_ENTER_KEY(a)      ((a == '\n') || (a == '\r') || (a == KEY_ENTER))
#define DEBUT_MIN(a, b)         ((a) < (b) ? (a) : (b))
#define DEBUT_UNUSED_WROWS      3
#define DEBUT_UNUSED_WCOLS      4

static void init_debut (void);
static void init_labels (Spreadsheet*);

static void update_filename (const char*, size_t*);
static void print_cell_name (const WindowInfo*, const size_t);

static void get_column_name (char*, const uint16_t);
static void update_grid (WindowInfo*, const bool, const bool);

static void start_to_typing (Spreadsheet*);
static Cell* move_cursor_to_cur_cell (const Spreadsheet*, const WindowInfo*);

static void check_bounds (WindowInfo*, const uint32_t);
static void check_if_gotta_upd_grid (WindowInfo*, uint16_t*, const uint16_t, uint16_t*);

static bool getting_fx (Cell*, const uint32_t);
static void eval_cell_and_show (Cell*, const Spreadsheet*);

#ifndef LEXER_TEST
int main (void)
{
    Spreadsheet sp = {0};

    init_debut();
    init_labels(&sp);

    sp.win_info.displayrows = (sp.win_info.win_rows - DEBUT_UNUSED_WROWS);
    sp.win_info.displaycols = (sp.win_info.win_columns - DEBUT_UNUSED_WCOLS) / DEBUT_CELL_WIDTH;
    update_grid(&sp.win_info, true, true);

    sp.cells = (Cell*) calloc(DEBUT_TOTAL_CELLS, sizeof(Cell)); // maybe calloc no needed.
    if (!sp.cells) {
        endwin();
        fprintf(stderr, "debut: err: cannot hold %d cells; aborting process.\n", DEBUT_TOTAL_CELLS);
        exit(1);
    }

    start_to_typing(&sp);
    getch();
    endwin();
    return 0;
}

/* This function starts ncurses configuration and
 * sets the colors the program will be using.
 * */
static void init_debut (void)
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

/* This function gets information about the current
 * screen and draw the labels.
 * */
static void init_labels (Spreadsheet* sp)
{
    getmaxyx(stdscr, sp->win_info.win_rows, sp->win_info.win_columns);

    if (!sp->filename) sp->filename = "no name";
    sp->flnamesz = strlen(sp->filename);

    update_filename(sp->filename, &sp->flnamesz);
    print_cell_name(&sp->win_info, sp->flnamesz);
    mvprintw(1, 0, "%*c", sp->win_info.win_columns + DEBUT_UNUSED_WCOLS, ' ');
}

/* Since the name can be set throughout the use of the program
 * there must be a function to update it.
 * */
static void update_filename (const char* filename, size_t* flnamesz)
{
    *flnamesz = strlen(filename);
    mvprintw(0, 0, "[%s]: ", filename);
}

/* Whenever the user changes its position the cell name must be updated
 * to let them know where they are!
 * */
static void print_cell_name (const WindowInfo* win_inf, const size_t flnamesz)
{
    char colname[3] = {0};
    get_column_name(colname, win_inf->cur_col);

    const uint16_t nbytes_2_rigth = (uint16_t) flnamesz + 4;
    mvprintw(0, nbytes_2_rigth, "%s%d: %*c", colname, win_inf->cur_row, win_inf->win_columns, ' ');
}

/* Twenty-six is the number of letters in the American alphabet,
 * this function calculates the name of the cell in the column 'a'.
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

/* Whenever the user reaches the bounds of the screen but there's columns or rows
 * left to be used then the program gotta update the grid to show the coordinates of
 * the current set of cells.
 * */
static void update_grid (WindowInfo* win_inf, const bool _rows, const bool _cols)
{
    uint16_t hilfsvar, i;

    if (_cols) {
        move(DEBUT_UNUSED_WROWS - 1, DEBUT_UNUSED_WCOLS);
        hilfsvar = win_inf->col_offset + win_inf->displaycols;

        for (i = win_inf->col_offset; i < hilfsvar; i++) {
            char name[3] = {0};
            get_column_name(name, i);
            printw("    %2s    ", name);
        }
    }

    if (_rows) {
        uint16_t w_row_pos = DEBUT_UNUSED_WROWS;
        hilfsvar = win_inf->row_offset + win_inf->displayrows;

        for (i = win_inf->row_offset; i < hilfsvar; i++)
            mvprintw(w_row_pos++, 0, "%-*d", DEBUT_UNUSED_WCOLS, i);
    }
}

/* Starts the process to let the user interact with the spreadsheet.
 * */
static void start_to_typing (Spreadsheet* sp)
{
    WindowInfo* win_inf = &sp->win_info;
    Cell* ths_cell = move_cursor_to_cur_cell(sp, win_inf);

    uint32_t key;
    keypad(stdscr, TRUE);

    const uint32_t bytes_available_for_fx = DEBUT_MIN(DEBUT_CELL_TEXT_CAP, (win_inf->win_columns - 4));
    bool insert_mode = false;

    while ((key = getch()) != KEY_F(1)) {
        if (DEBUT_MOVE_KEY(key) && !insert_mode) {
            check_bounds(win_inf, key);
            print_cell_name(win_inf, sp->flnamesz);
            ths_cell = move_cursor_to_cur_cell(sp, win_inf);
        }

        else if (key == 'i' && !insert_mode) {
            insert_mode = true;
            mvprintw(1, 0, "fx: %-*.*s", ths_cell->fx_ntch, bytes_available_for_fx, ths_cell->formula_txt);
        }

        else if (insert_mode) {
            insert_mode = getting_fx(ths_cell, key);
            if (insert_mode)
                mvprintw(1, 4, "%-*.*s", ths_cell->fx_ntch, bytes_available_for_fx, ths_cell->formula_txt);
            else {
                eval_cell_and_show(ths_cell, sp);
                mvprintw(1, 0, "%*c", win_inf->win_columns, ' ');
                (void) move_cursor_to_cur_cell(sp, win_inf);
            }
        }
    }
}

/* Sets the cursor to the current position within the grid and returns
 * the cell at such position.
 * */
static Cell* move_cursor_to_cur_cell (const Spreadsheet* sp, const WindowInfo* win_inf)
{
    move(DEBUT_UNUSED_WROWS + win_inf->rel_row_pos, DEBUT_UNUSED_WCOLS + win_inf->rel_col_pos * DEBUT_CELL_WIDTH);
    return &sp->cells[win_inf->cur_row * DEBUT_MAX_COLS + win_inf->cur_col];
}

/* This functions checks if the position where the user wanna go is within the bounds
 * and sets the values of the real cursor position and the relative ones.
 *
 * The real cursor position is the position within the whole table it means that:
 *      -1 < win_inf->cur_col < DEBUT_MAX_COLS
 *      -1 < win_inf->cur_row < DEBUT_MAX_ROWS
 *
 * And the relative one is where the cursor is but within the current grid:
 *      -1 < win_inf->rel_col_pos < winf->displaycols
 *      -1 < win_inf->rel_row_pos < winf->displayrows
 *
 * The relative position is useful to know whenever the program have to refresh
 * the layout.
 *
 * note: there must be a better way to write this but i haven't found it yet.
 * */
static void check_bounds (WindowInfo* win_inf, const uint32_t key)
{
    switch (key) {
        case 'h':
            if (!win_inf->cur_col) goto boundissue;
            win_inf->rel_col_pos -= (win_inf->rel_col_pos) ? 1 : 0;
            win_inf->cur_col--;

            check_if_gotta_upd_grid(win_inf, &win_inf->rel_col_pos, win_inf->displaycols, &win_inf->col_offset);
            break;

        case 'l':
            if (win_inf->cur_col == (DEBUT_MAX_COLS - 1)) goto boundissue;
            win_inf->rel_col_pos += (win_inf->rel_col_pos != win_inf->displaycols) ? 1 : 0;
            win_inf->cur_col++;

            check_if_gotta_upd_grid(win_inf, &win_inf->rel_col_pos, win_inf->displaycols, &win_inf->col_offset);
            break;

        case 'k':
            if (!win_inf->cur_row) goto boundissue;
            win_inf->rel_row_pos -= (win_inf->rel_row_pos) ? 1 : 0;
            win_inf->cur_row--;

            check_if_gotta_upd_grid(win_inf, &win_inf->rel_row_pos, win_inf->displayrows, &win_inf->row_offset);
            break;

        case 'j':
            if (win_inf->cur_row == (DEBUT_MAX_ROWS - 1)) goto boundissue;
            win_inf->rel_row_pos += (win_inf->rel_row_pos != win_inf->displayrows) ? 1 : 0;
            win_inf->cur_row++;

            check_if_gotta_upd_grid(win_inf, &win_inf->rel_row_pos, win_inf->displayrows, &win_inf->row_offset);
            break;
    }

    return; boundissue: beep();
}

/* Detects if it is need update the grid whichever row one or column one, it also modifies relative
 * value position since we need to undo the last increment/decrement made on 'check_bounds'.
 *      
 *      |   A   |   B   |
 *  1   |       |       |
 *  2   |       |       |
 *  3   |       |       |
 *          ->      ->    ~
 *                        here would be win_inf->rel_col_pos, since it is eq to max (win_inf->displaycols)
 *                        we have to update the grid for columns, since we update the whole the new columns
 *                        the new grid would look like:
 *      |   B   |   C   |
 *  1   |       |       |
 *  2   |       |       |
 *  3   |       |       |
 *                        ~ and win_inf->rel_col_pos would be here, but that is not what we want, we want to be on
 *                        C column, so we need to decrease win_inf->rel_col_pos to make sure the position is well
 *                        defined.
 * */
static void check_if_gotta_upd_grid (WindowInfo* win_inf, uint16_t *rel, const uint16_t max, uint16_t* offset)
{
    const bool forrow = &win_inf->rel_row_pos == rel;

    if (!*rel && *offset) { *rel += 1; *offset -= 1; update_grid(win_inf, forrow, !forrow); }
    else if (*rel == max) { *rel -= 1; *offset += 1; update_grid(win_inf, forrow, !forrow); }
}

/* Gets the characters typed by the user as he/she builds the formula
 * for the current cell.
 * */
static bool getting_fx (Cell* ths_cell, const uint32_t key)
{
    if (DEBUT_ENTER_KEY(key))
        return false;

    if (isprint(key) && ths_cell->fx_ntch < DEBUT_CELL_TEXT_CAP) {
        ths_cell->formula_txt[ths_cell->fx_ntch++] = key;
        return true;
    }

    if (DEBUT_BCKSPC_KEY(key) && ths_cell->fx_ntch) {
        ths_cell->formula_txt[--ths_cell->fx_ntch] = 0;
        return true;
    }

    if ((ths_cell->fx_ntch == DEBUT_CELL_TEXT_CAP) || !ths_cell->fx_ntch) {
        beep();
        return true;
    }

    return false;
}

static void eval_cell_and_show (Cell* ths_cell, const Spreadsheet* sp)
{
    lexer_lexer(sp, ths_cell);
}
#endif
