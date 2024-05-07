#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "gameLogicUtils.h"
#include "initBoard.h"
#include "parameters.h"
#include "fileUtils.h"

void searchPotentialReplacement(int left, int right, coordinate coordinate, char have);

void giveUserInstructions() {
    char buffer;
    printf("Welcome to the Crossword Game!\n");
    printf("Rules: Complete the board with correct terms.\n");
    printf("Press 'y' when ready to start, or 'e' to exit: ");
    scanf(" %c", &buffer);
    while (buffer != 'y' && buffer != 'e') {
        printf("Invalid input. Please press 'y' to start or 'e' to exit: ");
        scanf(" %c", &buffer);
    }
    if (buffer == 'e') {
        printf("Exiting game as requested by user.\n");
        // Kill the main process
        kill(getppid(), SIGKILL);
    } else {
        printf("User is ready. Waiting for game initialization...\n");
        exit(0);
    }
}

void printAnsweredBoard() {
    // Print the board
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("%c", crosswordBoardWithAnswers[i][j]);
        }
        printf("\n");
    }
}

bool isTermInHistory(char *word) {
    for (int i = 0; i < historyOfWordsIndex; i++) {
        if (strcmp(historyOfWords[i], word) == 0) {
            return true;
        }
    }
    return false;
}

bool checkAllTermsInBoard() {
    int t = 0;
    for (int i = 0; i < NUMBER_OF_TERMS; i++) {
        if (termsInBoard[i].term.word != NULL) {
            t++;
        }
    }
    if (t == NUMBER_OF_TERMS) {
        return true;
    }
    return false;
}

void printTermsHints() {
    printf("Hints:\n");
    // Printing all terms in board divided in horizontal and vertical
    printf("HORIZONTAL: \n");
    for (int i = 0; i < NUMBER_OF_TERMS; i++) {
        if (termsInBoard[i].isHorizontal) {
            printf("Term %d: %s\n", termsInBoard[i].index, termsInBoard[i].term.description);
        }
    }

    printf("VERTICAL: \n");
    for (int i = 0; i < NUMBER_OF_TERMS; i++) {
        if (!termsInBoard[i].isHorizontal) {
            printf("Term %d: %s\n", termsInBoard[i].index, termsInBoard[i].term.description);
        }
    }
}

void findAllIntersections(pair **intersections, int *numPairs, termInBoard param) {
    for (int i = 0; i < NUMBER_OF_TERMS; i++) {
        if (param.index == i) {
            continue;
        }
        pair *pairs;
        int numPairsPerTerm = 0;
        pairs = findIntersectionsBetweenTerms(param.term, termsInBoard[i].term, &numPairsPerTerm);
        // Allocation of memory for the new pairs
        if (numPairsPerTerm > 0) {
            *numPairs += numPairsPerTerm;
            *intersections = realloc(*intersections, *numPairs * sizeof(pair));
            for (int j = 0; j < numPairsPerTerm; j++) {
                (*intersections)[*numPairs - numPairsPerTerm + j] = pairs[j];
            }
        }
    }
}

// Generate a potential termToAppear

void termToAppearGenerator() {
    termInBoard tempTermToAppear;
    int indexTerm = rand() % NUMBER_OF_TERMS;

    while (termsInBoard[indexTerm].isKnown) {
        indexTerm = rand() % NUMBER_OF_TERMS;
    }

    tempTermToAppear.index = indexTerm;
    tempTermToAppear.isKnown = false;

    termInBoard termToReplace = termsInBoard[indexTerm];

    // Add something to be in constant check

    char toHave = termToReplace.term.word[termToReplace.intersection];
    int spaceUpLeft = 0;
    int spaceDownRight = 0;
    coordinate intersectionCoordinate;

    if (termsInBoard[indexTerm].isHorizontal) {
        spaceUpLeft = termToReplace.intersection - 1;
        spaceDownRight = strlen(termToReplace.term.word) - termToReplace.intersection;

        // Get extra space that may exist
        coordinate temp = termToReplace.starts;
        while (crosswordBoardWithAnswers[temp.row][temp.column--] == '*') {
            spaceUpLeft++;
        }

        while (crosswordBoardWithAnswers[temp.row][temp.column++ + strlen(termToReplace.term.word) - 1] == '*') {
            spaceDownRight++;
        }

    } else {
        spaceUpLeft = termToReplace.intersection - 1;
        spaceDownRight = strlen(termToReplace.term.word) - termToReplace.intersection;


        intersectionCoordinate.row = termToReplace.starts.row + termToReplace.intersection - 1;
        intersectionCoordinate.column = termToReplace.starts.column;

        // Get extra space that may exist
        // Get extra space that may exist
        coordinate temp = termToReplace.starts;
        while (crosswordBoardWithAnswers[temp.row--][temp.column] == '*') {
            spaceUpLeft++;
        }

        while (crosswordBoardWithAnswers[temp.row++ + strlen(termToReplace.term.word) - 1][temp.column] == '*') {
            spaceDownRight++;
        }
    }

    searchPotentialReplacement(spaceUpLeft, spaceDownRight, intersectionCoordinate, toHave);
}

void searchPotentialReplacement(int left, int right, coordinate coordinate, char have) {
    int maxSizeOfWord = left + right + 1;


}

int answerChecker(char answer[10]) {
    // Check if the answer is correct in any of the terms
    for (int i = 0; i < NUMBER_OF_TERMS; i++) {
        if (strcmp(termsInBoard[i].term.word, answer) == 0) {
            return termsInBoard[i].index;
        }
    }
    return -1;
}

void converterToUpperCase(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = str[i] - 32;
        }
    }
}

bool processUserAnswer() {
    int termNumber;
    printf("Enter your answer: ");
    char answer[10];
    scanf("%s", answer);

    converterToUpperCase(answer);

    if (answerChecker(answer) != -1) {
        termNumber = answerChecker(answer);
        printf("Correct answer! Term %d found.\n", termNumber);
        termsInBoard[termNumber].isKnown = true;
        return true;
    } else {
        printf("Incorrect answer. Try again.\n");
        return false;
    }
}

void printFormattedBoard(char displayBoard[BOARD_SIZE][BOARD_SIZE][20]) {
    // Print the top border of the board
    printf("+");
    for (int j = 0; j < BOARD_SIZE; j++) {
        printf("-------+");
    }
    printf("\n");

    // Print the board with formatting
    for (int i = 0; i < BOARD_SIZE; i++) {
        // Print each row with vertical dividers
        printf("|");
        for (int j = 0; j < BOARD_SIZE; j++) {
            // Ensure each cell has a uniform width
            printf(" %-5s |", displayBoard[i][j]);
        }
        printf("\n");

        // Print the row border
        printf("+");
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("-------+");
        }
        printf("\n");
    }
    // TODO: Add colors to the board
}

