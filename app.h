#ifndef APP_H
# define APP_H

struct app;
typedef struct app* app_t;

app_t app__create(int argc, char* argv[]);
void app__destroy(app_t self);

void app__run(app_t self);

#endif // APP_H
