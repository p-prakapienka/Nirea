/* Nirea runtime support for generated C code.
 *
 * Generated programs include only this header, so that extern "C"
 * declarations in Nirea code never conflict with system headers. The
 * functions that need the C library live in nirea_rt.c. */
#ifndef NIREA_RT_H
#define NIREA_RT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* A String owns its bytes when cap > 0. With cap == 0 it points at static
 * literal bytes (or is NULL, the empty string) and is copied to the heap on
 * its first change. ptr, when not NULL, is always followed by a null byte. */
typedef struct nr_String {
    char *ptr;
    int64_t len;
    int64_t cap;
} nr_String;

_Noreturn void nr_panic(const char *what, const char *loc);
_Noreturn void nr_unreachable(void);

void nr_print_i64(int64_t v);
void nr_print_u64(uint64_t v);
void nr_print_f32(float v);
void nr_print_f64(double v);
void nr_print_bool(bool v);
void nr_print_char(uint8_t v);
void nr_print_str(const nr_String *s);
void nr_print_newline(void);

double nr_fmod(double a, double b);
float nr_fmodf(float a, float b);

/* Strings (spec 6.5). */
static inline nr_String nr_str_lit(const char *bytes, int64_t len) {
    nr_String s = {(char *)bytes, len, 0};
    return s;
}

static inline const char *nr_cstr(const nr_String *s) {
    return s->ptr ? s->ptr : "";
}

void nr_str_free(nr_String *s);
nr_String nr_str_copy(const nr_String *s);
bool nr_str_eq(const nr_String *a, const nr_String *b);
int32_t nr_str_length(const nr_String *s, const char *loc);
uint8_t nr_str_char_at(const nr_String *s, int64_t index, const char *loc);
void nr_str_set_char_at(nr_String *s, int64_t index, uint8_t c, const char *loc);
void nr_str_append(nr_String *s, const nr_String *text);
nr_String nr_str_concat(const nr_String *a, const nr_String *b);
void nr_str_append_char(nr_String *s, uint8_t c);
nr_String nr_str_substring(const nr_String *s, int64_t from, int64_t to, const char *loc);
int32_t nr_str_index_of(const nr_String *s, const nr_String *text);
int32_t nr_str_to_i32(const nr_String *s, const char *loc);
int64_t nr_str_to_i64(const nr_String *s, const char *loc);
double nr_str_to_f64(const nr_String *s, const char *loc);
nr_String nr_str_of_i64(int64_t v);
nr_String nr_str_of_u64(uint64_t v);
nr_String nr_str_of_f32(float v);
nr_String nr_str_of_f64(double v);
nr_String nr_str_of_bool(bool v);
nr_String nr_str_of_char(uint8_t v);

/* Array indexing (spec 6.6). */
static inline int64_t nr_index_i64(int64_t i, int64_t length, const char *loc) {
    if (i < 0 || i >= length) nr_panic("array index out of range", loc);
    return i;
}

static inline int64_t nr_index_u64(uint64_t i, int64_t length, const char *loc) {
    if (i >= (uint64_t)length) nr_panic("array index out of range", loc);
    return (int64_t)i;
}

static inline int64_t nr_shift_amount_u64(uint64_t n) {
    return n > 64 ? 64 : (int64_t)n;
}

/* Checked integer arithmetic (spec 6.4). */
#if defined(__GNUC__) || defined(__clang__)
#define NR_ADD_OVERFLOW(a, b, r) __builtin_add_overflow(a, b, r)
#define NR_SUB_OVERFLOW(a, b, r) __builtin_sub_overflow(a, b, r)
#define NR_MUL_OVERFLOW(a, b, r) __builtin_mul_overflow(a, b, r)
#else
#error "Nirea generated code needs GCC or Clang overflow builtins"
#endif

#define NR_DEFINE_COMMON_OPS(S, T)                                                           \
    static inline T nr_add_##S(T a, T b, const char *loc) {                                  \
        T r;                                                                                 \
        if (NR_ADD_OVERFLOW(a, b, &r)) nr_panic("integer overflow", loc);                    \
        return r;                                                                            \
    }                                                                                        \
    static inline T nr_sub_##S(T a, T b, const char *loc) {                                  \
        T r;                                                                                 \
        if (NR_SUB_OVERFLOW(a, b, &r)) nr_panic("integer overflow", loc);                    \
        return r;                                                                            \
    }                                                                                        \
    static inline T nr_mul_##S(T a, T b, const char *loc) {                                  \
        T r;                                                                                 \
        if (NR_MUL_OVERFLOW(a, b, &r)) nr_panic("integer overflow", loc);                    \
        return r;                                                                            \
    }

#define NR_DEFINE_SIGNED_OPS(S, T, UT, BITS, MINV)                                           \
    NR_DEFINE_COMMON_OPS(S, T)                                                               \
    static inline T nr_div_##S(T a, T b, const char *loc) {                                  \
        if (b == 0) nr_panic("division by zero", loc);                                       \
        if (a == (MINV) && b == -1) nr_panic("integer overflow", loc);                       \
        return (T)(a / b);                                                                   \
    }                                                                                        \
    static inline T nr_rem_##S(T a, T b, const char *loc) {                                  \
        if (b == 0) nr_panic("division by zero", loc);                                       \
        if (b == -1) return 0;                                                               \
        return (T)(a % b);                                                                   \
    }                                                                                        \
    static inline T nr_neg_##S(T a, const char *loc) {                                       \
        if (a == (MINV)) nr_panic("integer overflow", loc);                                  \
        return (T)-a;                                                                        \
    }                                                                                        \
    static inline T nr_shl_##S(T a, int64_t n, const char *loc) {                            \
        if (n < 0 || n >= BITS) nr_panic("invalid shift amount", loc);                       \
        return (T)(UT)((UT)a << n);                                                          \
    }                                                                                        \
    static inline T nr_shr_##S(T a, int64_t n, const char *loc) {                            \
        if (n < 0 || n >= BITS) nr_panic("invalid shift amount", loc);                       \
        if (a < 0) return (T)~(T)((T)~a >> n);                                               \
        return (T)(a >> n);                                                                  \
    }

#define NR_DEFINE_UNSIGNED_OPS(S, T, BITS)                                                   \
    NR_DEFINE_COMMON_OPS(S, T)                                                               \
    static inline T nr_div_##S(T a, T b, const char *loc) {                                  \
        if (b == 0) nr_panic("division by zero", loc);                                       \
        return (T)(a / b);                                                                   \
    }                                                                                        \
    static inline T nr_rem_##S(T a, T b, const char *loc) {                                  \
        if (b == 0) nr_panic("division by zero", loc);                                       \
        return (T)(a % b);                                                                   \
    }                                                                                        \
    static inline T nr_shl_##S(T a, int64_t n, const char *loc) {                            \
        if (n < 0 || n >= BITS) nr_panic("invalid shift amount", loc);                       \
        return (T)(a << n);                                                                  \
    }                                                                                        \
    static inline T nr_shr_##S(T a, int64_t n, const char *loc) {                            \
        if (n < 0 || n >= BITS) nr_panic("invalid shift amount", loc);                       \
        return (T)(a >> n);                                                                  \
    }

NR_DEFINE_SIGNED_OPS(i8, int8_t, uint8_t, 8, INT8_MIN)
NR_DEFINE_SIGNED_OPS(i16, int16_t, uint16_t, 16, INT16_MIN)
NR_DEFINE_SIGNED_OPS(i32, int32_t, uint32_t, 32, INT32_MIN)
NR_DEFINE_SIGNED_OPS(i64, int64_t, uint64_t, 64, INT64_MIN)
NR_DEFINE_UNSIGNED_OPS(u8, uint8_t, 8)
NR_DEFINE_UNSIGNED_OPS(u16, uint16_t, 16)
NR_DEFINE_UNSIGNED_OPS(u32, uint32_t, 32)
NR_DEFINE_UNSIGNED_OPS(u64, uint64_t, 64)

/* Float-to-integer casts (spec 6.2): truncate toward zero; NaN, infinities
 * and out-of-range values are runtime errors. Bounds are exclusive. */
#define NR_DEFINE_F2I(S, T, LOW, HIGH)                                                       \
    static inline T nr_f2##S(double x, const char *loc) {                                    \
        if (!(x > (LOW) && x < (HIGH))) nr_panic("invalid cast of a floating-point value", loc); \
        return (T)x;                                                                         \
    }

NR_DEFINE_F2I(i8, int8_t, -129.0, 128.0)
NR_DEFINE_F2I(i16, int16_t, -32769.0, 32768.0)
NR_DEFINE_F2I(i32, int32_t, -2147483649.0, 2147483648.0)
NR_DEFINE_F2I(i64, int64_t, -9223372036854777856.0, 9223372036854775808.0)
NR_DEFINE_F2I(u8, uint8_t, -1.0, 256.0)
NR_DEFINE_F2I(u16, uint16_t, -1.0, 65536.0)
NR_DEFINE_F2I(u32, uint32_t, -1.0, 4294967296.0)
NR_DEFINE_F2I(u64, uint64_t, -1.0, 18446744073709551616.0)

#endif
