#include "libc.h"

#include <stdbool.h>
#include <float.h>
#include <ctype.h>

double strntod(const char* str, uint32_t str_len) {
    double result_integral_part = 0.0;
    double result_decimal_part  = 0.0;
    bool is_negative = *str == '-';

    uint32_t index = is_negative ? 1 : 0;
    char c = str[index];
    while (
        index < str_len &&
        c != '\0' &&
        c != '.' &&
        isdigit(c)
    ) {
        double cur_digit = c - '0';
        if (result_integral_part - cur_digit / 10.0 > DBL_MAX / 10.0) {
            break ;
        }
        result_integral_part *= 10.0;
        result_integral_part += cur_digit;

        c = str[++index];
    }

    if (c == '.') {
        c = str[++index];
        double cur_divisor = 10.0;
        while (
            index < str_len &&
            c != '\0' &&
            c != '.' &&
            isdigit(c)
        ) {
            double cur_digit = c - '0';
            if (cur_digit < DBL_MIN * cur_divisor) {
                break ;
            }
            result_decimal_part += cur_digit / cur_divisor;

            if (cur_divisor > DBL_MAX / 10) {
                break ;
            }
            cur_divisor *= 10;
            c = str[++index];
        }
    }

    double result = result_integral_part + result_decimal_part;

    return is_negative ? -result : result;
}