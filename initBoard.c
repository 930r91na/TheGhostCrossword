#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <malloc.h>

#include "fileUtils.h"
#include "initBoard.h"
#include "parameters.h"

// TODO: Make boardsize dynamic
// TODO: Make the generation between horizontal and vertical terms distributed equally
// TODO: Make the word check for space between words
// Global variables
extern char crosswordBoardWithAnswers[BOARD_SIZE][BOARD_SIZE];
extern termInBoard termsInBoard[NUMBER_OF_TERMS];
extern char **historyOfWords;
extern int historyOfWordsIndex;

// Global variable to control the thread pool
extern pthread_t threads[NUM_THREADS];
extern pthread_mutex_t lock;
extern pthread_cond_t work_cond;
extern bool keep_working;
extern bool work_available;

bool isTermAlreadyInBoard(struct term term) {
    for (int i = 0; i < NUMBER_OF_TERMS; i++) {
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
        if (starts.column + strlen(term.word) >= BOARD_SIZE) {
            return false;
        }
    } else {
        if (starts.row + strlen(term.word) >= BOARD_SIZE) {
            return false;
        }
    }
    return true;
}

bool termCollidesWithBoardCharacters(termInBoard term) {
    int termLength = strlen(term.term.word);
    if (term.isHorizontal) {
        for (int i = 0; i < termLength; i++) {
            char boardChar = crosswordBoardWithAnswers[term.starts.row][term.starts.column + i];
            if (boardChar != '*' && boardChar != term.term.word[i]) {
                return true;
            }
        }
    } else {
        for (int i = 0; i < termLength; i++) {
            char boardChar = crosswordBoardWithAnswers[term.starts.row + i][term.starts.column];
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
            crosswordBoardWithAnswers[term.starts.row][term.starts.column + l] = term.term.word[l];
        }
    } else {
        for (int l = 0; l < strlen(term.term.word); l++) {
            crosswordBoardWithAnswers[term.starts.row + l][term.starts.column] = term.term.word[l];
        }
    }
}

_Thread_local term newTerm;
// Main function to place a term in the board

bool  connectToOneWord(termInBoard param) {
    // Check the placed word just connect with its intersection
        int temp = 0;
        for (int i = 0; i < strlen(param.term.word); ++i) {
            if (param.isHorizontal){
                if (crosswordBoardWithAnswers[param.starts.row][param.starts.column + i] != '*'){
                    if (crosswordBoardWithAnswers[param.starts.row][param.starts.column + i] != param.term.word[i]){
                        temp++;
                    }
                }
            }else{
                if (crosswordBoardWithAnswers[param.starts.row + i][param.starts.column] != '*'){
                    if (crosswordBoardWithAnswers[param.starts.row + i][param.starts.column] != param.term.word[i]){
                        temp++;
                    }
                }
            }
        }

        if (temp > 1){
            return false;
        }else   {
            return true;
        }
}

bool tryToPlaceATerm() {
    // TODO: Add concurrency set to enhance performance
    // Threads simultaneously generate a term
    int size = rand() % 6 + 4;  // Random term size between 4 and 9
     newTerm = getRandomTerm(size);

    pthread_mutex_lock(&lock);
    bool success = false;

    if (isTermAlreadyInBoard(newTerm)) {
        pthread_mutex_unlock(&lock);
        return false;
    }

    // Check if the term works in the board
    for (int i = NUMBER_OF_TERMS; i >= 0 && !success; i--){
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
                if (success) {
                    break;
                }

                pair intersection = intersections[indices[j]];
                coordinate starts;
                bool isHorizontal = !termsInBoard[i].isHorizontal;
                calculateStartPosition(&starts, &termsInBoard[i], intersection);

                if (termFitsInBoard(newTerm, starts, isHorizontal) && !termCollidesWithBoardCharacters((termInBoard){newTerm, isHorizontal, starts, false}) && connectToOneWord((termInBoard){newTerm, isHorizontal, starts, false})) {
                    for (int k = 1; k < NUMBER_OF_TERMS; k++) {
                        if (termsInBoard[k].term.word != NULL) {
                            continue;
                        }
                        termInBoard newTermInBoard = {newTerm, isHorizontal, starts, false, k, intersection.first};
                        termsInBoard[k] = newTermInBoard;
                        addTermToCrosswordBoard(newTermInBoard);
                        // TODO: Fix memory allocation of historyOfWords
                        // historyOfWords[historyOfWordsIndex ++] = newTerm.word;
                        success = true;
                        break;
                    }
                }
            }
            free(intersections);
            free(indices);
        }
    }

    pthread_mutex_unlock(&lock);
    return success;
}


void *worker_function(void *arg) {
    while (true) {
        pthread_mutex_lock(&lock); // Lock the mutex (necessary for condition variable)
        // Wait for work to be available with condition variable
        while (!work_available && keep_working) {
            pthread_cond_wait(&work_cond, &lock);
        }

        // Check if it's time to shut down the pool
        if (!keep_working) {
            pthread_mutex_unlock(&lock);
            break;
        }

        // Reset work availability
        work_available = false;
        pthread_mutex_unlock(&lock);

        // Existing logic to place a term
        if (tryToPlaceATerm()) {
            pthread_mutex_lock(&lock);
            work_available = true;
            pthread_cond_broadcast(&work_cond);  // Signal other threads that there's more work - equivalent to pthread_cond_signal
            pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}


