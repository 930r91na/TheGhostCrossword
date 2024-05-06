#include "fileUtils.h"
#include "initBoard.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <malloc.h>

#define boardSize 11
#define numberOfTerms 12
#define NUM_THREADS 4

// Global variables
extern char crosswordBoard[boardSize][boardSize];
extern termInBoard termsInBoard[numberOfTerms];

// Global variable to control the thread pool
extern pthread_t threads[NUM_THREADS];
extern pthread_mutex_t lock;
extern pthread_cond_t work_cond;
extern bool keep_working;
extern bool work_available;


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

bool termCollidesWithBoardCharacters(termInBoard term) {
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

pair *findIntersectionsBetweenTerms(term term1, term term2, int *numPairs) {
    pair *intersections = NULL;
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

void calculateStartPosition(coordinate *starts, termInBoard *termInBoard, pair intersection) {
    if (!termInBoard->isHorizontal) {
        starts->row = termInBoard->starts.row + intersection.second;
        starts->column = termInBoard->starts.column - intersection.first;
    } else {
        starts->row = termInBoard->starts.row - intersection.first;
        starts->column = termInBoard->starts.column + intersection.second;
    }
}

void shuffleIntersections(int numPairs, int *indices) {
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

void addTermToCrosswordBoard(termInBoard term) {
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

bool tryToPlaceATerm() {
    // TODO: Ad concurrency set to enhance performance
    pthread_mutex_lock(&lock);
    // Threads simultaneously generate a term
    int size = rand() % 6 + 4;  // Random term size between 4 and 9
    term newTerm = getRandomTerm(size);

    bool success = false;
    if (isTermAlreadyInBoard(newTerm)) {
        pthread_mutex_unlock(&lock);
        return false;
    }

    for (int i = numberOfTerms; i >= 0 && !success; i--){
        if (termsInBoard[i].term.word != NULL) {  // Ensure the term is not NULL
            int numPairs;
            pair* intersections = findIntersectionsBetweenTerms(newTerm, termsInBoard[i].term, &numPairs);
            if (numPairs <= 0) {
                continue;
            }

            int* indices = malloc(numPairs * sizeof(int));
            if (!indices) {  // Memory allocation failure handling
                perror("Failed to allocate memory for indices");
                exit(EXIT_FAILURE);
            }
            shuffleIntersections(numPairs, indices);

            for (int j = 0; j < numPairs; j++) {
                pair intersection = intersections[indices[j]];
                coordinate starts;
                bool isHorizontal = !termsInBoard[i].isHorizontal;
                calculateStartPosition(&starts, &termsInBoard[i], intersection);

                if (termFitsInBoard(newTerm, starts, isHorizontal) && !termCollidesWithBoardCharacters((termInBoard){newTerm, isHorizontal, starts, false})) {
                    for (int k = 1; k < numberOfTerms; k++) {
                        if (termsInBoard[k].term.word != NULL) {
                            continue;
                        }
                        termInBoard newTermInBoard = {newTerm, isHorizontal, starts, false};
                        termsInBoard[k] = newTermInBoard;
                        addTermToCrosswordBoard(newTermInBoard);
                        success = true;
                        break;
                    }
                }
            }
            free(intersections);
        }
    }

    pthread_mutex_unlock(&lock);
    return success;
}


void *worker_function(void *arg) {
    while (true) {
        pthread_mutex_lock(&lock);
        while (!work_available && keep_working) {
            pthread_cond_wait(&work_cond, &lock);
        }
        if (!keep_working) {  // Check if it's time to shut down the pool
            pthread_mutex_unlock(&lock);
            break;
        }
        work_available = false;  // Reset work availability
        pthread_mutex_unlock(&lock);

        // Existing logic to place a term
        if (tryToPlaceATerm()) {
            pthread_mutex_lock(&lock);
            work_available = true;
            pthread_cond_broadcast(&work_cond);  // Signal other threads that there's more work
            pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}


