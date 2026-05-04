#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 5

// Circular buffer
char buffer[BUFFER_SIZE];
int  in    = 0;   // next write slot
int  out   = 0;   // next read slot
int  count = 0;   // number of items currently in buffer

// Synchronization primitives
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  not_full  = PTHREAD_COND_INITIALIZER; // producer waits here
pthread_cond_t  not_empty = PTHREAD_COND_INITIALIZER; // consumer waits here

// Sentinel lets consumer know producer is done
int done = 0;

// Producer thread
void *producer(void *arg) {
    FILE *fp = fopen("message.txt", "r");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: cannot open message.txt\n");
        pthread_exit(NULL);
    }

    int ch;
    while ((ch = fgetc(fp)) != EOF) {

        pthread_mutex_lock(&mutex);

        // Buffer full, wait until consumer frees a slot
        while (count == BUFFER_SIZE)
            pthread_cond_wait(&not_full, &mutex);

        // Write character into the circular buffer
        buffer[in] = (char)ch;
        in = (in + 1) % BUFFER_SIZE;
        count++;

        // Wake the consumer
        pthread_cond_signal(&not_empty);

        pthread_mutex_unlock(&mutex);
    }

    fclose(fp);

    // Signal done so consumer can exit after draining the buffer
    pthread_mutex_lock(&mutex);
    done = 1;
    pthread_cond_signal(&not_empty); // wake consumer
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

// Consumer thread
void *consumer(void *arg) {
    while (1) {

        pthread_mutex_lock(&mutex);

        // Buffer empty, wait unless producer is finished
        while (count == 0 && !done)
            pthread_cond_wait(&not_empty, &mutex);

        // If buffer is empty AND producer is done, we're finished
        if (count == 0 && done) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Read character from the circular buffer
        char ch = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;

        // Wake the producer, there's at least one free slot now
        pthread_cond_signal(&not_full);

        pthread_mutex_unlock(&mutex);

        // Print outside the lock, no need to hold it for I/O
        putchar(ch);
    }

    putchar('\n');
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t prod_tid, cons_tid;

    pthread_create(&prod_tid, NULL, producer, NULL);
    pthread_create(&cons_tid, NULL, consumer, NULL);

    pthread_join(prod_tid, NULL);
    pthread_join(cons_tid, NULL);

    /* Clean up */
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);

    return 0;
}