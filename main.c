// Georgina Zeron Cabrera 174592
// Last modification: 06-05-2024
// Crossword game using threads
// V 1.0.0
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>

#include "parameters.h"
#include "fileUtils.h"
#include "initBoard.h"
#include "threadsUtils.h"
#include "gameLogicUtils.h"

// Global variables
char crosswordBoardWithAnswers[BOARD_SIZE][BOARD_SIZE];
termInBoard termsInBoard[NUMBER_OF_TERMS];
termInBoard termToAppear;
char **historyOfWords;
int historyOfWordsIndex = 0;

// Init thread pool
pthread_t threads[NUM_THREADS];
pthread_mutex_t lock;
pthread_cond_t work_cond;
pthread_mutex_t getRandomTerm_lock;
bool keep_working = true;
bool work_available = false;

// Threads to get termToAppear
pthread_t termToAppearThread;

// Clock
pthread_t clockThread;
int clockTime = 0;


void findAllIntersections(pair **ptr, int *pInt, termInBoard param);

void initializeTermsInBoard() {
    // Initialize empty board with '*'
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            crosswordBoardWithAnswers[i][j] = '*';
        }
    }

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

    start_thread_pool();  // Start the thread pool

    // Signal work until all terms are placed
    while (!checkAllTermsInBoard()) {
        pthread_mutex_lock(&lock);
        work_available = true;
        pthread_cond_broadcast(&work_cond);  // Signal to threads that there is work
        pthread_mutex_unlock(&lock);
    }

    stop_thread_pool();  // Stop the thread pool when done
}

void termChangeHandler(int signal) {
    printf("Signal received: %d\n", signal);
    // Get random term index between 0 and 11
    // TODO: Change this to get
    int indexTermToReplace = rand() % 12;
    while (termsInBoard[indexTermToReplace].isKnown) {
        indexTermToReplace = rand() % 12;
    }

    printf("Term to replace: %d\n", indexTermToReplace);
    //replaceTerm(termsInBoard[indexTermToReplace]);
}

_Noreturn void *gameClock(void *arg) {
    while (true) {
        sleep(1);
        clockTime++;

        if (clockTime % WORD_DISAPPEAR_TIME == 0) {
            printf("TicToc TicToc in 15 seconds! A word will be replaced.\n");
            alarm(15);
        }
    }
}

void initCrosswordBoard() {
    char displayBoard[BOARD_SIZE][BOARD_SIZE][20];

    // Initialize the display board with '-'
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            strcpy(displayBoard[i][j], "-");
        }
    }

    // Populate the board with terms
    for (int t = 0; t < NUMBER_OF_TERMS; t++) {
        termInBoard current = termsInBoard[t];
        int len = strlen(current.term.word);

        for (int l = 0; l < len; l++) {
            int x = current.starts.row + (current.isHorizontal ? 0 : l);
            int y = current.starts.column + (current.isHorizontal ? l : 0);

            if (current.isKnown) {
                char letter[2] = {current.term.word[l], '\0'};
                strcpy(displayBoard[x][y], letter);
            } else {
                if (strcmp(displayBoard[x][y], "-") == 0) {
                    // If the cell is initially empty, add the index
                    sprintf(displayBoard[x][y], "%d", current.index);
                } else {
                    // If not, append the new index, indicating an intersection
                    char buffer[20];
                    sprintf(buffer, "%s-%d", displayBoard[x][y], current.index);
                    strcpy(displayBoard[x][y], buffer);
                }
            }
        }
    }

    printFormattedBoard(displayBoard);
}

int main(void) {
    srand(time(NULL));
    signal(SIGALRM, termChangeHandler);

    pid_t pidGivesInstructions;
    int processStatus;

    if ((pidGivesInstructions = fork()) == 0) {
        giveUserInstructions();
        exit(0);
    }

    initializeTermsInBoard();

    waitpid(pidGivesInstructions, &processStatus, 0);
    if (!WIFEXITED(processStatus)) {
        printf("Error: User instructions did not complete successfully.\n");
        return -1;
    }

    printAnsweredBoard();

    // Init game clock
    pthread_create(&clockThread, NULL, gameClock, NULL);
    // Main Game loop
    while (true) {
        initCrosswordBoard();
        printTermsHints();

        processUserAnswer();
    }

    return 0;
}