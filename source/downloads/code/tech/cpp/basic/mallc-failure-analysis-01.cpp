#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

namespace {

constexpr size_t kGiB = 1024ULL * 1024 * 1024;
// §3.4 主探测点：> CommitLimit 且 ≤ RAM+Swap（mode 0 不拒绝）→ 15872 MiB = 15.5 GiB
constexpr size_t kPassCapMiB = 15872;
// 对照：> RAM+Swap，由 mode 0 拒绝
constexpr size_t kFailCapGiB = 16; // 16 GiB = 16384 MiB

static long read_meminfo_kb(const char* key)
{
    std::ifstream f("/proc/meminfo");
    std::string line;
    while (std::getline(f, line)) {
        if (line.find(key) == 0) {
            long v = 0;
            sscanf(line.c_str(), "%*s %ld", &v);
            return v;
        }
    }
    return -1;
}

// §3.1：确认失败发生在 mmap 层而非 malloc 层
static void probe_mmap_vs_malloc()
{
    size_t sz = 64 * kGiB;
    errno = 0;
    void* mp = mmap(NULL, sz, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    printf("mmap(64 GiB):   %s errno=%d (%s)\n",
           mp == MAP_FAILED ? "FAIL" : "OK", errno, strerror(errno));
    if (mp != MAP_FAILED) munmap(mp, sz); // 释放本次 mmap 申请的虚拟内存

    errno = 0;
    void* p = malloc(sz);
    printf("malloc(64 GiB): %s errno=%d (%s)\n",
           p ? "OK" : "NULL", errno, strerror(errno));
    if (p) free(p); // 释放本次 malloc 申请的内存
}

// §3.5：MAP_NORESERVE 绕过承诺检查，确认失败不是地址碎片化
static void probe_noreserve()
{
    for (size_t gb : {16ULL, 64ULL}) {
        size_t sz = gb * kGiB;
        errno = 0;
        void* d = mmap(NULL, sz, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        printf("%zu GiB default:   %s errno=%d\n",
               gb, d == MAP_FAILED ? "FAIL" : "OK", errno);
        if (d != MAP_FAILED) munmap(d, sz); // 释放本次 mmap 申请的虚拟内存

        errno = 0;
        void* n = mmap(NULL, sz, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
        printf("%zu GiB NORESERVE: %s errno=%d\n",
               gb, n == MAP_FAILED ? "FAIL" : "OK", errno);
        if (n != MAP_FAILED) munmap(n, sz); // 释放本次 mmap 申请的虚拟内存
    }
}

// §3.3：探测 overcommit 检查的边界（RAM+Swap 附近逐步测试）
static void probe_threshold()
{
    // 15360 MiB=15 GiB；15872 MiB=15.5 GiB；15880 MiB=15.5078125 GiB；
    // 16384 MiB=16 GiB；65536 MiB=64 GiB
    for (int mb : {15360, 15872, 15880, 16384, 65536}) {
        size_t sz = static_cast<size_t>(mb) * 1024 * 1024;
        errno = 0;
        void* p = mmap(NULL, sz, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        printf("%5d MiB: %s errno=%d\n",
               mb, p == MAP_FAILED ? "FAIL" : "OK", errno);
        if (p != MAP_FAILED) munmap(p, sz); // 释放本次 mmap 申请的虚拟内存
    }
}

// §3.4：观察 Committed_AS 变化，排除 mode 2 累计限制
// 主探测：15872 MiB（> CommitLimit 且 ≤ RAM+Swap）；对照：16 GiB（> RAM+Swap）
static void probe_committed()
{
    auto kib_to_gib = [](long kib) {
        return static_cast<double>(kib) / (1024.0 * 1024.0);
    };
    // Committed_AS：AS = Address Space（地址空间承诺量），随 mmap/munmap 等动态升降
    auto committed = []() { return read_meminfo_kb("Committed_AS:"); };

    long limit = read_meminfo_kb("CommitLimit:");
    printf("CommitLimit: %ld KiB (%.2f GiB)\n", limit, kib_to_gib(limit));

    // ① 15872 MiB = 15.5 GiB：> CommitLimit 且 ≤ RAM+Swap → 应通过；若 Committed_AS 超过 CommitLimit 仍成功 → 非 mode 2
    long c0 = committed();
    size_t sz_ok = kPassCapMiB * 1024ULL * 1024; // 15872 MiB = 15.5 GiB
    void* p_ok = mmap(NULL, sz_ok, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    long c1 = committed();
    printf("mmap(15872 MiB / 15.5 GiB): %s  Committed_AS %ld KiB (%.2f GiB) -> %ld KiB (%.2f GiB)\n",
           p_ok == MAP_FAILED ? "FAIL" : "OK",
           c0, kib_to_gib(c0), c1, kib_to_gib(c1));
    if (p_ok != MAP_FAILED) munmap(p_ok, sz_ok); // 释放本次 mmap 申请的虚拟内存

    // ② 16 GiB = 16384 MiB：> RAM+Swap → mode 0 拒绝，且 Committed_AS 应不变
    c0 = committed();
    size_t sz_fail = kFailCapGiB * kGiB; // 16 GiB = 16384 MiB
    void* p_fail = mmap(NULL, sz_fail, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    c1 = committed();
    printf("mmap(16384 MiB / 16 GiB): %s  Committed_AS %ld KiB (%.2f GiB) -> %ld KiB (%.2f GiB)\n",
           p_fail == MAP_FAILED ? "FAIL" : "OK",
           c0, kib_to_gib(c0), c1, kib_to_gib(c1));
    if (p_fail != MAP_FAILED) munmap(p_fail, sz_fail); // 释放本次 mmap 申请的虚拟内存
}

} // namespace

int main(int argc, char* argv[])
{
    const char* mode = argc > 1 ? argv[1] : "all";
    if (!strcmp(mode, "mmap")      || !strcmp(mode, "all")) probe_mmap_vs_malloc();
    if (!strcmp(mode, "threshold") || !strcmp(mode, "all")) probe_threshold();
    if (!strcmp(mode, "commit")    || !strcmp(mode, "all")) probe_committed();
    if (!strcmp(mode, "noreserve") || !strcmp(mode, "all")) probe_noreserve();
    return 0;
}
