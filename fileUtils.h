
#ifndef THEGHOSTCROSSWORD_FILEUTILS_H
#define THEGHOSTCROSSWORD_FILEUTILS_H

#include <pthread.h>
#include "initBoard.h"
#include "parameters.h"

extern pthread_mutex_t getRandomTerm_lock;

int countLines(int fd);

int getTermFromLine(int fd, int line_number, term* t);

term getRandomTerm(int size);

term searchReplacement(coordinate * start, int  *intersection, int left, int right, coordinate coordinate, char have);

#endif //THEGHOSTCROSSWORD_FILEUTILS_H
