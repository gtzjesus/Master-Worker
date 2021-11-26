#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

// Threads used
#define THREADS 2
sem_t semaphoreEmpty;
sem_t semaphoreFull;
pthread_mutex_t mutex;

int buffer[10];
int count = 0;

void* master(void* args) {
    while (1) {
        /* 
        master will produce the random number that
        will be used towards the buffer.
        */
        int x = rand() % 100;
        /*
        Sleep function used so we can reduce cpu time
        with values that are skipped.
        */
        sleep(1);
        /*
        master then adds to the buffer. 'sem_wait' is used until
        there's an empty slot. If 'semaphoreEmpty' = 0, there are no slots.
        Therefore we wait -> no point in proceeding
        */
        sem_wait(&semaphoreEmpty);
        /* 
        mutex locks are used to prevent master and worker to run exactly
        at the same time, we lock, then unlock for both threads.
        */        
        pthread_mutex_lock(&mutex);
        buffer[count] = x;
        count++;
        pthread_mutex_unlock(&mutex);
        /*
        once master passes the addition to buffer, we increment the
        semaphore so that we know the space is full.
        */
        sem_post(&semaphoreFull);
    }
}

void* worker(void* args) {
    while (1) {
        int y;
        /* 
        wroker will remove the used point in the buffer
        */
        sem_wait(&semaphoreFull);
        /* 
        mutex locks are used to prevent master and worker to run exactly
        at the same time, we lock, then unlock for both threads.
        */ 
        pthread_mutex_lock(&mutex);
        y = buffer[count - 1];
        count--;
        pthread_mutex_unlock(&mutex);
        sem_post(&semaphoreEmpty);
        /* 
        wroker will consume the available from buffer,
        then print it to the terminal
        */
        printf("Got %d\n", y);
        /*
        Sleep function used so we can reduce cpu time
        with values that are skipped.
        */
        sleep(1);
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    pthread_t thread[THREADS];
    pthread_mutex_init(&mutex, NULL);
    /*
    Here we initialize both semaphores, an empty and a full one.
    empty -> takes 10 as default because 10 are empty at declaration.
    full -> takes 0 as default because none are full at declaration.
    */
    sem_open(&semaphoreEmpty, 0, 10);
    sem_open(&semaphoreFull, 0, 0);
    int i;
    /* 
    these loops are used to create the threads for both the master
    and the worker functions(pthread_create).
    */
    for (i = 0; i < THREADS; i++) {
        if (i > 0) {
            if (pthread_create(&thread[i], NULL, &master, NULL) != 0) {
                perror("Could not implement thread");
            }
        } else {
            if (pthread_create(&thread[i], NULL, &worker, NULL) != 0) {
                perror("Could not implement thread");
            }
        }
    }
    /* 
    this loop is used to join both threads using 'pthread_join'
    */
    for (i = 0; i < THREADS; i++) {
        if (pthread_join(thread[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }
    /* 
    at the end of all execution of threads with buffer, we destroy 
    the mutex + both semaphores(then exit).
    */
    sem_close(&semaphoreEmpty);
    sem_close(&semaphoreFull);
    pthread_mutex_destroy(&mutex);
    return 0;
}