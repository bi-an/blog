#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    
    // 设置CPU亲和性：绑定到CPU 0和1
    CPU_SET(0, &cpuset);
    CPU_SET(1, &cpuset);
    
    // 设置当前进程的CPU亲和性
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) != 0) {
        perror("sched_setaffinity failed");
        return 1;
    }
    
    // 验证设置结果
    CPU_ZERO(&cpuset);
    if (sched_getaffinity(0, sizeof(cpuset), &cpuset) != 0) {
        perror("sched_getaffinity failed");
        return 1;
    }
    
    printf("进程绑定到CPU: ");
    for (int i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, &cpuset)) {
            printf("%d ", i);
        }
    }
    printf("\n");
    
    return 0;
}

