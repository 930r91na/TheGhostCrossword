#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>
#include "fileUtils.h"
#include "initBoard.h"

#define boardSize 11
#define numberOfTerms 12

// Global variables
char crosswordBoard[boardSize][boardSize];
termInBoard termsInBoard[numberOfTerms];

// Init thread pool
#define NUM_THREADS 4
pthread_t threads[NUM_THREADS];
pthread_mutex_t lock;
pthread_cond_t work_cond;
bool keep_working = true;
bool work_available = false;

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

void start_thread_pool() {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&work_cond, NULL);

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, worker_function, NULL);
    }
}

void stop_thread_pool() {
    pthread_mutex_lock(&lock);
    keep_working = false;
    pthread_cond_broadcast(&work_cond);  // Wake up all threads to let them exit
    pthread_mutex_unlock(&lock);

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&work_cond);
}


void initializeTermsInBoard() {
    // Initialize empty board with '*'
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            crosswordBoard[i][j] = '*';
        }
    }

    srand(time(NULL));
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
    }
    printf("User is ready. Waiting for game initialization...\n");
    exit(0);
}

int main(void) {

    pid_t pidGivesInstructions;
    int processStatus;

    pidGivesInstructions = fork();
    if (pidGivesInstructions == 0) {
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

    return 0;
}




/*
 term randomTerm = getRandomTerm(9);
    printf("Random term: %s\n", randomTerm.word);
    printf("Description: %s\n", randomTerm.description);

 * */