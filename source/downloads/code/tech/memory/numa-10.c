#include <numa.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

#define NUM_THREADS 4
#define ARRAY_SIZE 1000000

typedef struct {
    int thread_id;
    int numa_node;
    double *array;
    int size;
} thread_data_t;

void *worker_thread(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    
    // 方法1：手动设置CPU亲和性
    struct bitmask *cpuset = numa_allocate_cpumask();
    numa_node_to_cpus(data->numa_node, cpuset);
    numa_sched_setaffinity(0, cpuset);
    numa_free_cpumask(cpuset);
    
    // 在指定节点上分配内存
    data->array = (double *)numa_alloc_onnode(
        data->size * sizeof(double), data->numa_node);
    
    if (data->array == NULL) {
        fprintf(stderr, "线程 %d: 内存分配失败\n", data->thread_id);
        return NULL;
    }
    
    // 执行计算（使用本地内存）
    for (int i = 0; i < data->size; i++) {
        data->array[i] = i * 1.0;
    }
    
    int cpu = sched_getcpu();
    printf("线程 %d 在CPU %d (节点 %d) 上完成计算\n", 
           data->thread_id, cpu, data->numa_node);
    
    return NULL;
}

int main() {
    if (numa_available() < 0) {
        fprintf(stderr, "NUMA不可用\n");
        return 1;
    }
    
    int max_node = numa_max_node();
    pthread_t threads[NUM_THREADS];
    thread_data_t thread_data[NUM_THREADS];
    
    // 创建线程，每个线程分配到不同的NUMA节点
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].numa_node = i % (max_node + 1);
        thread_data[i].size = ARRAY_SIZE;
        
        pthread_create(&threads[i], NULL, worker_thread, &thread_data[i]);
    }
    
    // 等待所有线程完成
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        if (thread_data[i].array) {
            numa_free(thread_data[i].array, 
                     thread_data[i].size * sizeof(double));
        }
    }
    
    return 0;
}

