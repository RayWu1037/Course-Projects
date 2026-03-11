/* 
 * CS 211 – Lab/Project 1: <Totalistic Cellular Automaton>
 * Name: <Ruiyi Wu>, NetID: <wruiy@uic.edu>
 * Date: 2025-09-25
 * Description:
 *   Implements a 1D totalistic cellular automaton.
 *   Code is formatted per course style: header, camelCase, comments, and consistent indentation.
 */

#include <stdio.h>
#include <stdbool.h>

/* -------------------- Constants & Types -------------------- */

const int WORLD_SIZE = 65;

typedef struct cell_struct {
    int localSum;  // sum of [left] + [me] + [right], range 0..6
    int status;    // current status value for this cell: 0, 1, or 2
    int count;     // accumulated sum of this cell's statuses across generations
} cell;

/* -------------------- Prototypes -------------------- */

// Task 1
bool setValArray(int valArray[7], int rule);

// Task 4
void setSums(cell world[WORLD_SIZE]);

// Task 6
int evolveWorld(cell world[WORLD_SIZE], int ruleValArray[7]);

// Print helpers (renamed to camelCase)
char statusCh(int s);
void printValueArray(const int ruleVal[7], int rule);
void printNewStatusRow(const int ruleVal[7]);
void printWorld(cell world[WORLD_SIZE], int total);
void printUnderscoreLine(void);
void printCounts(cell world[WORLD_SIZE]);

/* -------------------- Implementations -------------------- */

// Convert rule (0..2186) to base-3 digits (reversed) into valArray[7].
// Returns true if rule valid, false otherwise.
bool setValArray(int valArray[7], int rule) {
    if (rule < 0 || rule > 2186) {
        return false;
    }
    for (int i = 0; i < 7; ++i) {
        valArray[i] = rule % 3;
        rule /= 3;
    }
    return true;
}

// Update localSum for each cell using periodic neighbors.
void setSums(cell world[WORLD_SIZE]) {
    for (int i = 0; i < WORLD_SIZE; ++i) {
        int left  = (i == 0) ? WORLD_SIZE - 1 : i - 1;
        int right = (i == WORLD_SIZE - 1) ? 0 : i + 1;
        world[i].localSum = world[left].status + world[i].status + world[right].status;
    }
}

// Evolve world one generation; also update counts; return total status sum.
int evolveWorld(cell world[WORLD_SIZE], int ruleValArray[7]) {
    int total = 0;
    for (int i = 0; i < WORLD_SIZE; ++i) {
        int next = ruleValArray[ world[i].localSum ];
        world[i].status = next;
        world[i].count  += next;
        total           += next;
    }
    return total;
}

/* -------------------- Printing helpers (camelCase) -------------------- */

char statusCh(int s) { 
    return (s == 2) ? '+' : ((s == 1) ? '-' : ' '); 
}

void printValueArray(const int ruleVal[7], int rule) {
    printf("\n\nThe value array for rule #%d is ", rule);
    for (int i = 6; i >= 0; --i) {
        printf("%d", ruleVal[i]);
    }
    printf("\n\n");
}

void printNewStatusRow(const int ruleVal[7]) {
    printf("New Status: ");
    const char* sep = "     ";
    int order[7] = {6, 5, 4, 3, 2, 1, 0};
    for (int k = 0; k < 7; ++k) {
        printf("|%c|", statusCh(ruleVal[ order[k] ]));
        if (k != 6) {
            printf("%s", sep);
        }
    }
    putchar('\n');
}

void printWorld(cell world[WORLD_SIZE], int total) {
    for (int i = 0; i < WORLD_SIZE; ++i) {
        putchar(statusCh(world[i].status));
    }
    printf(" %d \n", total);
}

void printUnderscoreLine(void) {
    for (int i = 0; i < WORLD_SIZE; ++i) {
        putchar('_');
    }
    putchar('\n');
}

void printCounts(cell world[WORLD_SIZE]) {
    // tens row
    for (int i = 0; i < WORLD_SIZE; ++i) {
        int c = world[i].count;
        putchar(c == 0 ? ' ' : (c >= 10 ? ('0' + (c / 10) % 10) : ' '));
    }
    putchar('\n');
    // ones row
    for (int i = 0; i < WORLD_SIZE; ++i) {
        int c = world[i].count;
        putchar(c == 0 ? ' ' : ('0' + (c % 10)));
    }
    putchar('\n');
}

/* -------------------- main -------------------- */

int main(void) {
    cell world[WORLD_SIZE];
    int rule, ruleVal[7], generations, initVal;

    printf("Welcome to the Totalistic Cellular Automaton!\n");

    // Task 2 – read rule and print its value array
    while (1) {
        printf("Enter the rule # (0-2186): ");
        if (scanf("%d", &rule) != 1) return 0;
        if (setValArray(ruleVal, rule)) break;
    }
    printValueArray(ruleVal, rule);

    printf("The evolution of all possible states are as follows:");
    printf("Local Sum:   6       5       4       3       2       1       0      \n");
    printf("States:    |+++|   |++-|   |++ |   |+- |   |-- |   |-  |   |   |    \n");
    printf("                   |+-+|   |+ +|   |+ -|   |- -|   | - |            \n");
    printf("                   |-++|   | ++|   |-+ |   | --|   |  -|            \n");
    printf("                           |+--|   | +-|   |+  |                    \n");
    printf("                           |-+-|   |- +|   | + |                    \n");
    printf("                           |--+|   | -+|   |  +|                    \n");
    printf("                                   |---|                            \n");
    printNewStatusRow(ruleVal);
    printf("\n");

    // Task 5a – generations
    while (1) {
        printf("Enter the number of generations (1-49): ");
        if (scanf("%d", &generations) != 1) return 0;
        if (generations >= 1 && generations <= 49) break;
    }

    // Task 5b – initial value
    while (1) {
        printf("Enter the value (1 or 2) for the initial active cell: ");
        if (scanf("%d", &initVal) != 1) return 0;
        if (initVal == 1 || initVal == 2) break;
    }

    printf("\nInitializing world & evolving...\n");

    // Task 5c – initialize middle cell and zero others
    for (int i = 0; i < WORLD_SIZE; ++i) {
        world[i].status   = 0;
        world[i].count    = 0;
        world[i].localSum = 0;
    }
    world[WORLD_SIZE / 2].status = initVal;
    world[WORLD_SIZE / 2].count  = initVal;
    setSums(world);

    // First display
    int total = 0;
    for (int i = 0; i < WORLD_SIZE; ++i) total += world[i].status;
    printWorld(world, total);

    // Task 7 – evolve for remaining generations
    for (int gen = 2; gen <= generations; ++gen) {
        total = evolveWorld(world, ruleVal);
        setSums(world);
        printWorld(world, total);
    }

    // Underline and totals (Task 8)
    printUnderscoreLine();
    printCounts(world);

    printf("\n");
    return 0;
}
