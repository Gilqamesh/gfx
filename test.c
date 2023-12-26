#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

int main() {
    uint8_t a = 12;
    uint32_t b = 32;
    uint8_t res = (uint8_t) (a - b);
    printf("%u\n", res);

    return 0;
}
