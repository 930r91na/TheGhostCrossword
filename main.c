#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "fileUtils.h"
#include "initBoard.h"

#define boardSize 11
#define numberOfTerms 12

// Global variables
char crosswordBoard[boardSize][boardSize];
termInBoard termsInBoard[numberOfTerms];

bool checkAllTermsInBoard() {
    int t = 0;
    for (int i = 0; i < numberOfTerms; i++) {
        if (termsInBoard[i].term.word != NULL) {
            t++;
        }
    }

    if (t == numberOfTerms) {
        return true;
    }
    return false;
}
void initializeTermsInBoard() {
    // Init the first word in the middle
    termInBoard firstTermInBoard;
    firstTermInBoard.term = getRandomTerm(9);
    // Random start position
    firstTermInBoard.starts.row = 1;
    firstTermInBoard.starts.column = 1;
    firstTermInBoard.isHorizontal = true;
    firstTermInBoard.isKnown = false;
    firstTermInBoard.index = 0;
    termsInBoard[0] = firstTermInBoard;
    addTermToCrosswordBoard(firstTermInBoard);

    pthread_t threads[numberOfTerms];


    while (!checkAllTermsInBoard()) {
        generateAndPlaceTerm(NULL);
    }
}

void printAnsweredBoard() {
    char crosswordBoardAnswered[boardSize][boardSize];

    // Initialize the board with '*'
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            crosswordBoardAnswered[i][j] = '*';
        }
    }

    // Fill the board with the known terms
    for (int i = 0; i < numberOfTerms; i++) {
        coordinate starts = termsInBoard[i].starts;
        if (termsInBoard[i].isHorizontal) {
            for (int j = 0; j < strlen(termsInBoard[i].term.word); j++) {
                crosswordBoardAnswered[starts.row][starts.column + j] = termsInBoard[i].term.word[j];
            }
        } else {
            for (int j = 0; j < strlen(termsInBoard[i].term.word); j++) {
                crosswordBoardAnswered[starts.row + j][starts.column] = termsInBoard[i].term.word[j];
            }
        }
    }

    // Print the board
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            printf("%c", crosswordBoardAnswered[i][j]);
        }
        printf("\n");
    }
}

void printTermDescription(termInBoard termInBoard) {
    printf("Term %d: %s\n", termInBoard.index, termInBoard.term.description);
}

void printTermsHints() {
    // TODO: Print all terms hints (using threads)
}

void userAnswer() {
    int termNumber;
    printf("Enter the term number to answer: ");
    scanf("%d", &termNumber);
    printf("Enter the answer: ");
    char answer[100];
    scanf("%s", answer);

    // Check if the answer is correct
}

int main(void) {
    // Initialize empty board with '*'
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            crosswordBoard[i][j] = '*';
        }
    }

    // Test getRandomTerm
    initializeTermsInBoard();

    printAnsweredBoard();


    return 0;
}




/*
 term randomTerm = getRandomTerm(9);
    printf("Random term: %s\n", randomTerm.word);
    printf("Description: %s\n", randomTerm.description);

 * */