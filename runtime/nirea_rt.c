/* Nirea runtime: output, string comparison and runtime errors. */
#include "nirea_rt.h"

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void nr_panic(const char *what, const char *loc) {
    fflush(stdout);
    fprintf(stderr, "nirea: runtime error: %s at %s\n", what, loc);
    exit(101);
}

void nr_unreachable(void) {
    nr_panic("unreachable code", "?");
}

void nr_print_i64(int64_t v) {
    printf("%" PRId64, v);
}

void nr_print_u64(uint64_t v) {
    printf("%" PRIu64, v);
}

void nr_print_bool(bool v) {
    fputs(v ? "true" : "false", stdout);
}

void nr_print_char(uint8_t v) {
    putchar(v);
}

void nr_print_str(nr_String s) {
    if (s.len > 0) fwrite(s.ptr, 1, (size_t)s.len, stdout);
}

void nr_print_newline(void) {
    putchar('\n');
}

bool nr_str_eq(nr_String a, nr_String b) {
    if (a.len != b.len) return false;
    return a.len == 0 || memcmp(a.ptr, b.ptr, (size_t)a.len) == 0;
}

double nr_fmod(double a, double b) {
    return fmod(a, b);
}

float nr_fmodf(float a, float b) {
    return fmodf(a, b);
}

/* Prints a floating-point value in the shortest form that reads back as the
 * same value (spec 15), in the style of Java's Double.toString: plain
 * notation for magnitudes in [1e-3, 1e7), otherwise d.dddE<exp>. */
static void print_float(double v, int max_digits, bool is_f32) {
    if (isnan(v)) {
        fputs("NaN", stdout);
        return;
    }
    if (isinf(v)) {
        fputs(v > 0 ? "Infinity" : "-Infinity", stdout);
        return;
    }
    if (v == 0) {
        fputs(signbit(v) ? "-0.0" : "0.0", stdout);
        return;
    }

    /* Find the shortest digit string that reads back as v. For each length,
     * try the correctly rounded decimal and its neighbours one unit away in
     * the last digit: near powers of two the rounding interval is lopsided,
     * so a neighbour can round-trip when the nearest doesn't. Among
     * round-tripping candidates of the shortest length, take the closest. */
    bool negative = v < 0;
    double mag = negative ? -v : v;
    char digits[40] = "0";
    int exponent = 0;
    for (int precision = 1; precision <= max_digits; precision++) {
        char buf[64];
        snprintf(buf, sizeof buf, "%.*e", precision - 1, mag);
        uint64_t mantissa = 0;
        const char *p = buf;
        for (; *p && *p != 'e'; p++) {
            if (*p != '.') mantissa = mantissa * 10 + (uint64_t)(*p - '0');
        }
        int exp10 = atoi(p + 1) - (precision - 1); /* value = mantissa * 10^exp10 */
        bool found = false;
        long double best_error = 0;
        for (int delta = -1; delta <= 1; delta++) {
            if (mantissa == 0 && delta < 0) continue;
            uint64_t m = mantissa + (uint64_t)(int64_t)delta;
            char cand[64];
            snprintf(cand, sizeof cand, "%" PRIu64 "e%d", m, exp10);
            bool same = is_f32 ? strtof(cand, NULL) == (float)mag : strtod(cand, NULL) == mag;
            if (!same) continue;
            long double diff = (long double)m * powl(10.0L, exp10) - (long double)mag;
            long double error = diff < 0 ? -diff : diff;
            if (!found || error < best_error) {
                found = true;
                best_error = error;
                snprintf(digits, sizeof digits, "%" PRIu64, m);
                exponent = exp10 + (int)strlen(digits) - 1;
            }
        }
        if (found) break;
    }
    int ndigits = (int)strlen(digits);
    while (ndigits > 1 && digits[ndigits - 1] == '0') digits[--ndigits] = '\0';

    if (negative) putchar('-');
    if (exponent >= -3 && exponent < 7) {
        if (exponent < 0) {
            fputs("0.", stdout);
            for (int i = -1; i > exponent; i--) putchar('0');
            fputs(digits, stdout);
        } else {
            for (int i = 0; i <= exponent; i++) putchar(i < ndigits ? digits[i] : '0');
            putchar('.');
            if (ndigits > exponent + 1) {
                fputs(digits + exponent + 1, stdout);
            } else {
                putchar('0');
            }
        }
    } else {
        putchar(digits[0]);
        putchar('.');
        fputs(ndigits > 1 ? digits + 1 : "0", stdout);
        printf("E%d", exponent);
    }
}

void nr_print_f32(float v) {
    print_float((double)v, 9, true);
}

void nr_print_f64(double v) {
    print_float(v, 17, false);
}
