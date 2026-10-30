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

typedef struct nr_String {
    const char *ptr;
    int64_t len;
} nr_String;

_Noreturn void nr_panic(const char *what, const char *loc);
_Noreturn void nr_unreachable(void);

void nr_print_i64(int64_t v);
void nr_print_u64(uint64_t v);
void nr_print_f32(float v);
void nr_print_f64(double v);
void nr_print_bool(bool v);
void nr_print_char(uint8_t v);
void nr_print_str(nr_String s);
void nr_print_newline(void);

bool nr_str_eq(nr_String a, nr_String b);
double nr_fmod(double a, double b);
float nr_fmodf(float a, float b);

static inline const char *nr_cstr(nr_String s) {
    return s.ptr ? s.ptr : "";
}

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
