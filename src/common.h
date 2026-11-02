/* Shared utilities: memory, growable arrays, string building, interning,
 * hash maps and diagnostics. */
#ifndef NIREA_COMMON_H
#define NIREA_COMMON_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Memory. The compiler is short-lived, so allocations are never freed. */
void *xmalloc(size_t size);
void *xcalloc(size_t count, size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *s);
char *xstrndup(const char *s, size_t len);

#define NEW(T) ((T *)xcalloc(1, sizeof(T)))

/* Growable arrays: declare with `typedef VEC(T) Name;`. */
#define VEC(T)     \
    struct {       \
        T *data;   \
        int len;   \
        int cap;   \
    }

#define vec_push(v, x)                                                         \
    do {                                                                       \
        if ((v).len == (v).cap) {                                              \
            (v).cap = (v).cap ? (v).cap * 2 : 8;                               \
            (v).data = xrealloc((v).data, sizeof(*(v).data) * (size_t)(v).cap); \
        }                                                                      \
        (v).data[(v).len++] = (x);                                             \
    } while (0)

typedef VEC(const char *) StrVec;

/* String builder. */
typedef struct StrBuf {
    char *data;
    size_t len;
    size_t cap;
} StrBuf;

void sb_append(StrBuf *sb, const char *s);
void sb_append_n(StrBuf *sb, const char *s, size_t n);
void sb_putc(StrBuf *sb, char c);
void sb_printf(StrBuf *sb, const char *fmt, ...);
void sb_vprintf(StrBuf *sb, const char *fmt, va_list ap);
const char *sb_cstr(StrBuf *sb); /* never NULL */
char *fmt_str(const char *fmt, ...);

/* Interned strings compare equal by pointer. */
const char *intern(const char *s, size_t len);
const char *intern_cstr(const char *s);

/* Hash map from interned string (or any pointer) to pointer. */
typedef struct Map {
    const void **keys;
    void **values;
    int cap;
    int count;
} Map;

void *map_get(Map *m, const void *key);
void map_put(Map *m, const void *key, void *value);

/* Source files and locations. */
typedef struct SourceFile {
    const char *path;      /* path as shown in diagnostics */
    const char *real_path; /* canonical path, used for identity */
    const char *dir;       /* directory of real_path */
    char *text;
    size_t len;
    struct Decl **decls;
    int decl_count;
    struct ImportDecl *imports;
    int import_count;
    struct SourceFile **imported; /* resolved imports, import_count entries */
} SourceFile;

typedef struct Loc {
    SourceFile *file;
    int line;
    int col;
} Loc;

extern int error_count;
extern int error_suppress; /* > 0 while errors must not be reported */

void error_at(Loc loc, const char *fmt, ...);
void note_at(Loc loc, const char *fmt, ...);
void fatal(const char *fmt, ...);

/* Files. Paths and directories are in platform.h. */
char *read_file(const char *path, size_t *len_out);
bool write_file(const char *path, const char *data, size_t len);

#endif
