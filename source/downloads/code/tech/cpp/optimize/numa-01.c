// affinity_test.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 200

void* worker_thread(void* arg) {
    int thread_id = *(int*)arg;
    unsigned long long count = 0;
    
    // 执行一些计算工作
    for (int i = 0; i < 100000000; i++) {
        count += i;
    }
    
    printf("Thread %d completed, count = %llu\n", thread_id, count);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    printf("Creating %d threads...\n", NUM_THREADS);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, worker_thread, &thread_ids[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("All threads completed\n");
    return 0;
}

