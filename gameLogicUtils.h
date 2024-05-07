
#ifndef THEGHOSTCROSSWORD_GAMELOGICUTILS_H
#define THEGHOSTCROSSWORD_GAMELOGICUTILS_H

#include <stdbool.h>
#include <string.h>

#include "initBoard.h"
#include "parameters.h"

extern char crosswordBoardWithAnswers[BOARD_SIZE][BOARD_SIZE];
extern termInBoard termsInBoard[NUMBER_OF_TERMS];
extern char **historyOfWords;
extern int historyOfWordsIndex;
extern termInBoard termToAppear;
extern int termToReplaceIndex;
extern int clockTime;
extern bool reInitBoard;

void giveUserInstructions();

void printAnsweredBoard();

bool isTermInHistory(char *word);

bool checkAllTermsInBoard();

void printTermsHints();

bool processUserAnswer();

void printFormattedBoard(char board[BOARD_SIZE][BOARD_SIZE][20]);

void *termToAppearGenerator(void *args);

#endif //THEGHOSTCROSSWORD_GAMELOGICUTILS_H
