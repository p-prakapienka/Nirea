/* Nirea runtime: output, strings and runtime errors. */
#include "nirea_rt.h"

#include <errno.h>
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

static void *checked_realloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size);
    if (!p) nr_panic("out of memory", "?");
    return p;
}

/* ---- Formatting ---- */

/* Formats a floating-point value in the shortest form that reads back as the
 * same value (spec 15), in the style of Java's Double.toString: plain
 * notation for magnitudes in [1e-3, 1e7), otherwise d.dddE<exp>. out must
 * hold at least 40 bytes. */
static void format_float(char *out, double v, int max_digits, bool is_f32) {
    if (isnan(v)) {
        strcpy(out, "NaN");
        return;
    }
    if (isinf(v)) {
        strcpy(out, v > 0 ? "Infinity" : "-Infinity");
        return;
    }
    if (v == 0) {
        strcpy(out, signbit(v) ? "-0.0" : "0.0");
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

    char *o = out;
    if (negative) *o++ = '-';
    if (exponent >= -3 && exponent < 7) {
        if (exponent < 0) {
            *o++ = '0';
            *o++ = '.';
            for (int i = -1; i > exponent; i--) *o++ = '0';
            for (int i = 0; i < ndigits; i++) *o++ = digits[i];
        } else {
            for (int i = 0; i <= exponent; i++) *o++ = i < ndigits ? digits[i] : '0';
            *o++ = '.';
            if (ndigits > exponent + 1) {
                for (int i = exponent + 1; i < ndigits; i++) *o++ = digits[i];
            } else {
                *o++ = '0';
            }
        }
        *o = '\0';
    } else {
        *o++ = digits[0];
        *o++ = '.';
        if (ndigits > 1) {
            for (int i = 1; i < ndigits; i++) *o++ = digits[i];
        } else {
            *o++ = '0';
        }
        sprintf(o, "E%d", exponent);
    }
}

/* ---- Printing ---- */

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

void nr_print_str(const nr_String *s) {
    if (s->len > 0) fwrite(s->ptr, 1, (size_t)s->len, stdout);
}

void nr_print_newline(void) {
    putchar('\n');
}

void nr_print_f32(float v) {
    char buf[64];
    format_float(buf, (double)v, 9, true);
    fputs(buf, stdout);
}

void nr_print_f64(double v) {
    char buf[64];
    format_float(buf, v, 17, false);
    fputs(buf, stdout);
}

double nr_fmod(double a, double b) {
    return fmod(a, b);
}

float nr_fmodf(float a, float b) {
    return fmodf(a, b);
}

/* ---- Strings ---- */

static nr_String str_from(const char *bytes, size_t len) {
    nr_String s;
    s.cap = (int64_t)len + 1;
    s.ptr = checked_realloc(NULL, (size_t)s.cap);
    if (len) memcpy(s.ptr, bytes, len);
    s.ptr[len] = '\0';
    s.len = (int64_t)len;
    return s;
}

/* Makes s own a heap buffer with room for at least min_len bytes. */
static void str_reserve(nr_String *s, int64_t min_len) {
    if (s->cap > 0 && s->cap > min_len) return;
    int64_t cap = s->cap > 0 ? s->cap : 16;
    while (cap <= min_len) {
        if (cap > INT64_MAX / 2) nr_panic("out of memory", "?");
        cap *= 2;
    }
    if (s->cap > 0) {
        s->ptr = checked_realloc(s->ptr, (size_t)cap);
    } else {
        char *buf = checked_realloc(NULL, (size_t)cap);
        if (s->len) memcpy(buf, s->ptr, (size_t)s->len);
        buf[s->len] = '\0';
        s->ptr = buf;
    }
    s->cap = cap;
}

void nr_str_free(nr_String *s) {
    if (s->cap > 0) free(s->ptr);
    s->ptr = NULL;
    s->len = 0;
    s->cap = 0;
}

nr_String nr_str_copy(const nr_String *s) {
    if (s->cap == 0) return *s; /* static bytes can be shared until changed */
    return str_from(s->ptr, (size_t)s->len);
}

bool nr_str_eq(const nr_String *a, const nr_String *b) {
    if (a->len != b->len) return false;
    return a->len == 0 || memcmp(a->ptr, b->ptr, (size_t)a->len) == 0;
}

int32_t nr_str_length(const nr_String *s, const char *loc) {
    if (s->len > INT32_MAX) nr_panic("string too long for int length", loc);
    return (int32_t)s->len;
}

uint8_t nr_str_char_at(const nr_String *s, int64_t index, const char *loc) {
    if (index < 0 || index >= s->len) nr_panic("string index out of range", loc);
    return (uint8_t)s->ptr[index];
}

void nr_str_set_char_at(nr_String *s, int64_t index, uint8_t c, const char *loc) {
    if (index < 0 || index >= s->len) nr_panic("string index out of range", loc);
    str_reserve(s, s->len);
    s->ptr[index] = (char)c;
}

void nr_str_append(nr_String *s, const nr_String *text) {
    int64_t add = text->len;
    if (add == 0) return;
    str_reserve(s, s->len + add);
    /* text may be s itself; its pointer is read after the buffer moved */
    memmove(s->ptr + s->len, text->ptr, (size_t)add);
    s->len += add;
    s->ptr[s->len] = '\0';
}

nr_String nr_str_concat(const nr_String *a, const nr_String *b) {
    if (a->len > INT64_MAX - b->len - 1) nr_panic("out of memory", "?");
    nr_String s;
    s.cap = a->len + b->len + 1;
    s.ptr = checked_realloc(NULL, (size_t)s.cap);
    if (a->len) memcpy(s.ptr, a->ptr, (size_t)a->len);
    if (b->len) memcpy(s.ptr + a->len, b->ptr, (size_t)b->len);
    s.len = a->len + b->len;
    s.ptr[s.len] = '\0';
    return s;
}

void nr_str_append_char(nr_String *s, uint8_t c) {
    str_reserve(s, s->len + 1);
    s->ptr[s->len++] = (char)c;
    s->ptr[s->len] = '\0';
}

nr_String nr_str_substring(const nr_String *s, int64_t from, int64_t to, const char *loc) {
    if (from < 0 || from > to || to > s->len) nr_panic("string index out of range", loc);
    return str_from(s->ptr ? s->ptr + from : "", (size_t)(to - from));
}

int32_t nr_str_index_of(const nr_String *s, const nr_String *text) {
    if (text->len == 0) return 0;
    for (int64_t i = 0; i + text->len <= s->len && i <= INT32_MAX; i++) {
        if (memcmp(s->ptr + i, text->ptr, (size_t)text->len) == 0) return (int32_t)i;
    }
    return -1;
}

/* Copies s into a null-terminated buffer for parsing. */
static const char *parse_text(const nr_String *s, char *small, size_t small_size, char **heap) {
    *heap = NULL;
    if ((size_t)s->len < small_size) {
        if (s->len) memcpy(small, s->ptr, (size_t)s->len);
        small[s->len] = '\0';
        return small;
    }
    *heap = checked_realloc(NULL, (size_t)s->len + 1);
    memcpy(*heap, s->ptr, (size_t)s->len);
    (*heap)[s->len] = '\0';
    return *heap;
}

static bool valid_integer_text(const char *t) {
    if (*t == '-' || *t == '+') t++;
    if (!*t) return false;
    for (; *t; t++) {
        if (*t < '0' || *t > '9') return false;
    }
    return true;
}

static bool valid_float_text(const char *t) {
    if (*t == '-' || *t == '+') t++;
    if (strcmp(t, "Infinity") == 0 || strcmp(t, "NaN") == 0) return true;
    bool digits = false;
    while (*t >= '0' && *t <= '9') t++, digits = true;
    if (*t == '.') {
        t++;
        while (*t >= '0' && *t <= '9') t++, digits = true;
    }
    if (!digits) return false;
    if (*t == 'e' || *t == 'E') {
        t++;
        if (*t == '-' || *t == '+') t++;
        if (*t < '0' || *t > '9') return false;
        while (*t >= '0' && *t <= '9') t++;
    }
    return *t == '\0';
}

int64_t nr_str_to_i64(const nr_String *s, const char *loc) {
    char small[64];
    char *heap;
    const char *t = parse_text(s, small, sizeof small, &heap);
    bool ok = valid_integer_text(t);
    errno = 0;
    long long v = ok ? strtoll(t, NULL, 10) : 0;
    if (errno == ERANGE) ok = false;
    free(heap);
    if (!ok) nr_panic("invalid number", loc);
    return (int64_t)v;
}

int32_t nr_str_to_i32(const nr_String *s, const char *loc) {
    char small[64];
    char *heap;
    const char *t = parse_text(s, small, sizeof small, &heap);
    bool ok = valid_integer_text(t);
    errno = 0;
    long long v = ok ? strtoll(t, NULL, 10) : 0;
    if (errno == ERANGE || v < INT32_MIN || v > INT32_MAX) ok = false;
    free(heap);
    if (!ok) nr_panic("invalid number", loc);
    return (int32_t)v;
}

double nr_str_to_f64(const nr_String *s, const char *loc) {
    char small[64];
    char *heap;
    const char *t = parse_text(s, small, sizeof small, &heap);
    bool ok = valid_float_text(t);
    double v = 0;
    if (ok) {
        const char *body = (*t == '-' || *t == '+') ? t + 1 : t;
        if (strcmp(body, "Infinity") == 0) v = INFINITY;
        else if (strcmp(body, "NaN") == 0) v = NAN;
        else v = strtod(body, NULL);
        if (*t == '-') v = -v;
        if (isinf(v) && strcmp(body, "Infinity") != 0) ok = false;
    }
    free(heap);
    if (!ok) nr_panic("invalid number", loc);
    return v;
}

nr_String nr_str_of_i64(int64_t v) {
    char buf[32];
    int n = snprintf(buf, sizeof buf, "%" PRId64, v);
    return str_from(buf, (size_t)n);
}

nr_String nr_str_of_u64(uint64_t v) {
    char buf[32];
    int n = snprintf(buf, sizeof buf, "%" PRIu64, v);
    return str_from(buf, (size_t)n);
}

nr_String nr_str_of_f32(float v) {
    char buf[64];
    format_float(buf, (double)v, 9, true);
    return str_from(buf, strlen(buf));
}

nr_String nr_str_of_f64(double v) {
    char buf[64];
    format_float(buf, v, 17, false);
    return str_from(buf, strlen(buf));
}

nr_String nr_str_of_bool(bool v) {
    return v ? nr_str_lit("true", 4) : nr_str_lit("false", 5);
}

nr_String nr_str_of_char(uint8_t v) {
    char c = (char)v;
    return str_from(&c, 1);
}
