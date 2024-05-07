#ifndef INITBOARD_H
#define INITBOARD_H

#include <stdbool.h>

#include "fileUtils.h"
#include "parameters.h"

typedef struct {
    int first;
    int second;
} pair;

typedef struct {
    int row;
    int column;
} coordinate;

typedef struct {
    struct term term;
    bool isHorizontal;
    coordinate starts;
    bool isKnown;
    int index;
    int intersection;
} termInBoard;

// Function prototypes

bool isTermAlreadyInBoard(struct term term);

bool termFitsInBoard(struct term term, coordinate starts, bool isHorizontal);

bool termCollidesWithBoardCharacters(termInBoard term);

pair* findIntersectionsBetweenTerms(term term1, term term2, int* numPairs);

void calculateStartPosition(coordinate *starts,termInBoard* termInBoard, pair intersection);

void shuffleIntersections(int numPairs, int* indices);

void addTermToCrosswordBoard(termInBoard term);

void* worker_function(void* arg);

#endif //INITBOARD_H
