/*
 * thread-stack-32bit-demo.c
 *
 * 演示：ulimit -s 过大时，有限虚拟地址空间内线程创建失败的过程。
 *
 * ============================================================
 * 测试 32-bit 虚拟地址受限场景的两种方法
 * ============================================================
 *
 * 【方法一】真正编译为 32-bit 二进制（最真实）
 *
 *   前提：需安装 32-bit 开发库
 *     RHEL/CentOS:  sudo yum install glibc-devel.i686 libgcc.i686
 *     Ubuntu/Debian: sudo apt install gcc-multilib
 *
 *   编译：
 *     gcc -m32 -o thread-stack-32bit-demo thread-stack-32bit-demo.c -lpthread
 *
 *   运行：进程天然受 32-bit 用户态 ~3 GB 地址空间限制，无需 ulimit -v
 *     ulimit -s 262144    # 每线程栈 256 MB
 *     ./thread-stack-32bit-demo
 *
 * ------------------------------------------------------------
 *
 * 【方法二】64-bit 二进制 + ulimit -v 模拟（无需额外依赖）
 *
 *   ulimit -v 限制进程的最大虚拟地址空间（RLIMIT_AS），
 *   效果与 32-bit 地址空间受限完全等价：mmap 超出上限时同样返回 EAGAIN，
 *   pthread_create 因无法为新线程栈分配地址而失败。
 *
 *   编译：
 *     gcc -o thread-stack-32bit-demo thread-stack-32bit-demo.c -lpthread
 *
 *   运行：
 *     ulimit -v 3145728   # 限制虚拟地址空间为 3 GB（模拟 32-bit 用户态上限）
 *     ulimit -s 262144    # 每线程栈 256 MB
 *     ./thread-stack-32bit-demo
 *
 * ============================================================
 * 对照组（正常 ulimit -s，两种方法均适用）
 * ============================================================
 *   ulimit -v 3145728
 *   ulimit -s 8192      # 正常值：8 MB/线程，同样地址空间可创建 300+ 线程
 *   ./thread-stack-32bit-demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/resource.h>

static volatile int keep_running = 1;

/* 从 /proc/self/status 读取 VmSize 和 VmRSS，单位 MB */
static void print_mem(int n_threads) {
    FILE *f = fopen("/proc/self/status", "r");
    if (!f) return;
    char line[256];
    long vmsize_kb = -1, vmrss_kb = -1;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "VmSize:", 7) == 0) sscanf(line + 7, "%ld", &vmsize_kb);
        if (strncmp(line, "VmRSS:",  6) == 0) sscanf(line + 6, "%ld", &vmrss_kb);
    }
    fclose(f);
    printf("  [%3d threads]  Virt = %6ld MB   RSS = %5ld MB\n",
           n_threads, vmsize_kb / 1024, vmrss_kb / 1024);
    fflush(stdout);
}

static void *thread_func(void *arg) {
    (void)arg;
    while (keep_running)
        sleep(1);
    return NULL;
}

int main(void) {
    /* 读取当前 ulimit 设置 */
    struct rlimit rl_stack, rl_as;
    getrlimit(RLIMIT_STACK, &rl_stack);
    getrlimit(RLIMIT_AS,    &rl_as);

    long stack_mb = (long)(rl_stack.rlim_cur / 1024 / 1024);
    long as_mb    = (rl_as.rlim_cur == RLIM_INFINITY)
                    ? -1
                    : (long)(rl_as.rlim_cur / 1024 / 1024);

    printf("========================================\n");
    printf("ulimit -s (stack/thread) = %ld MB\n", stack_mb);
    if (as_mb < 0)
        printf("ulimit -v (virtual addr) = unlimited\n");
    else
        printf("ulimit -v (virtual addr) = %ld MB\n", as_mb);

    if (as_mb > 0 && stack_mb > 0)
        printf("理论最大线程数 ≈ %ld / %ld = %ld\n",
               as_mb, stack_mb, as_mb / stack_mb);
    printf("========================================\n\n");

    print_mem(0);   /* 基线 */

    pthread_t threads[1024];
    int i;
    for (i = 0; i < 1024; i++) {
        int ret = pthread_create(&threads[i], NULL, thread_func, NULL);
        if (ret != 0) {
            const char *meaning = (ret == EAGAIN) ? "EAGAIN: 虚拟地址空间耗尽，无法为线程栈执行 mmap"
                                : (ret == ENOMEM) ? "ENOMEM: 物理内存不足"
                                :                   "其他错误";
            printf("\npthread_create 在第 %d 个线程时失败\n", i + 1);
            printf("  errno = %d (%s)\n", ret, strerror(ret));
            printf("  含义 : %s\n", meaning);
            printf("成功创建线程数: %d\n", i);
            print_mem(i);
            keep_running = 0;
            for (int j = 0; j < i; j++)
                pthread_join(threads[j], NULL);
            return 1;
        }
        print_mem(i + 1);
    }

    printf("达到测试上限 1024 个线程，未触发失败。\n");
    keep_running = 0;
    for (int j = 0; j < i; j++)
        pthread_join(threads[j], NULL);
    return 0;
}
