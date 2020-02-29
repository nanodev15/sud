#include <ctype.h>
#include <getopt.h>
#include <libgen.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <preemptive_set.h>
#include "sudoku.h"

typedef struct choice {
    pset_t **grid;
    size_t x;
    size_t y;
    pset_t choice;
    struct choice *previous;
} choice_t;

static bool generate = false;
static bool strict = false;
static bool verbose = false;
static size_t grid_size = 0;
static size_t iter = 0;
static char* file; // executable name
static FILE* output; // output file
static int MAX_GRID_SIZE = 64; // max grid size


//********************Usage fonctions********************

/*
 * usage fonction will display the help and exit if status 
 * is EXIT_SUCCESS else it will print an error message and exit
 */

static void usage(int status) {
    if (status == EXIT_SUCCESS) {
        fprintf(stdout, "Usage %s [OPTION] FILE...\n"
                "Solve Sudoku puzzle's of variables sizes (1-4).\n\n"
                "\t-o, --output=FILE\t\twrite result to file\n"
                "\t-v, --verbose\t\tverbose output\n"
                "\t-V, --version\t\tdisplay version and exit\n"
                "\t-h, --help\t\t\tdisplay this help\n\n", file);
    } else {
        fprintf(stderr, "Try '%s --help' for more information\n", file);
    }
    exit(status);
}

/*
 * version function will print the version in stdout and exit
 */

static void version(void) {
    fprintf(stdout, "%s %d.%d.%d\nThis software is a sudoku solver\n",
            file, VERSION, SUBVERSION, REVISION);
    exit(EXIT_SUCCESS);
}

//********************Print grid********************

/*
 * print the grid
 */

static void grid_print(pset_t **grid) {
    size_t i, j, p = 0;
    size_t card = 1;
    char tmp [grid_size + 1];
    for (i = 0; i < grid_size; i++) { //detect spacement
        for (j = 0; j < grid_size; j++) {
            card = card >= pset_cardinality(grid[i][j]) ? card :
                    pset_cardinality(grid[i][j]);
        }
    }

    for (i = 0; i < grid_size; i++) {
        for (j = 0; j < grid_size; j++) {
            if (grid[i][j] == 0) printf("%d", 0);
            else {
                pset2str(tmp, (grid [i] [j]));
                printf("%s ", tmp);
            }
            for (p = 0; p <= (card - pset_cardinality(grid[i][j])); p++) {
                fprintf(stdout, " ");
            }
        }
        fprintf(stdout, "\n");
    }
}

//********************Grid file parsing and utilities ********************

/*
 * Check a character in thegrid.
 * Allowed characters :
 * grid_size=1  : '1', '_'
 * grid_size=4  : '1-4', '_'
 * grid_size=9  : '1-9', '_'
 * grid_size=16  : 'A-G', '1-9', '_'
 * grid_size=25  : 'A-P', '1-9', '_'
 * grid_size=36  : 'a', 'A-Z', '1-9', '_'
 * grid_size=49  : 'a-n', 'A-Z', '1-9', '_'
 * grid_size=64  : '@', '&', '*', 'a-z', 'A-Z', '1-9', '_'
 *
 * Return value : true if the character is allowed
 *                false else
 */

static bool check_input_char(char c) {
    bool ret = false;
    switch (grid_size) {
        case 64:
            ret |= ((c >= 'o' && c <= 'z') || (c == '@') || (c == '*') ||
                    (c == '&'));

        case 49:
            ret |= ((c >= 'b' && c <= 'n'));

        case 36:
            ret |= ((c >= 'Q' && c <= 'Z') || (c == 'a'));

        case 25:
            ret |= (c >= 'H' && c <= 'P');

        case 16:
            ret |= (c >= 'A' && c <= 'G');

        case 9:
            ret |= (c >= '5' && c <= '9');

        case 4:
            ret |= (c >= '2' && c <= '4');

        case 1:
            ret |= ((c == '_') || (c == '1'));
    }

    return (ret);
}

/*
 * allocate space memory to stock the grid
 * grid_alloc() will return pointer on the first "cell"
 * grid_alloc need a grid_free(); call to free this space
 * exit if memory can't be allocated
 */

static pset_t ** grid_alloc(void) {
    pset_t ** grid = calloc(grid_size, sizeof ( pset_t*));
    size_t i = 0;
    if (grid == NULL) {
        fprintf(stderr, "sudoku: error: out of memory !");
        exit(EXIT_FAILURE);
    }
    //Allocation of lines
    for (i = 0; i < grid_size; i++) {
        grid[i] = calloc(grid_size, sizeof (pset_t));
        if (grid[i] == NULL) {
            fprintf(stderr, "sudoku: error: out of memory !");
            exit(EXIT_FAILURE);
        }
    }
    return grid;
}

/*
 * grid_free will free memory
 * pset_t ** grid is created with grid_alloc
 */

static void grid_free(pset_t **grid) {
    size_t i;
    for (i = 0; i < grid_size; i++)
        free(grid[i]);
    //Free subgrid with pointers
    free(grid);
}

/*
 * grid_parser will parse the grid given in the FILE *file,
 * affect a value to grid_size (depending on the file),
 * create a pset_t ** containing grid,
 * check each character given.
 *
 * EXIT if :
 * wrong number of lines or columns
 * wrong character given (see function check_input_char)
 * grid empty
 *
 *
 * Grid sizes allowed:
 *   1x1
 *   4x4
 *   9x9
 *   16x16
 *   25x25
 *   36x36
 *   49x49
 *   64x64
 */

static pset_t** grid_parser(FILE* file) {
    char first_line[MAX_GRID_SIZE];
    pset_t** grid = NULL;
    int current_char;
    unsigned int j = 0; //column counter
    int line = 1; //line counter
    size_t gridline = 0; //grid line counter
    bool begin = false; // showing the first line is already read
    bool start = false; // showing a significant character is read
    bool comment = false; // comments
    bool initialize = false;
    while ((current_char = fgetc(file)) != EOF) {
        switch (current_char) {
            case ' ': break;
            case '\t':
                break;
            case '#':
                comment = true;
                break;
            case '\n':
                comment = false;
                if (begin && gridline < grid_size - 1 && j != 0) {
                    if (j != grid_size) {
                        fprintf(stderr,
                                "sudoku: error: line %d is malformed. \n",
                                line);
                        exit(EXIT_FAILURE);
                    } else {
                        gridline++;

                        j = 0;
                        initialize = false;
                    }
                }
                if (start && !begin) {
                    if (j != 1 && j != 4 && j != 9 && j != 16 &&
                            j != 25 && j != 36 && j != 49 && j != 64) {
                        fprintf(stderr, "sudoku: error: line %d is "
                                "malformed. \n", line);
                        exit(EXIT_FAILURE);
                    }
                    begin = true;
                    grid_size = j;
                    grid = grid_alloc();
                    gridline++;
                    size_t k;
                    for (k = 0; k < grid_size; k++) { // first line
                        if (check_input_char(first_line[k]) == true) {
                            grid[0][k] = (first_line[k] == '_') ?
                                    pset_full(grid_size) :
                                    char2pset(first_line[k]);
                        } else {
                            fprintf(stderr,
                                    "sudoku: error: wrong character '%c' "
                                    "at line %d. \n",
                                    first_line[k], line);
                            exit(EXIT_FAILURE);
                        }
                    }
                    j = 0;
                }
                line++;
                break;

            default:
                if (comment) break;
                if (!begin) {
                    start = true;
                    first_line[j] = current_char;
                    j++;
                    break;
                }
                if (begin) {
                    if (j >= grid_size) {
                        fprintf(stderr, "sudoku: error: line  %d is "
                                "malformed. \n", line);
                        exit(EXIT_FAILURE);
                    }
                    if (gridline >= grid_size) {
                        fprintf(stderr, "sudoku: error: too many/few lines"
                                "in the grid. \n");
                        exit(EXIT_FAILURE);
                    }
                    if (check_input_char(current_char)) {
                        if (current_char == '_')
                            grid[gridline][j] = pset_full(grid_size);
                        else
                            grid[gridline][j] = char2pset(current_char);
                        j++;
                        initialize = true;
                        break;
                    } else {
                        fprintf(stderr, "sudoku: error: wrong character '%c' "
                                "at"
                                " line %d. \n ", current_char, line);
                        exit(EXIT_FAILURE);
                    }
                }
        }
    }

    if (!start || !begin) {
        if (j == 1) {
            grid_size = 1;
            if (check_input_char(first_line[0])) {
                grid = grid_alloc();
                grid[0][0] = (first_line[0] == '_') ? pset_full(grid_size) :
                        char2pset(first_line[0]);
            } else {
                fprintf(stderr, "sudoku: error: wrong character '%c' at"
                        " line %d. \n ", first_line[0], line);
                exit(EXIT_FAILURE);
            }

        } else {
            fprintf(stderr, "sudoku: error: too many/few lines"
                    " in the grid.\n");
            exit(EXIT_FAILURE);
        }

    }


    if (grid_size != 1) {
        if (!initialize || gridline != grid_size - 1) {
            fprintf(stderr, "sudoku: error: too many/few lines "
                    "in the grid.\n");
            exit(EXIT_FAILURE);
        }
        if (j != grid_size) {
            fprintf(stderr, "line %d is malformed\n", line);
            exit(EXIT_FAILURE);
        }
    }
    return grid;
}

//********************Grid resolution********************

/*
 * grid_solved check if grid is solved
 * RETURN VALUE : true if gris is solved, false otherwise
 */

static bool grid_solved(pset_t **grid) {
    size_t i, j;
    for (i = 0; i < grid_size; i++)
        for (j = 0; j < grid_size; j++)
            if (!pset_is_singleton(grid [i] [j]))
                return false;
    return true;
}

/*
 * subgrid_print print a subgrid
 * Ex: "subgrid : (0) = '1234', (1) = '1234', (2) = '2', (3) = '1234'"
 */

static bool subgrid_print(pset_t * subgrid[]) {
    char tmp [grid_size + 1];
    size_t i;
    for (i = 0; i < grid_size; i++) {
        pset2str(tmp, *subgrid [i]);
        fprintf(stdout, "subgrid: (%zu) ='%s'", i, tmp);
        if (i < grid_size - 1)
            fprintf(stdout, ", ");
        else
            fprintf(stdout, "\n");
    }
    return true;
}

/*
 * subgrid_consistency check if subgrid is consistent
 *
 * a subgrid is consistent if and only if :
 * _ each color appears once at least in subgrid
 * _ two singleton in subgrid are not the same
 *
 * RETURN VALUE : true if subgrid is consistent, false otherwise
 */

static bool subgrid_consistency(pset_t *subgrid [grid_size]) {

    bool one_singleton = true;
    pset_t color_app = pset_empty();
    pset_t color_singleton = pset_empty();
    for (size_t i = 0; i < grid_size; i++) {
        if (*subgrid[i] == pset_empty()) return false;
        if (pset_is_singleton(*subgrid[i])) {
            if (pset_is_included(*subgrid[i], color_singleton)) {
                one_singleton = false;
                break;
            }
            color_singleton = pset_or(color_singleton, *subgrid[i]);
        }
        color_app = pset_or(color_app, *subgrid[i]);
    }
    return one_singleton && pset_is_included(pset_full(grid_size), color_app);
}

/*
 *
 * subgrid_map generates every subgrids contained in grid and applies (func)
 * given to each subgrid
 *
 * func is a pointer of function whose prototype is :
 *     bool func (pset_t *subgrid[grid_size])
 *
 * RETURN VALUE :
 * true if each call to func returns true
 * false otherwise
 */

static bool subgrid_map(pset_t **grid,bool(*func)(pset_t *subgrid[grid_size])){
    bool value = true;
    size_t n = sqrt(grid_size);
    size_t compteur = 0;
    pset_t * subgrid[grid_size];
    size_t i, j, k;

    // scanning  rows
    for (i = 0; i < grid_size; i++) {
        for (j = 0; j < grid_size; j++) {
            subgrid[j] = &grid[i][j];
        }
        value = value && func(subgrid);
    }
    // Scanning columns
    for (i = 0; i < grid_size; i++) {
        for (j = 0; j < grid_size; j++) {
            subgrid [j] = &grid[j][i];
        }
        value = value && func(subgrid);
    }

    // scanning blocks
    for (i = 0; i < grid_size; i++) {
        compteur = 0;
        for (j = (size_t) (i / n) * n; j < (size_t) (i / n) * n + n; j++) {
            for (k = (i % n) * n; k < (i % n) * n + n; k++) {
                subgrid [compteur] = &grid[j][k];
                compteur++;
            }
        }
        value = value && func(subgrid);

    }
    return value;
}

/*
 * grid_consistency check if grid is consistency
 *
 * a grid is consistent if and only if each subgrid is consistent
 *
 * a subgrid is consistent if and only if :
 * _ each color appears once at least in subgrid
 * _ two singleton in subgrid are not the same
 *
 * RETURN VALUE : true if grid is consistent, false otherwise
 */

static bool grid_consistency(pset_t **grid) {
    return subgrid_map(grid, subgrid_consistency);
}

/*
 * subgrid applies different heuristics to subgrid :
 * _ 'cross-hatching' : discard, in not-singleton psets, colors present as
 *   singleton in subgrid
 *   {1234, 2, 123, 14} -> {134, 2, 13, 14}
 * _ 'lone number' : if a color is present just once is subgrid,
 *  the pset having this color is changed in a singleton with this color
 *   {123, 2, 123, 124} -> {123, 2, 123, 12}
 * _'naked subset': f there are N cells in one unit that contain N
 * candidates between them, then the candidates can be removed from all the
 * other cells in the unit
 * RETURN VALUE : false if the subgrid is modified, true otherwise
 */

static bool subgrid_heuristics(pset_t* subgrid[]) {

    bool subfixpoint = true;
    size_t number;
    pset_t subset = pset_empty();
    pset_t colors_met_once, colors_met_more;
    colors_met_once = pset_empty();
    colors_met_more = pset_empty();

    //lone number heuristic
    for (size_t i = 0; i < grid_size; i++) {
        colors_met_more = pset_or(colors_met_more, pset_and(colors_met_once,
                *subgrid[i]));
        colors_met_once = pset_or(colors_met_once, *subgrid[i]);
    }

    colors_met_once = pset_xor(colors_met_once, colors_met_more);

    for (size_t i = 0; i < grid_size; i++) {
        if (!pset_is_included(*subgrid[i], colors_met_more)) {
            //check if subgrid[i] will be changed
            if (!pset_is_singleton(*subgrid[i])
                    && !pset_is_included(*subgrid[i], colors_met_once)) {
                subfixpoint = false;
                *subgrid[i] = pset_and(colors_met_once, *subgrid[i]);
                if (pset_is_singleton(*subgrid[i]))
                    colors_met_once = pset_or(colors_met_once, *subgrid[i]);
            }

        }
    }

    // cross-hatching heuristic
    for (size_t i = 0; i < grid_size; i++) {
        if (pset_is_singleton(*subgrid[i])) {
            subset = pset_or(subset, *subgrid[i]);
        }
    }

    for (size_t i = 0; i < grid_size; i++) {
        if (!pset_is_singleton(*subgrid[i])) {
            if (!pset_is_included(*subgrid[i], pset_and(pset_negate(subset),
                    *subgrid[i]))) {
                subfixpoint = false;
                *subgrid[i] = pset_and(pset_negate(subset), *subgrid[i]);
            }
        }
    }

    // Naked subset

    size_t size;
    for (size_t i = 0; i < grid_size; i++) {
        if (!pset_is_singleton(*subgrid [i])) {
            size = pset_cardinality(*subgrid [i]);
            number = 1;
            for (size_t j = 0; j < grid_size; j++) {
                if (j == i) continue;
                if ((pset_discard2(*subgrid[i], *subgrid[j])) == pset_empty()) {
                    number++;
                    if (number > size) break;
                }

            }
            if (number == size) {
                for (size_t j = 0; j < grid_size; j++) {
                    if (!pset_is_singleton(*subgrid [j]) &&
                            pset_and(*subgrid [i], *subgrid [j])
                            != pset_empty()
                            && !pset_is_included(*subgrid [j], *subgrid [i])) {

                        subfixpoint = false;
                        *subgrid [j] = pset_xor(*subgrid [j],
                                pset_and(*subgrid [i], *subgrid [j]));

                    }
                }
            }
        }
    }
    return subfixpoint;
}

/*
 * grid_heuristics applies heuristics written in subgrid_heuristics while the
 * heuristics are useful (ie grid is not a fixpoint)
 *
 * if verbose option, each intermediate grid is printed
 *
 * RETURN VALUE :
 * _ 0 if grid is solved
 * _ 1 if grid is not solved but still consistent
 * _ 2 if grid is not solved and no more consistent
 */

static int grid_heuristics(pset_t **grid) {
    bool fixpoint = false;
    while (!fixpoint) /*Fixpoint is reached ? */ {
        if (verbose) {
            fprintf(stdout, "%zu appel(s) heuristiques\n", ++iter);
            grid_print(grid);
            fprintf(stdout, "\n");
        }

        fixpoint = subgrid_map(grid, subgrid_heuristics);

        if (!grid_consistency(grid)) {
            return 2;
        }

    }

    if (grid_solved(grid)) return 0;
    return 1;
}


//************Backtracking********************

/*
 * utility function to retrieve depth of stack
 */

static size_t stack_depth(choice_t *stack) {
    if (stack == NULL) return 0;
    return 1 + stack_depth(stack->previous);
}

/*  
 * utility function print the stack
 */

static void stack_print(choice_t *stack) {
        char tmp[grid_size + 1];
        pset2str(tmp, stack->choice);
        printf("choice %s at %zu %zu", tmp, stack->x, stack->y);
        stack_print(stack->previous);
}

/*
 * copy grid before trying chosen values in stack
 */

static pset_t** grid_copy(pset_t **grid) {
    pset_t** gr = grid_alloc();
    for (size_t i = 0; i < grid_size; i++) {
        for (size_t j = 0; j < grid_size; j++) {
            gr[i][j] = grid[i][j];
        }
    }
    return gr;
}

/*
 * getMincardIndex quickly retrieve the minimum cardinality in a grid lin
 * it is used by stack_push
 */

static size_t getMinCardIndex(pset_t a[], size_t start, size_t len, size_t *l){
    if (len == 1) {
        return start;
    }
    int p = floor(len / 2);
    size_t n1 = getMinCardIndex(a, start, p, l);
    size_t n2 = getMinCardIndex(a, start + p, len - p, l);
    size_t min = grid_size - 1;
    if (pset_is_singleton(a[n1]) && !pset_is_singleton(a[n2])) {
        min = n2;
        *l = pset_cardinality(a[n2]);
    } else if (!pset_is_singleton(a[n1]) && pset_is_singleton(a[n2])) {
        min = n1;
        *l = pset_cardinality(a[n1]);
    } else if (!pset_is_singleton(a[n1]) && !pset_is_singleton(a[n2])) {
        if (pset_cardinality(a[n1]) < pset_cardinality(a[n2])) {
            min = n1;
            *l = pset_cardinality(a[n1]);
        } else {
            min = n2;
            *l = pset_cardinality(a[n2]);
        }
    }
    return min;
}

/*
 * stack_push pushes choices stack by looking for
 * the minimal cardinality and makes allocations for it
 */
static choice_t *stack_push(choice_t *stack, pset_t ***grid) {
    size_t i, line = 0, column = 0, mincard = grid_size;
    size_t cur = mincard;
    size_t l;
    choice_t* newstack = malloc(sizeof (choice_t));
    if (newstack == NULL) {
        fprintf(stderr, "sudoku: error: out of memory !");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < grid_size; i++) {
        l = getMinCardIndex((*grid)[i], 0, grid_size, &cur);
        if (cur < mincard) {
            mincard = cur;
            line = i;
            column = l;
        }

    }

    if (mincard == 0) return NULL;
    newstack->x = line;
    newstack->y = column;
    newstack->choice = pset_leftmost((*grid)[line][column]);
    newstack->previous = stack;
    newstack->grid = grid_copy(*grid);
    newstack->grid[line][column] = pset_discard2(newstack->choice,
            newstack->grid[line][column]);
    (*grid)[line][column] = newstack->choice;

    return newstack;
}

/*
 * stack_pop pop choices stack. If there is a choice, it tries it.
 * otherwise it returns NULL
 */

static choice_t *stack_pop(choice_t *stack, pset_t ***grid) {
    if (stack == NULL) return NULL;
    size_t line = stack->x;
    size_t column = stack->y;
    choice_t* t = stack;
    choice_t *l = NULL;
    pset_t value = t->grid[line][column];
    while (value == pset_empty()) {
        l = t->previous;
        if (l == NULL) {
            free(t);
            t = NULL;
            return NULL;
        }
        line = l->x;
        column = l->y;
        grid_free(t->grid);
        value = l->grid[line][column];
        free(t);
        t = l;
    }
    t->choice = pset_leftmost(t->grid[line][column]);
    t->grid[line][column] = pset_discard2(t->choice, t->grid[line][column]);
    for (size_t i = 0; i < grid_size; i++) {
        for (size_t j = 0; j < grid_size; j++) {
            (*grid)[i][j] = (t->grid)[i][j];
        }
    }
    (*grid)[line][column] = t->choice;
    return t;
}

/*
 * free choices stack
 */

static void stack_free(choice_t *stack) {
    choice_t* temp;
    while (stack != NULL) {
        temp = stack->previous;
        grid_free(stack->grid);
        free(stack);
        stack = temp;
    }
}

/*
 * grid_solver solves the grid. Stops at the first solution found and return 1
 * if there is no solutions it returns 0
 */

static bool grid_solver(pset_t **grid) {
    bool solvable = true;
    int t = grid_heuristics(grid);
    if (t == 0)
        return 1;
    if (t == 2) {
        return 0;
    }
    if (t == 1) {
        choice_t* chosen = NULL;
        bool popped = false;
        while (solvable) {
            if (!popped) {
                chosen = stack_push(chosen, &grid);
                popped = false;
                if (chosen == NULL) {
                    solvable = false;
                    break;
                }

            }
            t = grid_heuristics(grid);

            if (t == 1) {
                popped = false;
                continue;
            }
            if (t == 0)
                break;
            if (t == 2) {
                chosen = stack_pop(chosen, &grid);
                if (chosen == NULL) {
                    solvable = false;
                    break;
                }
                popped = true;
            }
        }
        stack_free(chosen);
    }
    return solvable;
}

/*
 * grid_solver2 check if the grid has more than 1 solution. In this case
 * it return 0; if it has only one solution it returns 1;
 */

static int grid_solver2(pset_t **grid) {
    bool solvable = true;
    pset_t ** grid2 = grid_copy(grid);
    int t = grid_heuristics(grid2);
    int n = 0;
    if (t == 0) {
        grid_free(grid2);
        return 1;
    }
    choice_t* chosen = NULL;
    bool popped = false;
    if (t != 2) {
        while (solvable) {
            if (!popped) {
                chosen = stack_push(chosen, &grid2);
                popped = false;
                if (chosen == NULL) {

                    solvable = false;
                }
            }
            //try the chosen value
            t = grid_heuristics(grid2);
            if (t == 1) {
                popped = false;
                continue;
            }
            if (t == 0) {
                n++;
                if (n > 1) {
                    stack_free(chosen);
                    grid_free(grid2);
                    return 0;
                }
                chosen = stack_pop(chosen, &grid2);
                if (chosen == NULL) {
                    stack_free(chosen);
                    grid_free(grid2);
                    return n;
                } else {
                    popped = true;
                    continue;
                }
            }
            if (t == 2) {
                // backtracking
                chosen = stack_pop(chosen, &grid2);
                if (chosen == NULL) {
                    stack_free(chosen);
                    grid_free(grid2);
                    return n;
                }
                popped = true;
            }
        }
        stack_free(chosen);
    }
    grid_free(grid2);
    return n;
}
//********************Grid generation********************

/*
 * shuffle shuffles an array where values is the array
 * and l the size of the array
 */

void shuffle(size_t l, size_t values []) {
    size_t temporary = 0;
    size_t randomNum = 0;
    size_t last = 0;

    for (last = l; last > 1; last--) {
        randomNum = rand() % last;
        temporary = values[randomNum];
        values[randomNum] = values[last - 1];
        values[last - 1] = temporary;
    }
}

/*
 *permutate applies permutations on grid;
 * Informally, a permutation of a set of objects is an arrangement of
 * those objects into a particular order.
 */

void permutate(pset_t **grid) {
    time_t t;
    srand((unsigned) time(&t));
    size_t values[grid_size];
    for (size_t i = 0; i < grid_size; i++) {
        values[i] = i;
    }
    shuffle(grid_size, values);
    for (size_t i = 0; i < grid_size; i++) {
        for (size_t j = 0; j < grid_size; j++) {
            size_t p = trailingZeroCount(grid[i][j]);
            size_t l = values[p];
            grid[i][j] = char2pset(color_table[l]);
        }
    }
}

/*
 *permutate_row permutates two random rows
 * in the same block
 */

void permutate_row(pset_t **grid) {
    time_t t;
    srand((unsigned) time(&t));
    size_t p = (sqrt(grid_size));
    size_t values[p];
    for (size_t i = 0; i < p; i++) {
        values[i] = i;
    }
    shuffle(p, values);
    size_t blocknum = rand() % p;
    size_t row1 = values[1] + blocknum*p;
    size_t row2 = values[2] + blocknum*p;
    pset_t temp = 0;
    for (size_t i = 0; i < grid_size; i++) {
        temp = grid[row1][i];
        grid[row1][i] = grid[row2][i];
        grid[row2][i] = temp;
    }
}

/*
 *permutate_column permutates two random
 * columns in the same block
 */

void permutate_column(pset_t **grid) {
    time_t t;
    srand((unsigned) time(&t));
    size_t p = (sqrt(grid_size));
    size_t values[p];
    for (size_t i = 0; i < p; i++) {
        values[i] = i;

    }
    shuffle(p, values);
    size_t blocknum = rand() % p;
    size_t column1 = values[0] + blocknum*p;
    size_t column2 = values[1] + blocknum*p;
    pset_t temp;
    for (size_t j = 0; j < grid_size; j++) {
        temp = grid[j][column1];
        grid[j][column1] = grid[j][column2];
        grid[j][column2] = temp;
    }
}

/*
 * grid_print_or prints the grid with dash(_) at removed cells
 */

static void grid_print_or(pset_t ** grid) {
    char tmp [grid_size + 1];
    size_t i, j;
    for (i = 0; i < grid_size; i++) {
        for (j = 0; j < grid_size; j++) {
            if (grid[i][j] == pset_full(grid_size)) {
                printf("_ ");
            } else {
                pset2str(tmp, grid[i][j]);
                printf("%s ", tmp);
            }
        }
        printf("\n");
    }
}

/* dash_gen  removes some cells
 * from solved grid at random
 */

static void dash_gen(pset_t ** grid) {
    time_t t;
    size_t line;
    size_t column;
    size_t value;
    srand((unsigned) time(&t));
    size_t p = grid_size*grid_size;
    size_t n = (grid_size == 1) ? 0 : (rand() % (p / 2)) + p / 2;
    size_t values [n];
    size_t i = 1;
    values[0] = rand() % p;
    while (i < n) {
        value = rand() % p;
        values[i] = value;
        i++;
    }
    for (size_t j = 0; j < i; j++) {
        line = values[j] / grid_size;
        column = values[j] % grid_size;

        grid[line][column] = pset_full(grid_size);
    }
    grid_print_or(grid);
}

/*
 * randomchooser chooses a random line and generates a permutation
 * to fill on this single line
 */

static void randomchooser(pset_t **grid) {
    time_t t;
    size_t i = 1;
    size_t values [grid_size];
    for (size_t i = 0; i < grid_size; i++) {
        values[i] = i;
    }
    srand((unsigned) time(&t));
    shuffle(grid_size, values);
    size_t temporary;
    size_t randomNum = rand() % grid_size;
    for (i = 0; i < grid_size; i++) {
        temporary = values[i];
        grid[randomNum][i] = char2pset(color_table[temporary]);
    }
}

/*
 * randomcleaner removes a
 * random cell from the grid
 */

static void randomcleaner(pset_t **grid, size_t *line2, size_t *column2,
        pset_t * value) {
    time_t t;
    bool done = false;
    srand((unsigned) time(&t));
    size_t line;
    size_t column;
    do {
        line = rand() % grid_size;
        column = rand() % grid_size;
        if (grid[line][column] != pset_full(grid_size)) {
            *line2 = line;
            *column2 = column;
            *value = grid[line][column];
            grid [line][column] = pset_full(grid_size);
            done = true;

        }
    } while (done == false);

}

/*  gen generates both strict
 *  and non-strict grids
 */

static void gen(void) {
    size_t n = sqrt(grid_size);
    pset_t** grid = grid_alloc();
    if (grid_size==1){
        grid[0][0]=pset_full(grid_size);
        grid_print_or(grid);
        grid_free(grid);
    }else{
    if (!strict) {
        if (grid_size > 25) {
            time_t t;
            srand((unsigned) time(&t));
            for (size_t i = 0; i < grid_size; i++) {
                for (size_t j = 0; j < grid_size; j++) {
                    grid[i][j] = char2pset(color_table[(((i / n)
                            + n * (i % n) + j) % grid_size)]);
                }
            }
            size_t l = rand() % grid_size + 1;
            for (size_t i = 0; i < l; i++) {
                permutate_column(grid);
                permutate_row(grid);
                permutate(grid);
            }
            grid_solver(grid);

        } else {
            for (size_t i = 0; i < grid_size; i++) {
                for (size_t j = 0; j < grid_size; j++) {
                    grid[i][j] = pset_full(grid_size);

                }
            }
            randomchooser(grid);
            grid_solver(grid);

        }
        dash_gen(grid);
        grid_free(grid);

    } else {
        int p = 0;
        if (grid_size > 25) {
            time_t t;
            srand((unsigned) time(&t));
            for (size_t i = 0; i < grid_size; i++) {
                for (size_t j = 0; j < grid_size; j++) {
                    grid[i][j] = char2pset(color_table[(((i / n) + n *
                            (i % n) + j) % grid_size)]);

                }
            }
            size_t l = rand() % grid_size + 1;
            for (size_t i = 0; i < l; i++) {
                permutate_column(grid);
                permutate_row(grid);
                permutate(grid);
            }

        } else {
            for (size_t i = 0; i < grid_size; i++) {
                for (size_t j = 0; j < grid_size; j++) {
                    grid[i][j] = pset_full(grid_size);

                }
            }
            randomchooser(grid);
        }
        while (grid_solver(grid) == false) {
            for (size_t i = 0; i < grid_size; i++) {
                for (size_t j = 0; j < grid_size; j++) {
                    grid[i][j] = pset_full(grid_size);

                }
            }
            randomchooser(grid);
            grid_solver(grid);
        }
        size_t line, column;
        pset_t value;

        randomcleaner(grid, &line, &column, &value);
        p = grid_solver2(grid);
        // remove random cell until unique solution
        while (p == 1) {
            randomcleaner(grid, &line, &column, &value);
            p = grid_solver2(grid);
        }
        grid[line][column] = value;
        grid_print_or(grid);
        grid_free(grid);
    }
}
}
//********************Main********************

int main(int argc, char* argv[]) {

    bool o = false;
    file = basename(argv[0]);
    int optc;
    static struct option long_opts[ ] = {
        {"help", 0, 0, 'h'},
        {"output", 1, 0, 'o'},
        {"version", 0, 0, 'V'},
        {"verbose", 0, 0, 'v'},
        {"generate", 2, 0, 'g'},
        {"strict", 0, 0, 's'},
        {0, 0, 0, 0}
    };
    while ((optc = getopt_long(argc, argv, "ho:Vvg::s", long_opts, NULL)) != -1) {
        switch (optc) {
            case 'o':
                o = true;
                output = freopen(optarg, "w", stdout);
                if (output == NULL) usage(EXIT_FAILURE);
                break;

            case 'h':
                usage(EXIT_SUCCESS);
                break;

            case 'V':
                version();
                break;
            case 'v':
                verbose = true;
                break;
            case 'g':
                generate = true;
                grid_size = 9;
                if (optarg != NULL) {
                    grid_size = (size_t) atoi(optarg);

                    if (grid_size != 1 && grid_size != 4 && grid_size != 9
                            && grid_size != 16 && grid_size != 25 && grid_size
                            != 36 && grid_size != 49 && grid_size != 64) {

                        usage(EXIT_FAILURE);
                    }
                }
                break;
            case 's':
                strict = true;
                break;
            default:
                usage(EXIT_FAILURE);
        }
    }

    if (argc < optind || argc == 1) {
        usage(EXIT_FAILURE);
    }
    if (o == true && argc == 3) {
        printf("missing grid_file argument");
        exit(EXIT_FAILURE);
    }

    if (generate) {
        gen();
    } else {

        FILE* grid_fichier = NULL;

        grid_fichier = fopen(argv[argc - 1], "r");
        if (grid_fichier == NULL){
              fprintf(stdout, "%s: unable to open grid file\n", file);
              exit(EXIT_FAILURE);
        }else {
            pset_t** t = grid_parser(grid_fichier);
            if (!grid_consistency(t)) {
                fprintf(stdout, "%s: error: grid is not consistent\n", file);
                exit(EXIT_SUCCESS);
            }
            if (grid_solver(t) == 1) {
                printf("The grid has been solved !\n");
                grid_print(t);
            } else {
                printf("The grid hasn't been solved\n");

            }
            grid_free(t);
        }
        fclose(grid_fichier);
    }
    exit(EXIT_SUCCESS);
}
