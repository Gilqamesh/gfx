#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

typedef enum any_type {
    S32,
    R64
} any_type_t;

typedef struct any {
    any_type_t t;
    union {
        int    a;
        double b;
    } _;
} any_t;

typedef void (*operation_fn)(any_t* opt_out, ...);

void add_int(any_t* opt_out, ...) {
    va_list ap;
    va_start(ap, opt_out);
    any_t* left = va_arg(ap, any_t*);
    any_t* right = va_arg(ap, any_t*);
    opt_out->_.a = left->_.a + right->_.a;
    va_end(ap);
}

void add_double(any_t* opt_out, ...) {
    va_list ap;
    va_start(ap, opt_out);
    any_t* left = va_arg(ap, any_t*);
    any_t* right = va_arg(ap, any_t*);
    opt_out->_.b = left->_.b + right->_.b;
    va_end(ap);
}

void mul_int(any_t* opt_out, ...) {
    va_list ap;
    va_start(ap, opt_out);
    any_t* left = va_arg(ap, any_t*);
    any_t* right = va_arg(ap, any_t*);
    opt_out->_.a = left->_.a * right->_.a;
    va_end(ap);
}

void mul_double(any_t* opt_out, ...) {
    va_list ap;
    va_start(ap, opt_out);
    any_t* left = va_arg(ap, any_t*);
    any_t* right = va_arg(ap, any_t*);
    opt_out->_.b = left->_.b * right->_.b;
    va_end(ap);
}

void print_int(any_t* opt_out, ...) {
    va_list ap;
    va_start(ap, opt_out);
    any_t* value = va_arg(ap, any_t*);
    printf("%d\n", value->_.a);
    va_end(ap);
}

void print_double(any_t* opt_out, ...) {
    va_list ap;
    va_start(ap, opt_out);
    any_t* value = va_arg(ap, any_t*);
    printf("%lf\n", value->_.b);
    va_end(ap);
}

operation_fn add_dispatch(any_type_t type) {
    switch (type) {
    case S32: return &add_int;
    case R64: return &add_double;
    }
}

operation_fn mul_dispatch(any_type_t type) {
    switch (type) {
    case S32: return &mul_int;
    case R64: return &mul_double;
    }
}

operation_fn print_dispatch(any_type_t type) {
    switch (type) {
    case S32: return &print_int;
    case R64: return &print_double;
    }
}

enum op_type {
    ADD,
    MUL,
    PRINT
};

operation_fn dispatch_fn(enum op_type op, any_type_t type) {
    switch (op) {
    case ADD: return add_dispatch(type);
    case MUL: return mul_dispatch(type);
    case PRINT: return print_dispatch(type);
    }
}

int main() {
    any_t ia = {
        .t = S32,
        ._.a = -1
    };
    any_t ib = {
        .t = S32,
        ._.a = 6
    };

    any_t da = {
        .t = R64,
        ._.b = -1.3
    };
    any_t db = {
        .t = R64,
        ._.b = 12.23
    };

    any_t ic;
    operation_fn ifn = dispatch_fn(ADD, S32);
    ifn(&ic, &ia, &ib);
    operation_fn ifn2 = dispatch_fn(PRINT, S32);
    ifn2(0, &ic);

    any_t dc;
    operation_fn dfn = dispatch_fn(ADD, R64);
    dfn(&dc, &da, &db);
    operation_fn dfn2 = dispatch_fn(PRINT, R64);
    dfn2(0, &dc);

    return 0;
}
