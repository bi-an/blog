#include <stdio.h>
#include <stdarg.h>

/* 包装器：只做 va_start / va_end，参数解析交给 vprintf */
void log_message(const char *level, const char *format, ...) {
    va_list args;
    va_start(args, format);

    printf("[%s]: ", level);
    vprintf(format, args);
    printf("\n");

    va_end(args);
}

int main(void) {
    log_message("INFO", "User %s logged in from %s", "Alice", "192.168.1.1");
    return 0;
}
