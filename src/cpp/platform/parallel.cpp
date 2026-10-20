#include "parallel.h"

#include <algorithm>
#include <atomic>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <process.h>
#include <windows.h>
#endif

namespace intransitive {
namespace {

struct WorkQueue {
    std::size_t item_count;
    std::atomic<std::size_t> next_index;
    ParallelTask task;
    void* context;

    WorkQueue(std::size_t count, ParallelTask work, void* work_context)
        : item_count(count),
          next_index(0),
          task(work),
          context(work_context) {}
};

void run_pending_work(WorkQueue& queue) {
    for (;;) {
        const std::size_t index =
            queue.next_index.fetch_add(1, std::memory_order_relaxed);
        if (index >= queue.item_count) {
            return;
        }
        queue.task(index, queue.context);
    }
}

#ifdef _WIN32
unsigned __stdcall worker_entry(void* context) {
    run_pending_work(*static_cast<WorkQueue*>(context));
    return 0;
}
#endif

}

void parallel_for(
    std::size_t item_count,
    ParallelTask task,
    void* context
) {
    if (item_count == 0) {
        return;
    }
    if (task == NULL) {
        throw std::invalid_argument("La tarea paralela no puede estar vacía.");
    }

    WorkQueue queue(item_count, task, context);
#ifdef _WIN32
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    const std::size_t worker_limit = std::min<std::size_t>(
        item_count > 0 ? item_count - 1 : 0,
        std::min<std::size_t>(
            system_info.dwNumberOfProcessors > 0
                ? system_info.dwNumberOfProcessors - 1
                : 0,
            MAXIMUM_WAIT_OBJECTS
        )
    );

    std::vector<HANDLE> workers;
    workers.reserve(worker_limit);
    for (std::size_t index = 0; index < worker_limit; ++index) {
        const uintptr_t worker = _beginthreadex(
            NULL, 0, worker_entry, &queue, 0, NULL
        );
        if (worker == 0) {
            break;
        }
        workers.push_back(reinterpret_cast<HANDLE>(worker));
    }

    run_pending_work(queue);
    if (!workers.empty()) {
        const DWORD wait_result = WaitForMultipleObjects(
            static_cast<DWORD>(workers.size()),
            workers.data(),
            TRUE,
            INFINITE
        );
        for (HANDLE worker : workers) {
            CloseHandle(worker);
        }
        if (wait_result == WAIT_FAILED) {
            throw std::runtime_error(
                "No se pudieron esperar los trabajadores paralelos."
            );
        }
    }
#else
    run_pending_work(queue);
#endif
}

}
