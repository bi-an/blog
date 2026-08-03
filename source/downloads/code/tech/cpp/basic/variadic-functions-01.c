#include <stdio.h>
#include <stdarg.h>

/* count 为固定参数，表示后面跟随的可变参数个数 */
double calculate_average(int count, ...) {
    if (count <= 0) {
        return 0.0;
    }

    va_list args;
    double sum = 0.0;

    va_start(args, count);
    for (int i = 0; i < count; i++) {
        int value = va_arg(args, int);
        sum += value;
    }
    va_end(args);

    return sum / count;
}

int main(void) {
    double avg1 = calculate_average(4, 10, 20, 30, 40);
    printf("Average 1: %.2f\n", avg1); /* 25.00 */

    double avg2 = calculate_average(3, 5, 15, 25);
    printf("Average 2: %.2f\n", avg2); /* 15.00 */

    return 0;
}
