#ifndef THREAD_H
# define THREAD_H

# include <stdint.h>

struct         thread;
struct         mutex;
typedef struct thread* thread_t;
typedef struct mutex* mutex_t;

thread_t thread__create(
    void (*worker_fn)(void* user_data),
    void* user_data
);

void thread__destroy(thread_t self);

void thread__start_execution(thread_t self);
void thread__wait_execution(thread_t self);

void thread__cancel_execution(thread_t self);
void thread__test_cancel();

mutex_t mutex__create();
void mutex__destroy(mutex_t self);

void mutex__lock(mutex_t self);
void mutex__unlock(mutex_t self);

#endif // THREAD_H
