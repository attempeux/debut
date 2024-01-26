/* File created by Attempeux on Jan 24 2024.
 * termsp's starting point. */
#define m_todo(s)               fprintf(stderr, "TODO: %s (%s, %d)\n", s, __FILE__, __LINE__);

/* Stands for: color pallete for labels macro.
 * This is the number associated with the pair of colors
 * to color the labels: first, second, third and last rows
 * and the column of row numbers. */
#define m_cp_labels             1

/* Stands for: number of bytes no used since they are used
 * to print the row numbers macro. */
#define m_unused_Xs             5

#define m_unused_rows           4
#define m_cell_width            10

#include "main.h"

static void get_args (Termsp*, const uint32_t, char**);
static void print_usage (const char*);

static void init_ncurses (void);
static void init_layout (WinScrInformation*, SheetInformation*, WINDOW*);

static void indicate_current_cell_name (WinScrInformation*);
static uint16_t get_column_name (char*, const uint32_t);

int main (int argc, char** argv)
{
    if (argc == 1) print_usage(*argv);

    Termsp termsp = {0};
    get_args(&termsp, argc, argv);
    init_ncurses();
    init_layout(&termsp.winscr_info, &termsp.sheet_info, termsp.panel);

    getch();
    endwin();
    return 0;
}

static void get_args (Termsp* termsp, const uint32_t nargs, char** vargs)
{
    int32_t op;
    while ((op = getopt(nargs, vargs, "s:")) != -1) {
        switch (op) {
            case 's': termsp->sheet_info.sheetname = optarg; break;
            case '?': case 'h': default:
                print_usage(*vargs);
        }
    }

    if (!termsp->sheet_info.sheetname) m_fatal_error("cannot proceed, no file given.\n");
}

static void print_usage (const char* as)
{
    fprintf(stderr, "usage: %s [-s <sheet>] [-h].\n", as);
    fprintf(stderr, "-s: Spreadsheet to be used.\n");
    fprintf(stderr, "-h: Print this message.\n");
    exit(0);
}

static void init_ncurses (void)
{
    initscr();
    cbreak();
    noecho();
    start_color();

    /* Pair for labels. */
    init_color(COLOR_BLACK, 0, 0, 0);
    init_color(COLOR_RED, 890, 420, 470);
    init_pair(m_cp_labels, COLOR_BLACK, COLOR_RED);

    attron(COLOR_PAIR(m_cp_labels));
}

static void init_layout (WinScrInformation* winscr_info, SheetInformation* sheet_info, WINDOW* panel)
{
    getmaxyx(curscr, winscr_info->final_y, winscr_info->final_x);

    indicate_current_cell_name(winscr_info);
    mvprintw(1, 0, "%*c", winscr_info->final_x, ' ');

    /* offseterr_message: Since errors does not stop the program unless they are about
     * memory stuff, we need a place where to write the error messages, such place is at
     * the bottom part of the window next to the name of the current sheet, the name of
     * such file usses bytes we cannot longer use to print the error message so we need to
     * know how many bytes they are. */
    sheet_info->sheetname_length = winscr_info->final_x - strlen(sheet_info->sheetname);
    mvprintw(winscr_info->final_y - 1, 0, "[%s]: %*c", sheet_info->sheetname, (uint32_t) sheet_info->sheetname_length, ' ');


    const uint32_t available_rows = winscr_info->final_y - m_unused_rows;
    const uint32_t available_cols = winscr_info->final_x - m_unused_Xs;

    /* Panel is gonna be where text numbers and data in short are gonna be displayed, it needs
     * to be a pad since whenever the user walk throughout all the sheet the information of all
     * those cells needs to be displayed at once and we do not want to print n_rows * n_cols any
     * time the user moves. */
    m_todo("give correct number of bytes!");
    m_todo("https://docs.oracle.com/cd/E36784_01/html/E36880/newpad-3curses.html#scrolltoc");

    panel = newpad(100, 10);
    /* renderNcols: this variable is used to know how many columns can be showed
     * at the same time depending on the size of the winddow and the offset given
     * the column of row numbers. renderNrows it is the same but for rows.
     * +-----------------------------+
     * |A0: fx:                      |
     * |_____________________________|
     * |    .    A     .    B      . | Only two columns can be show at the time
     * |0   .          .           . | given the size of this window.
     * |1   .          .           . |
     * |2   .          .           . | The number of rows is always gonna be
     * |3   .          .           . | ymax - m_unused_rows since three rows are used to display
     * |[filename]: <errors>         | information rather than use them as cells.
     * +-----------------------------+ */
    winscr_info->display_n_cols = (available_cols) / m_cell_width;
    winscr_info->display_n_rows = available_rows;
}

static void indicate_current_cell_name (WinScrInformation* winscr_info)
{
    const uint16_t used = get_column_name(winscr_info->current_name, winscr_info->current_x);
    snprintf(winscr_info->current_name + used, m_cellname_maxlength - used, "%d", winscr_info->current_y);
    mvprintw(0, 0, "[%-*s] fx: %*c", m_cellname_maxlength, winscr_info->current_name, winscr_info->final_x - 12, ' ');
    m_todo("maybe if we overwirte only the position it will work (?)");
}

static uint16_t get_column_name (char* colpart, const uint32_t colnum)
{
    if (colnum < 26) {
        colpart[0] = 'A' + colnum;
        return 1;
    }

    const uint16_t nthtime = colnum / 26;
    colpart[0] = nthtime + 'A' - 1;
    colpart[1] = 'A' + (colnum - 26 * nthtime);
    return 2;
}
