#include "threadsUtils.h"
#include "parameters.h"

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

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&work_cond);
}