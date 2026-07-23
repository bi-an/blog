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
    double *array;
    int size;
} thread_data_t;

void *worker_thread(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    
    // 注意：numa_run_on_node在主线程中调用后，
    // 所有线程都会继承这个绑定，但每个线程仍可能运行在不同CPU上
    // 内存分配会优先在绑定的节点上进行（偏好设置，不强制保证）
    
    // 分配内存（会优先在绑定的节点上分配，但不保证）
    data->array = (double *)malloc(data->size * sizeof(double));
    
    if (data->array == NULL) {
        fprintf(stderr, "线程 %d: 内存分配失败\n", data->thread_id);
        return NULL;
    }
    
    // 执行计算
    for (int i = 0; i < data->size; i++) {
        data->array[i] = i * 1.0;
    }
    
    int cpu = sched_getcpu();
    int node = numa_node_of_cpu(cpu);
    printf("线程 %d 在CPU %d (节点 %d) 上完成计算\n", 
           data->thread_id, cpu, node);
    
    return NULL;
}

int main() {
    if (numa_available() < 0) {
        fprintf(stderr, "NUMA不可用\n");
        return 1;
    }
    
    int target_node = 0; // 所有线程绑定到节点0
    
    // 在主线程中绑定到目标节点
    // 这会影响到所有后续创建的线程
    if (numa_run_on_node(target_node) != 0) {
        perror("绑定到NUMA节点失败");
        return 1;
    }
    
    printf("主进程绑定到节点 %d，创建 %d 个工作线程\n", target_node, NUM_THREADS);
    
    pthread_t threads[NUM_THREADS];
    thread_data_t thread_data[NUM_THREADS];
    
    // 创建线程（会自动继承NUMA绑定）
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].size = ARRAY_SIZE;
        pthread_create(&threads[i], NULL, worker_thread, &thread_data[i]);
    }
    
    // 等待所有线程完成
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        if (thread_data[i].array) {
            free(thread_data[i].array);
        }
    }
    
    return 0;
}

