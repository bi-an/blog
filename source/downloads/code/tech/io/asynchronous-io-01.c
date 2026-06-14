#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <liburing.h>

#define QUEUE_DEPTH  8
#define BUFFER_SIZE  1024

int main() {
    struct io_uring ring;
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;
    int ret, fd;
    char buf[BUFFER_SIZE];

    // 打开文件
    fd = open("test.txt", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // 初始化 io_uring
    ret = io_uring_queue_init(QUEUE_DEPTH, &ring, 0);
    if (ret < 0) {
        perror("io_uring_queue_init");
        return 1;
    }

    // 获取提交队列条目
    sqe = io_uring_get_sqe(&ring);
    if (!sqe) {
        fprintf(stderr, "io_uring_get_sqe failed\n");
        return 1;
    }

    // 准备读取操作
    io_uring_prep_read(sqe, fd, buf, BUFFER_SIZE, 0);

    // 提交到内核
    ret = io_uring_submit(&ring);
    if (ret < 0) {
        perror("io_uring_submit");
        return 1;
    }

    // 等待完成
    ret = io_uring_wait_cqe(&ring, &cqe);
    if (ret < 0) {
        perror("io_uring_wait_cqe");
        return 1;
    }

    // 读取结果
    if (cqe->res < 0) {
        fprintf(stderr, "Async read failed: %d\n", cqe->res);
    } else {
        printf("Read %d bytes: %.*s\n", cqe->res, cqe->res, buf);
    }

    // 通知内核完成
    io_uring_cqe_seen(&ring, cqe);

    // 关闭
    io_uring_queue_exit(&ring);
    close(fd);

    return 0;
}
