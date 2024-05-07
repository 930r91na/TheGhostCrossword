#ifndef THEGHOSTCROSSWORD_PARAMETERS_H
#define THEGHOSTCROSSWORD_PARAMETERS_H

// Constants
#define NUM_THREADS 4
#define BOARD_SIZE 11
#define NUMBER_OF_TERMS 8
#define WORD_DISAPPEAR_TIME 20

#define BLU   "\x1B[34m"
#define RESET "\x1B[0m"
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"

// Custom types
typedef struct {
    int first;
    int second;
} pair;

typedef struct {
    int row;
    int column;
} coordinate;

typedef struct term{
    char* word;
    char* description;
} term;

typedef struct {
    struct term term;
    bool isHorizontal;
    coordinate starts;
    bool isKnown;
    int index;
    int intersection;
} termInBoard;

#endif //THEGHOSTCROSSWORD_PARAMETERS_H
