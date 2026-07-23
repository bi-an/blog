#include <numa.h>
#include <numaif.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main() {
    // 检查NUMA是否可用
    if (numa_available() < 0) {
        printf("NUMA不可用\n");
        return 1;
    }

    // 获取NUMA节点数量
    int max_node = numa_max_node();
    printf("系统有 %d 个NUMA节点\n", max_node + 1);

    // 获取当前进程运行的节点
    int current_node = numa_node_of_cpu(sched_getcpu());
    printf("当前CPU属于节点: %d\n", current_node);

    // 分配内存到指定NUMA节点
    size_t size = 1024 * 1024 * 100; // 100MB
    void *mem = numa_alloc_onnode(size, 0); // 在节点0上分配内存
    
    if (mem == NULL) {
        perror("内存分配失败");
        return 1;
    }

    // 获取内存所在的节点
    int mem_node;
    if (get_mempolicy(&mem_node, NULL, 0, mem, MPOL_F_NODE | MPOL_F_ADDR) == 0) {
        printf("分配的内存位于节点: %d\n", mem_node);
    }

    // 使用内存...
    memset(mem, 0, size);

    // 释放内存
    numa_free(mem, size);

    return 0;
}

