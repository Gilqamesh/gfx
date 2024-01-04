#include "vec_math.h"

#include <stdio.h>
#include <stdint.h>

#include <x86intrin.h>

#define vec2__print(a) printf("%s: (vec2) {%.3f %.3f}\n", #a, a._[0], a._[1])
#define vec3__print(a) printf("%s: (vec3) {%.3f %.3f %.3f}\n", #a, a._[0], a._[1], a._[2])
#define vec4__print(a) printf("%s: (vec4) {%.3f %.3f %.3f %.3f}\n", #a, a._[0], a._[1], a._[2], a._[3])
#define mat4__print(a) do { \
    mat4_t result = (a);\
    printf("%s {\n", #a); \
    for (int i = 0; i < 4; ++i) { \
        printf("  "); \
        for (int j = 0; j < 4; ++j) { \
            printf("%10.3f ", result._[i * 4 + j]); \
        } \
        printf("\n"); \
    } \
    printf("}\n"); \
} while (0)

#define quat__print(a) { printf("%s: (quat) {%.3f %.3f %.3f %.3f}\n", #a, a._[0], a._[1], a._[2], a._[3]); }

int main() {
    // vec2_t a = vec2(1.0f, 0.0f);
    // vec2_t b = vec2(0.0f, 2.0f);
    // vec2__print(a);
    // vec2__print(b);
    // vec2__print(vec2__dup(a));
    // vec2__print(vec2__add(a, b));
    // vec2__print(vec2__sub(a, b));
    // vec2__print(vec2__mul(a, 1.2));
    // vec2__print(vec2__div(a, 1.2));
    // vec2__print(vec2__neg(a));
    // printf("%20s: %.3f\n", "vec2__inner(a, b)", vec2__inner(a, b));
    // vec2__print(vec2__norm(a));
    // vec2__print(vec2__reflect(a, b));
    // vec2__print(vec2__refract(a, b, 1.0f, 1.0f));
    // vec2__print(vec2__refract(a, b, 1.0f, 1.5f));
    // vec2__print(vec2__refract(a, b, 2.0f, 1.0f));
    // printf("%20s: %.3f\n", "vec2__len(a)", vec2__len(a));
    // printf("%20s: %.3f\n", "vec2__dist(a, b)", vec2__dist(a, b));
    // printf("%20s: %.3f\n", "vec2__angl(a, b)", vec2__angl(a, b));

    // vec3_t c = vec3(2.0f, -2.0f, 0.0f);
    // vec3_t d = vec3(2.2f, 3.f, -4.f);
    // vec3__print(c);
    // vec3__print(d);
    // vec3__print(vec3__dup(c));
    // vec3__print(vec3__add(c, d));
    // vec3__print(vec3__sub(c, d));
    // vec3__print(vec3__mul(c, 1.2));
    // vec3__print(vec3__div(c, 1.2));
    // vec3__print(vec3__neg(c));
    // printf("%20s: %.3f\n", "vec3__inner(c, d)", vec3__inner(c, d));
    // vec3__print(vec3__outer(c, d));
    // vec3__print(vec3__norm(c));
    // vec3__print(vec3__reflect(c, d));
    // vec3__print(vec3__refract(c, d, 1.0f, 1.0f));
    // printf("%20s: %.3f\n", "vec3__len(c)", vec3__len(c));

    // vec4_t e = vec4(1.2f, 2.f, -3.f, 2.3f);
    // vec4_t f = vec4(2.2f, 3.f, -4.f, -1.f);
    // vec4__print(e);
    // vec4__print(f);
    // vec4__print(vec4__dup(e));
    // vec4__print(vec4__add(e, f));
    // vec4__print(vec4__sub(e, f));
    // vec4__print(vec4__neg(e));
    // printf("%20s: %.3f\n", "vec4__inner(e, f)", vec4__inner(e, f));
    // vec4__print(vec4__norm(e));
    // printf("%20s: %.3f\n", "vec4__len(a)", vec4__len(e));

    // mat4_t m1 = (mat4_t){
    //     1.2, 5.4, 23, -23,
    //     230, -4.2, 3.f, 23,
    //     1, 2, 3, 0,
    //     -2, 2, 2, 3
    // };
    // mat4_t m2 = (mat4_t){
    //     2, 3, 4, 5,
    //     -1, -2, -3, 0,
    //     4, 2, 1, 6,
    //     0, 0, 1, 1
    // };
    // mat4__print(m1);
    // mat4__print(m2);
    // mat4__print(mat4__null());
    // mat4__print(mat4__id());
    // mat4__print(mat4__dup(m1));
    // mat4__print(mat4__add(m1, m2));
    // mat4__print(mat4__sub(m1, m2));
    // mat4__print(mat4__mul(m1, m2));
    // vec4__print(mat4__mul_vec4(m1, e));
    // mat4__print(mat4__tranlate(m1, c));
    // vec3__norm_self(&c);
    // mat4__print(mat4__rot(c, 1.2f));

    // vec3__print(vec3__rot(c, vec3(0.0f, 0.0f, 1.0f), 3.1415f / 2.0f));

    vec3_t e = vec3(6, 4, 5);
    vec3_t p = vec3(3, 1.5, 2);
    vec3__print(e);
    vec3__print(p);

    // uint64_t time_total = 0;
    // uint32_t runs = 10000;
    // for (uint32_t i = 0; i < runs; ++i) {
    //     uint64_t time_start = __rdtsc();
    //     vec3__lerp(e, p, 1.0f);
    //     uint64_t time_end   = __rdtsc();
    //     time_total += time_end - time_start;
    // }
    // printf("%.3lf\n", (double) time_total / (double) runs);

    vec3__print(vec3__lerp(e, p, 2.0f));

    return 0;
}
