#include <stdio.h>
#include <stdarg.h>

/* 打印非负整数序列，约定以 -1 作为结束哨兵 */
void print_numbers(int first, ...) {
    va_list args;
    va_start(args, first);

    int current = first;
    while (current != -1) {
        printf("%d ", current);
        current = va_arg(args, int);
    }

    va_end(args);
    printf("\n");
}

int main(void) {
    print_numbers(10, 20, 30, 40, -1); /* 10 20 30 40 */
    return 0;
}
