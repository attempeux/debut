#include "lexer.h"

static void print_usage (const char*);
static void read_file (FileContent*, const char*);

static void build_from_this (Spread*);

int main (int argc, char** argv)
{
    if (argc == 1) print_usage(*argv);

    Spread spread;
    char opt;

    while ((opt = getopt(argc, argv, "p:o:u:h")) != -1) {
        switch (opt) {
            case 'o': spread.filename_w = optarg; break;
            case 'p': spread.filename_r = optarg; break;
            case 'u': spread.filename_r = optarg; spread.ui_mode = 1; break;
            case 'h': print_usage(*argv); break;
            default : print_usage(*argv);
        }
    }

    read_file(&spread.src, spread.filename_r);
    if (!spread.ui_mode) {
        build_from_this(&spread);
        lexer_cell_by_cell(&spread);
    }

    return EXIT_SUCCESS;
}

static void print_usage (const char* s)
{
    fprintf(stderr, "debut: usage: %s [-p|o|u <filename>] [-h]\n", s);
    fputs("-p: parse and solve the <filename> table only.\n", stderr);
    fputs("-o: save the result of the table in <filename>.\n", stderr);
    fputs("-u: display ui and load contents of <filename>.\n", stderr);
    fputs("-h: display this message\n", stderr);
    exit(EXIT_SUCCESS);
}

static void read_file (FileContent* src, const char* flname)
{
    FILE* file = fopen(flname, "r");
    if (!file) {
        fprintf(stderr, "debut: error: cannot load '%s' file; %s (%d).\n", flname, strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    src->len = ftell(file);
    fseek(file, 0, SEEK_SET);

    src->src = (char*) calloc(src->len, 1);
    const size_t read = fread(src->src, 1, src->len, file);

    if (read != src->len) {
        fprintf(stderr, "debut: error: could not read all the %ldB, %ldB read only.\n", src->len, read);
        exit(EXIT_FAILURE);
    }

    src->src[src->len - 1] = 0;
    fclose(file);
}

/* This function is used to know the number of rows
 * and cells made in a table when the -p arguement
 * is used, this is needed because the size of the
 * table could not be fixed.
 * */
static void build_from_this (Spread* spread)
{
    const size_t len = spread->src.len;
    for (size_t i = 0; i < len; i++) {
        const char a = spread->src.src[i];

        if (a == '\n') spread->info.max_rows++;
        else if (a == '|') spread->info.max_cells++;
    }

    spread->spread        = (Cell*) calloc(spread->info.max_cells, sizeof(Cell));
    spread->offsets.marks = (uint16_t*) calloc(spread->info.max_rows, sizeof(uint16_t));
    assert(spread->spread && spread->offsets.marks && "no memory enough");
}

