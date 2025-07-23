#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#define TAG_NAME "test2:fake_dlfcn"

#ifdef __arm__
#include <android/log.h>
#define log_info(fmt, args...) __android_log_print(ANDROID_LOG_INFO, TAG_NAME, fmt, ##args)
#define log_err(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, TAG_NAME, fmt, ##args)
#else
#define log_info(fmt, ...) do { \
    std::stringstream ss; \
    ss << "[INFO] " << std::format(fmt, ##__VA_ARGS__) << "\n"; \
    std::cout << ss.str(); \
} while (0)

#define log_err(fmt, ...) do { \
    std::stringstream ss; \
    ss << "[ERROR] " << std::format(fmt, ##__VA_ARGS__) << "\n"; \
    std::cerr << ss.str(); \
} while (0)
#endif

#ifdef LOG_DBG
#define log_dbg log_info
#else
#define log_dbg(...)
#endif

#ifdef __arm__
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Shdr Elf32_Shdr
#define Elf_Sym Elf32_Sym
#elif defined(__aarch64__) || defined(__x86_64__)
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Shdr Elf64_Shdr
#define Elf_Sym Elf64_Sym
#else
#error "Arch unknown, please port me"
#endif

struct Context {
    void *load_addr = nullptr;
    std::unique_ptr<char[]> dynstr;
    std::unique_ptr<Elf_Sym[]> dynsym;
    int nsyms = 0;
    off_t bias = 0;

    ~Context() {
        log_dbg("Context destroyed");
    }
};

int fake_dlclose(void *handle) {
    if (handle) {
        delete static_cast<Context *>(handle);
    }
    return 0;
}

void *fake_dlopen(const char *libpath, int flags) {
    std::ifstream maps("/proc/self/maps");
    if (!maps.is_open()) {
        log_err("Failed to open /proc/self/maps");
        return nullptr;
    }

    std::string line;
    std::string found_line;
    log_err("/proc/self/maps:\n");
    while (std::getline(maps, line)) {
        log_err("%s\n", line.c_str());
        if (line.find("r-xp") != std::string::npos && line.find(libpath) != std::string::npos) {
            found_line = line;
            break;
        }
    }

    if (found_line.empty()) {
        log_err("%s not found in my userspace", libpath);
        return nullptr;
    }

    off_t load_addr = 0;
    std::istringstream iss(found_line);
    iss >> std::hex >> load_addr;

    log_info("%s loaded in Android at 0x%08lx", libpath, load_addr);

    int fd = open(libpath, O_RDONLY);
    if (fd < 0) {
        log_err("Failed to open %s", libpath);
        return nullptr;
    }

    off_t size = lseek(fd, 0, SEEK_END);
    if (size <= 0) {
        log_err("lseek() failed for %s", libpath);
        close(fd);
        return nullptr;
    }

    auto elf = static_cast<Elf_Ehdr *>(mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0));
    close(fd);

    if (elf == MAP_FAILED) {
        log_err("mmap() failed for %s", libpath);
        return nullptr;
    }

    auto ctx = std::make_unique<Context>();
    ctx->load_addr = reinterpret_cast<void *>(load_addr);

    auto shoff = reinterpret_cast<char *>(elf) + elf->e_shoff;
    for (int k = 0; k < elf->e_shnum; ++k, shoff += elf->e_shentsize) {
        auto sh = reinterpret_cast<Elf_Shdr *>(shoff);
        log_dbg("%s: k=%d shdr=%p type=%x", __func__, k, sh, sh->sh_type);

        switch (sh->sh_type) {
            case SHT_DYNSYM:
                if (ctx->dynsym) {
                    log_err("%s: duplicate DYNSYM sections", libpath);
                    munmap(elf, size);
                    return nullptr;
                }
                ctx->dynsym = std::make_unique<Elf_Sym[]>(sh->sh_size / sizeof(Elf_Sym));
                std::memcpy(ctx->dynsym.get(), reinterpret_cast<char *>(elf) + sh->sh_offset, sh->sh_size);
                ctx->nsyms = sh->sh_size / sizeof(Elf_Sym);
                break;

            case SHT_STRTAB:
                if (ctx->dynstr) break;
                ctx->dynstr = std::make_unique<char[]>(sh->sh_size);
                std::memcpy(ctx->dynstr.get(), reinterpret_cast<char *>(elf) + sh->sh_offset, sh->sh_size);
                break;

            case SHT_PROGBITS:
                if (!ctx->dynstr || !ctx->dynsym) break;
                ctx->bias = static_cast<off_t>(sh->sh_addr) - static_cast<off_t>(sh->sh_offset);
                k = elf->e_shnum;  // exit loop
                break;
        }
    }

    munmap(elf, size);

    if (!ctx->dynstr || !ctx->dynsym) {
        log_err("Dynamic sections not found in %s", libpath);
        return nullptr;
    }

    log_dbg("%s: ok, dynsym = %p, dynstr = %p", libpath, ctx->dynsym.get(), ctx->dynstr.get());
    return ctx.release();
}

void *fake_dlsym(void *handle, const char *name) {
    auto ctx = static_cast<Context *>(handle);
    auto sym = ctx->dynsym.get();
    auto strings = ctx->dynstr.get();

    for (int k = 0; k < ctx->nsyms; ++k, ++sym) {
        if (std::strcmp(strings + sym->st_name, name) == 0) {
            void *ret = static_cast<char *>(ctx->load_addr) + sym->st_value - ctx->bias;
            log_info("%s found at %p", name, ret);
            return ret;
        }
    }
    return nullptr;
}