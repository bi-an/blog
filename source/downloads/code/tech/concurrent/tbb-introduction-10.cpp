#include <tbb/tbb.h>
#include <iostream>

void taskA() {
    std::cout << "Task A executed by thread " << tbb::this_task_arena::current_thread_index() << std::endl;
    tbb::parallel_invoke(
         {
            std::cout << "Subtask A1 executed by thread " << tbb::this_task_arena::current_thread_index() << std::endl;
        },
         {
            std::cout << "Subtask A2 executed by thread " << tbb::this_task_arena::current_thread_index() << std::endl;
        }
    );
}

void taskB() {
    std::cout << "Task B executed by thread " << tbb::this_task_arena::current_thread_index() << std::endl;
    tbb::parallel_invoke(
         {
            std::cout << "Subtask B1 executed by thread " << tbb::this_task_arena::current_thread_index() << std::endl;
        },
         {
            std::cout << "Subtask B2 executed by thread " << tbb::this_task_arena::current_thread_index() << std::endl;
        }
    );
}

int main() {
    tbb::task_arena arena;
    arena.execute([&] {
        tbb::this_task_arena::isolate([&] {
            tbb::parallel_invoke(taskA, taskB);
        });
    });
    return 0;
}