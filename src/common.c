#include "common.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *xmalloc(size_t size) {
    void *p = malloc(size ? size : 1);
    if (!p) fatal("out of memory");
    return p;
}

void *xcalloc(size_t count, size_t size) {
    void *p = calloc(count ? count : 1, size ? size : 1);
    if (!p) fatal("out of memory");
    return p;
}

void *xrealloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size ? size : 1);
    if (!p) fatal("out of memory");
    return p;
}

char *xstrdup(const char *s) {
    return xstrndup(s, strlen(s));
}

char *xstrndup(const char *s, size_t len) {
    char *p = xmalloc(len + 1);
    memcpy(p, s, len);
    p[len] = '\0';
    return p;
}

/* String builder */

static void sb_reserve(StrBuf *sb, size_t extra) {
    if (sb->len + extra + 1 <= sb->cap) return;
    size_t cap = sb->cap ? sb->cap : 64;
    while (cap < sb->len + extra + 1) cap *= 2;
    sb->data = xrealloc(sb->data, cap);
    sb->cap = cap;
}

void sb_append_n(StrBuf *sb, const char *s, size_t n) {
    sb_reserve(sb, n);
    memcpy(sb->data + sb->len, s, n);
    sb->len += n;
    sb->data[sb->len] = '\0';
}

void sb_append(StrBuf *sb, const char *s) {
    sb_append_n(sb, s, strlen(s));
}

void sb_putc(StrBuf *sb, char c) {
    sb_append_n(sb, &c, 1);
}

void sb_vprintf(StrBuf *sb, const char *fmt, va_list ap) {
    va_list copy;
    va_copy(copy, ap);
    int n = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);
    if (n < 0) fatal("formatting failed");
    sb_reserve(sb, (size_t)n);
    vsnprintf(sb->data + sb->len, (size_t)n + 1, fmt, ap);
    sb->len += (size_t)n;
}

void sb_printf(StrBuf *sb, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    sb_vprintf(sb, fmt, ap);
    va_end(ap);
}

const char *sb_cstr(StrBuf *sb) {
    if (!sb->data) sb_reserve(sb, 0), sb->data[0] = '\0';
    return sb->data;
}

char *fmt_str(const char *fmt, ...) {
    StrBuf sb = {0};
    va_list ap;
    va_start(ap, fmt);
    sb_vprintf(&sb, fmt, ap);
    va_end(ap);
    sb_cstr(&sb);
    return sb.data;
}

/* Interning */

typedef struct InternEntry {
    const char *str;
    size_t len;
    uint64_t hash;
} InternEntry;

static InternEntry *intern_table;
static size_t intern_cap;
static size_t intern_count;

static uint64_t hash_bytes(const char *s, size_t len) {
    uint64_t h = 1469598103934665603ULL;
    for (size_t i = 0; i < len; i++) {
        h ^= (unsigned char)s[i];
        h *= 1099511628211ULL;
    }
    return h;
}

static void intern_grow(void) {
    size_t cap = intern_cap ? intern_cap * 2 : 1024;
    InternEntry *table = xcalloc(cap, sizeof(InternEntry));
    for (size_t i = 0; i < intern_cap; i++) {
        if (!intern_table[i].str) continue;
        size_t j = intern_table[i].hash & (cap - 1);
        while (table[j].str) j = (j + 1) & (cap - 1);
        table[j] = intern_table[i];
    }
    free(intern_table);
    intern_table = table;
    intern_cap = cap;
}

const char *intern(const char *s, size_t len) {
    if ((intern_count + 1) * 2 > intern_cap) intern_grow();
    uint64_t h = hash_bytes(s, len);
    size_t i = h & (intern_cap - 1);
    while (intern_table[i].str) {
        InternEntry *e = &intern_table[i];
        if (e->hash == h && e->len == len && memcmp(e->str, s, len) == 0) return e->str;
        i = (i + 1) & (intern_cap - 1);
    }
    intern_table[i].str = xstrndup(s, len);
    intern_table[i].len = len;
    intern_table[i].hash = h;
    intern_count++;
    return intern_table[i].str;
}

const char *intern_cstr(const char *s) {
    return intern(s, strlen(s));
}

/* Pointer-keyed map */

static size_t ptr_hash(const void *p) {
    uintptr_t x = (uintptr_t)p;
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    return (size_t)x;
}

static void map_grow(Map *m) {
    int cap = m->cap ? m->cap * 2 : 16;
    const void **keys = xcalloc((size_t)cap, sizeof(void *));
    void **values = xcalloc((size_t)cap, sizeof(void *));
    for (int i = 0; i < m->cap; i++) {
        if (!m->keys[i]) continue;
        size_t j = ptr_hash(m->keys[i]) & (size_t)(cap - 1);
        while (keys[j]) j = (j + 1) & (size_t)(cap - 1);
        keys[j] = m->keys[i];
        values[j] = m->values[i];
    }
    free(m->keys);
    free(m->values);
    m->keys = keys;
    m->values = values;
    m->cap = cap;
}

void *map_get(Map *m, const void *key) {
    if (!m->cap) return NULL;
    size_t i = ptr_hash(key) & (size_t)(m->cap - 1);
    while (m->keys[i]) {
        if (m->keys[i] == key) return m->values[i];
        i = (i + 1) & (size_t)(m->cap - 1);
    }
    return NULL;
}

void map_put(Map *m, const void *key, void *value) {
    if ((m->count + 1) * 2 > m->cap) map_grow(m);
    size_t i = ptr_hash(key) & (size_t)(m->cap - 1);
    while (m->keys[i]) {
        if (m->keys[i] == key) {
            m->values[i] = value;
            return;
        }
        i = (i + 1) & (size_t)(m->cap - 1);
    }
    m->keys[i] = key;
    m->values[i] = value;
    m->count++;
}

/* Diagnostics */

int error_count;
int error_suppress;

static void report(const char *kind, Loc loc, const char *fmt, va_list ap) {
    if (loc.file) {
        fprintf(stderr, "%s:%d:%d: %s: ", loc.file->path, loc.line, loc.col, kind);
    } else {
        fprintf(stderr, "nirea: %s: ", kind);
    }
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
}

void error_at(Loc loc, const char *fmt, ...) {
    if (error_suppress > 0) return;
    va_list ap;
    va_start(ap, fmt);
    report("error", loc, fmt, ap);
    va_end(ap);
    error_count++;
}

void note_at(Loc loc, const char *fmt, ...) {
    if (error_suppress > 0) return;
    va_list ap;
    va_start(ap, fmt);
    report("note", loc, fmt, ap);
    va_end(ap);
}

void fatal(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "nirea: error: ");
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(1);
}

/* Files and paths */

char *read_file(const char *path, size_t *len_out) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    StrBuf sb = {0};
    char buf[65536];
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, f)) > 0) sb_append_n(&sb, buf, n);
    fclose(f);
    sb_cstr(&sb);
    if (len_out) *len_out = sb.len;
    return sb.data;
}

bool write_file(const char *path, const char *data, size_t len) {
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    bool ok = fwrite(data, 1, len, f) == len;
    if (fclose(f) != 0) ok = false;
    return ok;
}
