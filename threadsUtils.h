#ifndef THEGHOSTCROSSWORD_THREADSUTILS_H
#define THEGHOSTCROSSWORD_THREADSUTILS_H

#include <pthread.h>
#include <stdbool.h>
#include "parameters.h"
#include "initBoard.h"

// Global variables
extern pthread_t threads[NUM_THREADS];
extern pthread_mutex_t lock;
extern pthread_cond_t work_cond;
extern bool keep_working;

void start_thread_pool();

void stop_thread_pool();


#endif //THEGHOSTCROSSWORD_THREADSUTILS_H
