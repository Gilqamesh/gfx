#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

int32_t mod(int32_t a, int32_t b) {
    assert(b > 0);
    return (a % b + b) % b;
}

int main() {
    // (a % b)
    int32_t remainder = mod(-1, 90);
    printf("%d\n", remainder);

    return 0;
}
