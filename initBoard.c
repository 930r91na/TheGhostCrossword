#include "fileUtils.h"
#include "initBoard.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#define boardSize 11
#define numberOfTerms 12

// Global variables
extern char crosswordBoard[boardSize][boardSize];
extern termInBoard termsInBoard[numberOfTerms];

bool isTermAlreadyInBoard(struct term term) {
    for (int i = 0; i < numberOfTerms; i++) {
        if (termsInBoard[i].term.word == NULL) {
            continue;
        }
        if (strcmp(termsInBoard[i].term.word, term.word) == 0) {
            return true;
        }
    }
    return false;
}
bool termFitsInBoard(struct term term, coordinate starts, bool isHorizontal) {
    if (starts.column < 0 || starts.row < 0) {
        return false;
    }
    if (isHorizontal) {
        if (starts.column + strlen(term.word) >= boardSize) {
            return false;
        }
    } else {
        if (starts.row + strlen(term.word) >= boardSize) {
            return false;
        }
    }
    return true;
}

bool termCollidesWithBoardCharacters(termInBoard term){
    int termLength = strlen(term.term.word);
    if (term.isHorizontal) {
        for (int i = 0; i < termLength; i++) {
            char boardChar = crosswordBoard[term.starts.row][term.starts.column + i];
            if (boardChar != '*' && boardChar != term.term.word[i]) {
                return true;
            }
        }
    } else {
        for (int i = 0; i < termLength; i++) {
            char boardChar = crosswordBoard[term.starts.row + i][term.starts.column];
            if (boardChar != '*' && boardChar != term.term.word[i]) {
                return true;
            }
        }
    }
    return false;
}

pair* findIntersectionsBetweenTerms(term term1, term term2, int* numPairs) {
    pair* intersections = NULL;
    *numPairs = 0;

    // Check if term1 or term2 is NULL
    if (term1.word == NULL || term2.word == NULL) {
        *numPairs = 0;
        return NULL;
    }

    for (int i = 0; i < strlen(term1.word); i++) {
        for (int j = 0; j < strlen(term2.word); j++) {
            if (term1.word[i] == term2.word[j]) {
                intersections = realloc(intersections, (*numPairs + 1) * sizeof(pair));
                intersections[*numPairs].first = i;
                intersections[*numPairs].second = j;
                (*numPairs)++;
            }
        }
    }

    return intersections;
}
void calculateStartPosition(coordinate *starts,termInBoard* termInBoard, pair intersection) {
    if (!termInBoard->isHorizontal) {
        starts->row = termInBoard->starts.row + intersection.second;
        starts->column = termInBoard->starts.column - intersection.first;
    } else {
        starts->row = termInBoard->starts.row - intersection.first;
        starts->column = termInBoard->starts.column + intersection.second;
    }
}

void shuffleIntersections(int numPairs, int* indices){
    // Create an array of indices
    for (int j = 0; j < numPairs; j++) {
        indices[j] = j;
    }

    // Shuffle the indices array
    for (int j = numPairs - 1; j > 0; j--) {
        int k = rand() % (j + 1);
        int temp = indices[j];
        indices[j] = indices[k];
        indices[k] = temp;
    }
}

void addTermToCrosswordBoard(termInBoard term){
    if (term.isHorizontal) {
        for (int l = 0; l < strlen(term.term.word); l++) {
            crosswordBoard[term.starts.row][term.starts.column + l] = term.term.word[l];
        }
    } else {
        for (int l = 0; l < strlen(term.term.word); l++) {
            crosswordBoard[term.starts.row + l][term.starts.column] = term.term.word[l];
        }
    }
}

void generateAndPlaceTerm() {
    bool placed = false;
    srand(time(NULL)); // Seed the random number generator

    while (!placed) {
        int size = rand() % 6 + 4;  // Generate a random term size between 4 and 9
        term newTerm = getRandomTerm(size);
        if (isTermAlreadyInBoard(newTerm)) {
            continue;
        }

        // Iterate through all terms to find intersections with non-NULL terms
        for (int i = numberOfTerms; i >= 0 && !placed; i--) {
            if (termsInBoard[i].term.word != NULL) {  // Ensure the term is not NULL
                int numPairs;
                pair* intersections = findIntersectionsBetweenTerms(newTerm, termsInBoard[i].term, &numPairs);
                int* indices = malloc(numPairs * sizeof(int));

                // Shuffle the intersections
                shuffleIntersections(numPairs, indices);

                if (numPairs > 0) {
                    for (int j = 0; j < numPairs; j++) {
                        pair intersection = intersections[indices[j]];
                        coordinate starts;
                        bool isHorizontal = !termsInBoard[i].isHorizontal;

                        calculateStartPosition(&starts, &termsInBoard[i], intersection);

                        if (termFitsInBoard(newTerm, starts, isHorizontal)) {
                            termInBoard newTermInBoard = {newTerm, isHorizontal, starts, false};

                            if (!termCollidesWithBoardCharacters(newTermInBoard)) {
                                // Find an empty spot to place the new term
                                for (int k = 0; k < numberOfTerms; k++) {
                                    if (termsInBoard[k].term.word == NULL) {
                                        newTermInBoard.index = k;
                                        termsInBoard[k] = newTermInBoard;
                                        addTermToCrosswordBoard(newTermInBoard);
                                        placed = true;
                                        break;
                                    }
                                }
                                if (placed) break;
                            }
                        }
                    }
                    free(intersections);
                }
            }
        }
    }
}

