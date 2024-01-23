#include "thread.h"

#include <pthread.h>
#include <stdlib.h>

struct mutex {
    pthread_mutex_t _;
};

typedef struct thread_user_data {
    void (*worker_fn)(void* user_data);
    void* user_data;
} thread_user_data_t;

struct thread {
    pthread_t                _;
    mutex_t                  mutex_start_execution;
    thread_user_data_t       user_data;
};

static void* thread__execute_worker_fn(void* user_data);

static void* thread__execute_worker_fn(void* user_data) {
    thread_t thread = (thread_t) user_data;
    mutex__lock(thread->mutex_start_execution);
    thread->user_data.worker_fn(thread->user_data.user_data);

    pthread_testcancel();

    return 0;
}

thread_t thread__create(
    void (*worker_fn)(void* user_data),
    void* user_data
) {
    thread_t result = calloc(1, sizeof(*result));
    result->user_data.user_data = user_data;
    result->user_data.worker_fn = worker_fn;
    result->mutex_start_execution = mutex__create();
    mutex__lock(result->mutex_start_execution);
    if (pthread_create(&result->_, 0, &thread__execute_worker_fn, result) != 0) {
        free(result);
        return 0;
    }
    return result;
}

void thread__destroy(thread_t self) {
    pthread_join(self->_, 0);
    thread__wait_execution(self);
    mutex__destroy(self->mutex_start_execution);
    free(self);
}

void thread__start_execution(thread_t self) {
    mutex__unlock(self->mutex_start_execution);
}

void thread__wait_execution(thread_t self) {
    pthread_join(self->_, 0);
}

void thread__cancel_execution(thread_t self) {
    pthread_cancel(self->_);
}

void thread__test_cancel() {
    pthread_testcancel();
}

mutex_t mutex__create() {
    mutex_t result = calloc(1, sizeof(*result));

    if (pthread_mutex_init(&result->_, 0) != 0) {
        free(result);
        return 0;
    }

    return result;
}

void mutex__destroy(mutex_t self) {
    pthread_mutex_destroy(&self->_);
}

void mutex__lock(mutex_t self) {
    pthread_mutex_lock(&self->_);
}

void mutex__unlock(mutex_t self) {
    pthread_mutex_unlock(&self->_);
}
