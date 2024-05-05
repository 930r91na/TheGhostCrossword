#include "fileUtils.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

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

term getRandomTerm(int size) {
    term randomTerm;

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

    // Generate a random line number
    srand(time(NULL));
    int randomLine = (rand() % (numLines - 1)) + 2;  // Generate a random number between 2 and numLines

    // Get the term at the random line number
    if (getTermFromLine(fd, randomLine, &randomTerm) == -1) {
        printf("Error getting term from line\n");
    }

    // Close the file
    close(fd);

    return randomTerm;
}