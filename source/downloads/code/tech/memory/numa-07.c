#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4

void *worker_thread(void *arg) {
    int thread_id = *(int *)arg;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    
    // 每个线程绑定到不同的CPU
    CPU_SET(thread_id, &cpuset);
    
    // 设置线程的CPU亲和性
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) != 0) {
        perror("pthread_setaffinity_np failed");
        return NULL;
    }
    
    // 获取当前运行的CPU
    int cpu = sched_getcpu();
    printf("线程 %d 运行在CPU %d\n", thread_id, cpu);
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, worker_thread, &thread_ids[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    return 0;
}

