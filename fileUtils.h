
#ifndef THEGHOSTCROSSWORD_FILEUTILS_H
#define THEGHOSTCROSSWORD_FILEUTILS_H

typedef struct term{
    char* word;
    char* description;
} term;

int countLines(int fd);

int getTermFromLine(int fd, int line_number, term* t);

term getRandomTerm(int size);

#endif //THEGHOSTCROSSWORD_FILEUTILS_H
