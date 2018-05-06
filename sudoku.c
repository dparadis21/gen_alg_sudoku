#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define POPULATION 100
#define NINES 1
#define MUTATION_P 0.01

void print_board(unsigned char c[81]);
int board_fitness(unsigned char c[81]);
int row_fitness(unsigned char c[81]);
int column_fitness(unsigned char c[81]);
int box_fitness(unsigned char c[81]);
int choose_parent(int *c, int t);
void make_child(unsigned char c[81], unsigned char p1[81], unsigned char p2[81]);

unsigned char rowtosquare[81] = {0, 1, 2, 9, 10, 11, 18, 19, 20, 3, 4, 5, 12, 13, 14, 21, 22, 23, 6, 7, 8, 15, 16, 17, 24, 25, 26,
27, 28, 29, 36, 37, 38, 45, 46, 47, 30, 31, 32, 39, 40, 41, 48, 49, 50, 33, 34, 35, 42, 43, 44, 51, 52, 53,
54, 55, 56, 63, 64, 65, 72, 73, 74, 57, 58, 59, 66, 67, 68, 75, 76, 77, 60, 61, 62, 69, 70, 71, 78, 79, 80};

unsigned char fixed_boxes[81] =
{10, 10, 8, 10, 9, 10, 7, 10, 10,
 7, 3, 10, 10, 10, 10, 1, 2, 6,
 6, 4, 2, 3, 10, 10, 9, 5, 10,
 10, 10, 10, 9, 6, 10, 2, 10, 7,
 10, 10, 5, 10, 2, 10, 3, 10, 10,
 10, 10, 7, 8, 10, 10, 10, 10, 1,
 10, 10, 1, 2, 10, 10, 10, 10, 3,
 2, 8, 3, 4, 10, 9, 6, 10, 10,
 4, 7, 10, 1, 5, 3, 10, 10, 10};

unsigned char solution[81] =
{1, 5, 8, 6, 9, 2, 7, 3, 4,
7, 3, 9, 5, 4, 8, 1, 2, 6,
6, 4, 2, 3, 1, 7, 9, 5, 8,
3, 1, 4, 9, 6, 5, 2, 8, 7,
8, 6, 5, 7, 2, 1, 3, 4, 9,
9, 2, 7, 8, 3, 4, 5, 6, 1,
5, 9, 1, 2, 8, 6, 4, 7, 3,
2, 8, 3, 4, 7, 9, 6, 1, 5,
4, 7, 6, 1, 5, 3, 8, 9, 2};

int main() {
        unsigned char parents[POPULATION][81]; // POPULATION number of sudoku boards
        unsigned char children[POPULATION][81];
        unsigned char parent_index[POPULATION];
        int roulette_thresh[POPULATION + 1];
        int roulette_total;
        int not_done = 1;
        long long int generations = 0;
        int i, j; // loop counters

        unsigned char best_attempt[81];
        int best_fitness = 0;

        srand(time(NULL)); // initialize random numbers

        // make all numbers 1 less
        for (i = 0; i < 81; i++) {
                fixed_boxes[i]--;
                solution[i]--;
        }

        printf("Fitness: %d\n", board_fitness(solution));
        print_board(solution);

        // fill initial population with random values
        for (i = 0; i < 81; i++) {
                for (j = 0; j < POPULATION; j++) {
                        if (fixed_boxes[i] == 9)
                                parents[j][i] = rand() % 9;
                        else
                                parents[j][i] = fixed_boxes[i];
                }
        }

        while(generations < 1000000) {
                // determine fitness of all boards and create thresholds for the roulette
                generations++;
                roulette_thresh[0] = 0;
                roulette_total = 0;
                for (i = 0; i < POPULATION; i++) {
                        roulette_total += board_fitness(parents[i]);

                        if (board_fitness(parents[i]) > best_fitness) {
                                for (j = 0; j < 81; j++) {
                                        best_attempt[j] = parents[i][j];
                                }
                                printf("Generation: %lld\n", generations);
                                print_board(best_attempt);

                                best_fitness = board_fitness(parents[i]);
                        }

                        if (board_fitness(parents[i]) == 243) {
                                not_done = 0;
                                printf("This puzzle has been solved in %lld generations!\n", generations);
                                print_board(parents[i]);
                                printf("Fitness: %d\n", board_fitness(parents[i]));
                        }

                        roulette_thresh[i + 1] = roulette_total;
                }

                // determine the parents that will be passing on their genes
                for (i = 0; i < POPULATION; i++)
                        parent_index[i] = choose_parent(roulette_thresh, roulette_total);

                // make children
                for (i = 0; i < POPULATION - 1; i++) {
                        if (i % 2)
                                make_child(children[i], parents[parent_index[i]], parents[parent_index[i + 1]]);
                        else
                                make_child(children[i], parents[parent_index[i + 1]], parents[parent_index[i]]);
                }

                // copy children into parents
                for (i = 0; i < POPULATION; i++) {
                        for (j = 0; j < 81; j++) {
                                parents[i][j] = children[i][j];
                        }
                }
        }

        print_board(best_attempt);
        printf("Fitness: %d\n", best_fitness);

        return 0;
}

// prints the sudoku board
void print_board(unsigned char c[81])
{
        int i, j;

        for (i = 0; i < 9; i++) {
                for (j = 0; j < 9; j++) {
                        printf("%d ", c[i * 9 + j] + NINES);
                }
                printf("\n");
        }
}

// determines the fitness of the sudoku board.  1 point for each unique number in a row, column, and box
int board_fitness(unsigned char c[81])
{
        return row_fitness(c) + column_fitness(c) + box_fitness(c);
}

// fitness of the rows in a sudoku board
int row_fitness(unsigned char c[81])
{
        int ret = 0;
        int i, j, k;

        for (k = 0; k < 9; k++) {
                for (i = 0; i < 9; i++) {
                        for (j = 0; j < i; j++) {
                                if (c[i + k * 9] == c[j + k * 9])
                                        break;
                        }
                        if (i == j)
                                ret++;
                }
        }

        return ret;
}

// fitness of the columns in a sudoku board
int column_fitness(unsigned char c[81])
{
        int ret = 0;
    int i, j, k;

    for (k = 0; k < 9; k++) {
        for (i = 0; i < 9; i++) {
            for (j = 0; j < i; j++) {
                if (c[i * 9 + k] == c[j * 9 + k])
                    break;
            }
            if (i == j)
                ret++;
        }
    }

    return ret;
}

// fitness of each box in a sudoku board
int box_fitness(unsigned char c[81])
{
        int ret = 0;
        int i, j, k;
        char t[81];

        // transpose c to t where each square is given in a row
        for (i = 0; i < 81; i++) {
                t[i] = c[rowtosquare[i]];
        }

    for (k = 0; k < 9; k++) {
        for (i = 0; i < 9; i++) {
            for (j = 0; j < i; j++) {
                                if (t[i + k * 9] == t[j + k * 9])
                    break;
            }
            if (i == j)
                ret++;
        }
    }

    return ret;
}

int choose_parent(int *c, int t)
{
        int i, r;

        r = rand() % t;
        for (i = 0; i < POPULATION; i++) {
                if ((r >= c[i]) && (r < c[i + 1]))
                        return i;
                else
                        return 0;
        }

        return 0;
}

void make_child(unsigned char c[81], unsigned char p1[81], unsigned char p2[81])
{
        int i, r, mut;

        r = rand() % 81;
        mut = rand() % 100;
        for (i = 0; i < 81; i++) {
                if (i <= r)
                        c[i] = p1[i];
                else
                        c[i] = p2[i];

                if (!mut && (fixed_boxes[i] == 9))
                        c[i] = rand() % 9;
        }
}
