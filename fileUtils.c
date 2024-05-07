#include "fileUtils.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

int countLines(int fd) {
    int lines = 0;
    char buffer[1024];
    int bytes_read;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                lines++;
            }
        }
    }
    lseek(fd, 0, SEEK_SET);
    return lines;
}

// Implementation of get_term_from_line
int getTermFromLine(int fd, int line_number, term* t) {
    lseek(fd, 0, SEEK_SET);

    char buffer[1024];
    int current_line = 1;
    int bytes_read;
    int start_of_line = 1;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                if (current_line == line_number) {
                    buffer[i] = 0; // Null-terminate the line
                    char* comma = strchr(buffer + start_of_line, ',');
                    if (!comma) return -1;
                    *comma = 0; // Split word and description
                    t->word = strdup(buffer + start_of_line);
                    t->description = strdup(comma + 2);
                    return 0;
                }
                current_line++;
                start_of_line = i + 1;
            }
        }
    }
    return -1; // Line not found
}

_Thread_local unsigned int seed;
_Thread_local term randomTerm;

term getRandomTerm(int size) {
    // Construct the file name
    char fileName[50];
    sprintf(fileName, "../PoolOfWords/wordsSize%d", size);

    // Open the file
    int fd = open(fileName, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return randomTerm;
    }

    // Count the number of lines in the file
    int numLines = countLines(fd);

    // Generate a random line number with micros as seed
    struct timeval time;
    gettimeofday(&time, NULL);
    long micros = (unsigned long long) time.tv_sec * 1000000 + time.tv_usec;
    seed = micros;
    int randomLine = (rand_r(&seed) % (numLines - 1)) + 2;  // Generate a random number between 2 and numLines


    // Get the term at the random line number
    if (getTermFromLine(fd, randomLine, &randomTerm) == -1) {
        printf("Error getting term from line\n");
    }

    // Close the file
    close(fd);
    return randomTerm;
}

term searchReplacement(coordinate * start, int * intersectionIndex, int left, int right, coordinate coordinate, char have) {
    int maxSizeOfWord = left + right + 1;
    term t;

    // Create and shuffle a list of word sizes
    int sizes[6] = {4, 5, 6, 7, 8, 9};
    for (int i = 0; i < 6; i++) {
        int j = rand() % 6;
        int temp = sizes[i];
        sizes[i] = sizes[j];
        sizes[j] = temp;
    }

    for (int i = 0; i < 6; i++) {
        if (sizes[i] > maxSizeOfWord) {
            continue;
        }

        // Open file with words of size i
        char fileName[50];
        sprintf(fileName, "../PoolOfWords/wordsSize%d", sizes[i]);

        int fd = open(fileName, O_RDONLY);
        if (fd == -1) {
            perror("Error opening file");
            return t;
        }

        // Count the number of lines in the file
        int numLines = countLines(fd);

        // Generate a random start line
        int startLine = (rand() % (numLines - 2)) + 2;

        for (int j = 0; j < numLines - 1; j++) {
            int line = (startLine + j) % (numLines - 1) + 2;

            if (getTermFromLine(fd, line, &t) == -1) {
                printf("Error getting term from line\n");
            }

            // Checks that the term has the letter needed and enough space
            int tleft = 0;
            int tright;
            for (int k = 0; k < strlen(t.word); k++) {
                tleft++;
                if (t.word[k] == have) {
                    tright = strlen(t.word) - tleft;

                    if (tleft <= left && tright <= right) {
                        intersectionIndex = &tleft;
                        start->row = coordinate.row - tleft;
                        start->column = coordinate.column - tleft;
                        close(fd);
                        return t;
                    }
                }
            }
        }

        close(fd);
    }
    return t;
}