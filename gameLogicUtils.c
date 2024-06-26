#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "gameLogicUtils.h"
#include "initBoard.h"
#include "parameters.h"


bool isdigit(char i);

void giveUserInstructions() {
    char buffer;
    printf(BLU "Welcome to the Crossword Game!\n");
    printf("Rules: Complete the board with correct terms" RESET "\n");
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

int max(int a, int b){
    return a > b ? a : b;
}

void printTermsHints() {
    printf("Hints:\n");
    // Storing all terms in arrays
    char horizontalTerms[NUMBER_OF_TERMS][50];
    char verticalTerms[NUMBER_OF_TERMS][50];
    int hCount = 0, vCount = 0;

    for (int i = 0; i < NUMBER_OF_TERMS; i++) {
        if (termsInBoard[i].isHorizontal && !termsInBoard[i].isKnown) {
            sprintf(horizontalTerms[hCount++], "Term %d: %s", termsInBoard[i].index, termsInBoard[i].term.description);
        } else if (!termsInBoard[i].isHorizontal && !termsInBoard[i].isKnown) {
            sprintf(verticalTerms[vCount++], "Term %d: %s", termsInBoard[i].index, termsInBoard[i].term.description);
        }
    }

    // Printing terms in columns
    printf("VERTICAL \t\t\t\t\t\t\t\t\tHORIZONTAL\n");
    for (int i = 0; i < max(hCount, vCount); i++) {
        if (i < vCount) {
            printf("%-50s", verticalTerms[i]);
        } else {
            printf("%-s", "");
        }

        if (i < hCount) {
            printf("%30s", horizontalTerms[i]);
        }
        printf("\n");
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

void *termToAppearGenerator(void *args) {
    while (true) {
        // Constantly checking if a termToAppear is in need
        if (termToAppear.term.word != NULL && !termsInBoard[termToAppear.index].isKnown) {
            continue;
        }

        termInBoard tempTermToAppear;
        int indexTerm = rand() % (NUMBER_OF_TERMS - 1) + 1;

        // Find a term that is not known and is different from the term to appear
        while (termsInBoard[indexTerm].isKnown ||
               (tempTermToAppear.term.word != NULL && strcmp(termsInBoard[indexTerm].term.word, tempTermToAppear.term.word) == 0)) {

            indexTerm = rand() % (NUMBER_OF_TERMS - 1) + 1;
        }

        tempTermToAppear.term.word = NULL;
        tempTermToAppear.index = indexTerm;
        tempTermToAppear.isKnown = false;


        termInBoard termToReplace = termsInBoard[indexTerm];

        // Parameters to search a word with the common intersection
        char toHave = termToReplace.term.word[termToReplace.intersection];
        int spaceUpLeft = 0;
        int spaceDownRight = 0;
        coordinate intersectionCoordinate;
        spaceUpLeft = termToReplace.intersection;
        spaceDownRight = strlen(termToReplace.term.word) - termToReplace.intersection - 1;

        // Get global position a coordinate
        if (termsInBoard[indexTerm].isHorizontal) {
            intersectionCoordinate.row = termToReplace.starts.row;
            intersectionCoordinate.column = termToReplace.starts.column + termToReplace.intersection;

            // Get extra space that may exist
            coordinate temp = termToReplace.starts;
            while (temp.column > 0 && crosswordBoardWithAnswers[temp.row][temp.column-- - 1] == '*') {
                spaceUpLeft++;
            }
            temp = termToReplace.starts;
            while (temp.row < BOARD_SIZE &&
                   crosswordBoardWithAnswers[temp.row][temp.column++ + strlen(termToReplace.term.word)] == '*') {
                spaceDownRight++;
            }
        } else {
            intersectionCoordinate.row = termToReplace.starts.row + termToReplace.intersection;
            intersectionCoordinate.column = termToReplace.starts.column;

            // Get extra space that may exist
            coordinate temp = termToReplace.starts;
            while (temp.row > 0 && crosswordBoardWithAnswers[temp.row-- - 1][temp.column] == '*') {
                spaceUpLeft++;
            }
            temp = termToReplace.starts;
            while (temp.column < BOARD_SIZE && crosswordBoardWithAnswers[temp.row++ + strlen(termToReplace.term.word)][temp.column] == '*') {
                spaceDownRight++;
            }
        }


        coordinate start;
        int tleft = 0;

        // Search a valid candidate

        do{
            tempTermToAppear.term = searchReplacement(&start, &tleft, spaceUpLeft, spaceDownRight,
                                                      intersectionCoordinate, toHave);
        } while (strcmp(tempTermToAppear.term.word, termToReplace.term.word) == 0 || isTermAlreadyInBoard(tempTermToAppear.term));
        // If no term was found, continue
        if (tempTermToAppear.term.word == NULL) {
            continue;
        }
        // Assign valid candidate to global termToAppear

        tempTermToAppear.isHorizontal = termToReplace.isHorizontal;
        // Get Intersection and starts
        if (tempTermToAppear.isHorizontal) {
            tempTermToAppear.starts.row = termsInBoard[indexTerm].starts.row;
            tempTermToAppear.starts.column = start.column + 1;

        } else {
            tempTermToAppear.starts.column = termsInBoard[indexTerm].starts.column;
            tempTermToAppear.starts.row = start.row + 1;
        }
        tempTermToAppear.intersection = tleft;
        termToReplaceIndex = indexTerm;
        if (termCollidesWithBoardCharacters(tempTermToAppear)) {
            continue;
        }
        termToAppear = tempTermToAppear;
    }
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
    char answer[11];
    if (reInitBoard){
        reInitBoard = false;
        return false;
    }
    scanf("%10s", answer);

    converterToUpperCase(answer);

    if (answerChecker(answer) != -1) {
        termNumber = answerChecker(answer);
        clockTime = 0;
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
    printf("\n+");
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
            if (strcmp(displayBoard[i][j], "-") == 0) {
                // If the string is "-", print in white
                printf(WHT " %-5s " RESET "|", displayBoard[i][j]);
            } else if (isdigit(displayBoard[i][j][0])) {
                // If the first character is a digit, print in red
                printf(RED " %-5s " RESET "|", displayBoard[i][j]);
            } else {
                // If the first character is a letter, print in green
                printf(GRN " %-5s " RESET "|", displayBoard[i][j]);
            }
        }
        printf("\n");

        // Print the row border
        printf("+");
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("-------+");
        }
        printf("\n");
    }
}

bool isdigit(char i) {
    return i >= '0' && i <= '9';
}

