#include <stdio.h>
#include <unistd.h>

#include "test_if.h"

int main() {
    int i = 0;
    while (1) {
        printf("%4d ", i);
        i = next(i);
        if (i % 10 == 0) {
            printf("\n");
        } else {
            fflush(stdout);
        }
        usleep(100000);
    }

    return 0;
}
