#ifndef INITBOARD_H
#define INITBOARD_H

#include <stdbool.h>
#include "fileUtils.h"


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
} termInBoard;

// Function prototypes

bool isTermAlreadyInBoard(struct term term);

bool termFitsInBoard(struct term term, coordinate starts, bool isHorizontal);

bool termCollidesWithBoardCharacters(termInBoard term);

pair* findIntersectionsBetweenTerms(term term1, term term2, int* numPairs);

void calculateStartPosition(coordinate *starts,termInBoard* termInBoard, pair intersection);

void shuffleIntersections(int numPairs, int* indices);

void addTermToCrosswordBoard(termInBoard term);

void generateAndPlaceTerm();

#endif //INITBOARD_H