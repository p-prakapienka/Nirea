/* Loads source files, resolves imports (spec 12), runs the compiler passes
 * and invokes the C compiler (spec 13). */
#include "driver.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ast.h"
#include "codegen.h"
#include "lexer.h"
#include "parser.h"
#include "sema.h"

extern const unsigned char nirea_rt_h_data[];
extern const size_t nirea_rt_h_size;
extern const unsigned char nirea_rt_c_data[];
extern const size_t nirea_rt_c_size;

typedef struct Loader {
    Program *prog;
    const char *root;         /* source root as given */
    const char *root_real;    /* canonical source root */
    Map by_real_path;         /* interned real path -> SourceFile */
} Loader;

static bool ends_with(const char *s, const char *suffix) {
    size_t ls = strlen(s), lx = strlen(suffix);
    return ls >= lx && strcmp(s + ls - lx, suffix) == 0;
}

/* Removes "./" segments and collapses "dir/../" for nicer display paths. */
static char *clean_path(const char *path) {
    StrVec parts = {0};
    bool absolute = path[0] == '/';
    char *copy = xstrdup(path);
    for (char *tok = strtok(copy, "/"); tok; tok = strtok(NULL, "/")) {
        if (strcmp(tok, ".") == 0) continue;
        if (strcmp(tok, "..") == 0 && parts.len > 0 && strcmp(parts.data[parts.len - 1], "..") != 0) {
            parts.len--;
            continue;
        }
        vec_push(parts, tok);
    }
    StrBuf sb = {0};
    if (absolute) sb_putc(&sb, '/');
    for (int i = 0; i < parts.len; i++) {
        if (i) sb_putc(&sb, '/');
        sb_append(&sb, parts.data[i]);
    }
    if (sb.len == 0) sb_append(&sb, ".");
    return (char *)sb_cstr(&sb);
}

static SourceFile *load_file(Loader *ld, const char *display_path) {
    char *real = path_realpath(display_path);
    if (!real) return NULL;
    const char *key = intern_cstr(real);
    SourceFile *existing = map_get(&ld->by_real_path, key);
    if (existing) return existing;

    SourceFile *f = NEW(SourceFile);
    f->path = clean_path(display_path);
    f->real_path = key;
    f->dir = path_dirname(display_path);
    f->text = read_file(display_path, &f->len);
    if (!f->text) return NULL;
    map_put(&ld->by_real_path, key, f);
    vec_push(ld->prog->files, f);

    TokenVec tokens = lex_file(f);
    parse_file(f, tokens);
    return f;
}

static void collect_sources(Loader *ld, const char *dir, StrVec *out) {
    DIR *d = opendir(dir);
    if (!d) return;
    StrVec names = {0};
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        vec_push(names, xstrdup(ent->d_name));
    }
    closedir(d);
    /* sort for deterministic output */
    for (int i = 1; i < names.len; i++) {
        for (int j = i; j > 0 && strcmp(names.data[j - 1], names.data[j]) > 0; j--) {
            const char *tmp = names.data[j];
            names.data[j] = names.data[j - 1];
            names.data[j - 1] = tmp;
        }
    }
    for (int i = 0; i < names.len; i++) {
        char *path = path_join(dir, names.data[i]);
        if (dir_exists(path)) {
            collect_sources(ld, path, out);
        } else if (ends_with(names.data[i], ".ni") && file_exists(path)) {
            vec_push(*out, path);
        }
    }
}

static void resolve_imports(Loader *ld, SourceFile *f) {
    f->imported = xcalloc((size_t)(f->import_count ? f->import_count : 1), sizeof(SourceFile *));
    for (int i = 0; i < f->import_count; i++) {
        ImportDecl *imp = &f->imports[i];
        const char *p = imp->path;
        if (strchr(p, '\\')) {
            error_at(imp->loc, "import paths use forward slashes: '%s'", p);
            continue;
        }
        if (p[0] == '/') {
            error_at(imp->loc, "import paths must be relative to the source root or the importing file: '%s'", p);
            continue;
        }
        if (!ends_with(p, ".ni")) {
            error_at(imp->loc, "import paths must name a .ni file, including the extension: '%s'", p);
            continue;
        }
        char *local = path_join(f->dir, p);
        bool relative_only = strncmp(p, "./", 2) == 0 || strncmp(p, "../", 3) == 0;
        char *from_root = relative_only ? NULL : path_join(ld->root, p);
        bool local_ok = file_exists(local);
        bool root_ok = from_root && file_exists(from_root);
        const char *chosen = NULL;
        if (local_ok && root_ok) {
            char *a = path_realpath(local), *b = path_realpath(from_root);
            if (a && b && strcmp(a, b) != 0) {
                error_at(imp->loc,
                         "import '%s' is ambiguous: it matches both %s (next to this file) and %s (from the source root)",
                         p, clean_path(local), clean_path(from_root));
                continue;
            }
            chosen = local;
        } else if (local_ok) {
            chosen = local;
        } else if (root_ok) {
            chosen = from_root;
        } else {
            error_at(imp->loc, "can't find imported file '%s'", p);
            continue;
        }
        SourceFile *target = load_file(ld, chosen);
        if (!target) {
            error_at(imp->loc, "can't read imported file '%s'", p);
            continue;
        }
        f->imported[i] = target;
    }
}

static bool write_runtime(const char *dir) {
    return write_file(path_join(dir, "nirea_rt.h"), (const char *)nirea_rt_h_data, nirea_rt_h_size) &&
           write_file(path_join(dir, "nirea_rt.c"), (const char *)nirea_rt_c_data, nirea_rt_c_size);
}

int run_process(const char *const *argv) {
    fflush(stdout);
    fflush(stderr);
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "nirea: error: can't start %s: %s\n", argv[0], strerror(errno));
        return 127;
    }
    if (pid == 0) {
        execvp(argv[0], (char *const *)argv);
        fprintf(stderr, "nirea: error: can't run %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }
    int status;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) return 127;
    }
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return 1;
}

void remove_dir_tree(const char *dir) {
    DIR *d = opendir(dir);
    if (d) {
        struct dirent *ent;
        while ((ent = readdir(d)) != NULL) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
            char *path = path_join(dir, ent->d_name);
            if (dir_exists(path)) remove_dir_tree(path);
            else remove(path);
        }
        closedir(d);
    }
    rmdir(dir);
}

static char *make_temp_dir(void) {
    const char *base = getenv("TMPDIR");
    if (!base || !*base) base = "/tmp";
    char *templ = path_join(base, "nirea-XXXXXX");
    if (!mkdtemp(templ)) fatal("can't create a temporary directory in %s: %s", base, strerror(errno));
    return templ;
}

static char *file_stem(const char *path) {
    const char *slash = strrchr(path, '/');
    const char *name = slash ? slash + 1 : path;
    size_t len = strlen(name);
    if (ends_with(name, ".ni")) len -= 3;
    return xstrndup(name, len);
}

int nirea_build(BuildOptions *opts, char **exe_out, char **temp_dir_out) {
    *exe_out = NULL;
    *temp_dir_out = NULL;
    Program *prog = NEW(Program);
    Loader ld = {0};
    ld.prog = prog;

    const char *input = opts->input ? opts->input : ".";
    bool single_file = ends_with(input, ".ni");
    const char *project_dir = NULL;

    if (single_file) {
        if (!file_exists(input)) {
            fprintf(stderr, "nirea: error: can't find source file '%s'\n", input);
            return 1;
        }
        ld.root = path_dirname(input);
        if (!load_file(&ld, input)) {
            fprintf(stderr, "nirea: error: can't read '%s'\n", input);
            return 1;
        }
    } else {
        project_dir = input;
        char *root = path_join(input, "src/main/nirea");
        if (!dir_exists(root)) {
            fprintf(stderr, "nirea: error: '%s' is not a Nirea project: missing %s\n", input, root);
            return 1;
        }
        ld.root = root;
        StrVec files = {0};
        collect_sources(&ld, root, &files);
        if (files.len == 0) {
            fprintf(stderr, "nirea: error: no .ni files found in %s\n", root);
            return 1;
        }
        for (int i = 0; i < files.len; i++) load_file(&ld, files.data[i]);
    }
    ld.root_real = path_realpath(ld.root);

    /* Resolve imports; this may load more files, which are appended. */
    for (int i = 0; i < prog->files.len; i++) resolve_imports(&ld, prog->files.data[i]);

    if (error_count == 0) sema_check_program(prog);
    if (error_count == 0) ownership_check_program(prog);
    if (error_count > 0) {
        fprintf(stderr, "nirea: %d error%s\n", error_count, error_count == 1 ? "" : "s");
        return 1;
    }
    if (opts->check_only) return 0;

    char *c_source = codegen_program(prog);

    /* Choose output locations. */
    char *c_dir;
    char *exe;
    if (project_dir) {
        char *target = path_join(project_dir, "target");
        c_dir = opts->emit_c_dir ? xstrdup(opts->emit_c_dir) : path_join(target, "generated-c");
        exe = opts->output ? xstrdup(opts->output) : path_join(target, "nirea-app");
    } else {
        /* Single file: generated C goes to a temporary directory unless
         * --emit-c is given; the executable goes to -o, else the current
         * directory for build and the temporary directory for run. */
        bool exe_in_temp = !opts->output && opts->for_run;
        char *tmp = NULL;
        if (!opts->emit_c_dir || exe_in_temp) {
            tmp = make_temp_dir();
            *temp_dir_out = tmp;
        }
        c_dir = opts->emit_c_dir ? xstrdup(opts->emit_c_dir) : path_join(tmp, "c");
        if (opts->output) exe = xstrdup(opts->output);
        else if (exe_in_temp) exe = path_join(tmp, file_stem(input));
        else exe = file_stem(input);
    }
    if (!make_dirs(c_dir)) {
        fprintf(stderr, "nirea: error: can't create directory %s\n", c_dir);
        return 1;
    }
    char *exe_dir = path_dirname(exe);
    if (!make_dirs(exe_dir)) {
        fprintf(stderr, "nirea: error: can't create directory %s\n", exe_dir);
        return 1;
    }
    char *program_c = path_join(c_dir, "program.c");
    if (!write_file(program_c, c_source, strlen(c_source)) || !write_runtime(c_dir)) {
        fprintf(stderr, "nirea: error: can't write generated C to %s\n", c_dir);
        return 1;
    }

    const char *cc = opts->cc;
    if (!cc) cc = getenv("CC");
    if (!cc || !*cc) cc = "cc";
    StrVec argv = {0};
    vec_push(argv, cc);
    vec_push(argv, "-std=c11");
    vec_push(argv, "-O2");
    vec_push(argv, "-o");
    vec_push(argv, exe);
    vec_push(argv, program_c);
    vec_push(argv, path_join(c_dir, "nirea_rt.c"));
    for (int i = 0; i < opts->cc_args.len; i++) vec_push(argv, opts->cc_args.data[i]);
    const char *extra = getenv("NIREA_CFLAGS");
    if (extra) {
        for (char *tok = strtok(xstrdup(extra), " \t"); tok; tok = strtok(NULL, " \t")) vec_push(argv, tok);
    }
    vec_push(argv, "-lm");
    vec_push(argv, NULL);
    int status = run_process(argv.data);
    if (status != 0) {
        fprintf(stderr, "nirea: error: the C compiler (%s) failed with status %d\n", cc, status);
        return 1;
    }
    *exe_out = exe;
    return 0;
}
