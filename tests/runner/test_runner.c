/* Test runner for the Nirea compiler, driven by CTest.
 *
 *   nirea-test run     NIREA FILE.ni  WORKDIR   compile and run, compare output
 *   nirea-test error   NIREA FILE.ni  WORKDIR   compilation must fail as marked
 *   nirea-test project NIREA DIR      WORKDIR   build a project, compare output
 *
 * Expectations live in comments in the test source:
 *   // expect: TEXT           one expected stdout line, in order
 *   // exit: N                expected exit status (default 0)
 *   // runtime-error: TEXT    program must fail with TEXT in stderr
 *   // error[: TEXT]          (error tests) a diagnostic is expected on this line
 * Project tests compare stdout with expected.txt, or must fail to compile if
 * the project contains a file named expect-error. */
#define _XOPEN_SOURCE 700
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct Lines {
    char **items;
    int count;
    int cap;
} Lines;

static void lines_push(Lines *l, char *s) {
    if (l->count == l->cap) {
        l->cap = l->cap ? l->cap * 2 : 16;
        l->items = realloc(l->items, sizeof(char *) * (size_t)l->cap);
    }
    l->items[l->count++] = s;
}

static char *read_all(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    size_t cap = 4096, len = 0;
    char *buf = malloc(cap);
    size_t n;
    while ((n = fread(buf + len, 1, cap - len - 1, f)) > 0) {
        len += n;
        if (cap - len < 2) buf = realloc(buf, cap *= 2);
    }
    fclose(f);
    buf[len] = '\0';
    return buf;
}

static char *trim_right(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\r')) s[--n] = '\0';
    return s;
}

/* Splits text into lines; a final newline doesn't start an extra line. */
static Lines split_lines(const char *text) {
    Lines l = {0};
    const char *p = text;
    while (*p) {
        const char *nl = strchr(p, '\n');
        size_t n = nl ? (size_t)(nl - p) : strlen(p);
        char *line = strndup(p, n);
        lines_push(&l, trim_right(line));
        if (!nl) break;
        p = nl + 1;
    }
    return l;
}

/* Runs argv with stdout and stderr redirected to files. Returns exit status. */
static int run(char **argv, const char *out_path, const char *err_path) {
    fflush(stdout);
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0) {
        int out = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        int err = open(err_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (out < 0 || err < 0) _exit(126);
        dup2(out, 1);
        dup2(err, 2);
        execv(argv[0], argv);
        fprintf(stderr, "can't run %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }
    int status;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) return -1;
    }
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return -1;
}

static char *join(const char *a, const char *b) {
    char *s = malloc(strlen(a) + strlen(b) + 2);
    sprintf(s, "%s/%s", a, b);
    return s;
}

static void print_file(const char *title, const char *path) {
    char *text = read_all(path);
    printf("---- %s ----\n%s", title, text ? text : "(missing)\n");
    if (text && *text && text[strlen(text) - 1] != '\n') printf("\n");
}

static void mkdir_p(const char *path) {
    char *p = strdup(path);
    for (char *s = p + 1; *s; s++) {
        if (*s == '/') {
            *s = '\0';
            mkdir(p, 0777);
            *s = '/';
        }
    }
    mkdir(p, 0777);
    free(p);
}

typedef struct Expectations {
    Lines stdout_lines;
    int exit_status;
    char *runtime_error;
} Expectations;

static Expectations parse_expectations(const char *source) {
    Expectations ex = {0};
    Lines lines = split_lines(source);
    for (int i = 0; i < lines.count; i++) {
        char *line = lines.items[i];
        char *m;
        if ((m = strstr(line, "// expect:")) != NULL) {
            m += strlen("// expect:");
            if (*m == ' ') m++;
            lines_push(&ex.stdout_lines, m);
        } else if ((m = strstr(line, "// exit:")) != NULL) {
            ex.exit_status = atoi(m + strlen("// exit:"));
        } else if ((m = strstr(line, "// runtime-error:")) != NULL) {
            m += strlen("// runtime-error:");
            while (*m == ' ') m++;
            ex.runtime_error = m;
        }
    }
    return ex;
}

static bool compare_stdout(Lines *expected, const char *out_path) {
    char *text = read_all(out_path);
    Lines actual = split_lines(text ? text : "");
    bool ok = actual.count == expected->count;
    for (int i = 0; ok && i < actual.count; i++) ok = strcmp(actual.items[i], expected->items[i]) == 0;
    if (!ok) {
        printf("stdout mismatch\n---- expected ----\n");
        for (int i = 0; i < expected->count; i++) printf("%s\n", expected->items[i]);
        printf("---- actual ----\n");
        for (int i = 0; i < actual.count; i++) printf("%s\n", actual.items[i]);
    }
    return ok;
}

static int check_run_result(Expectations *ex, int status, const char *out, const char *err) {
    bool ok = compare_stdout(&ex->stdout_lines, out);
    if (ex->runtime_error) {
        char *errtext = read_all(err);
        if (status == 0) {
            printf("expected a runtime error, but the program exited with status 0\n");
            ok = false;
        } else if (!errtext || !strstr(errtext, ex->runtime_error)) {
            printf("expected runtime error containing \"%s\"\n", ex->runtime_error);
            print_file("stderr", err);
            ok = false;
        }
    } else if (status != ex->exit_status) {
        printf("exit status %d, expected %d\n", status, ex->exit_status);
        print_file("stderr", err);
        ok = false;
    }
    return ok ? 0 : 1;
}

static int test_run(char *nirea, char *file, char *work) {
    mkdir_p(work);
    char *exe = join(work, "program");
    char *cdir = join(work, "c");
    char *out = join(work, "stdout.txt");
    char *err = join(work, "stderr.txt");
    char *build_argv[] = {nirea, "build", file, "-o", exe, "--emit-c", cdir, NULL};
    int status = run(build_argv, out, err);
    if (status != 0) {
        printf("compilation failed (status %d)\n", status);
        print_file("compiler output", err);
        return 1;
    }
    char *source = read_all(file);
    Expectations ex = parse_expectations(source);
    char *prog_argv[] = {exe, NULL};
    status = run(prog_argv, out, err);
    return check_run_result(&ex, status, out, err);
}

typedef struct Marker {
    int line;
    char *text;
    bool seen;
} Marker;

static int test_error(char *nirea, char *file, char *work) {
    mkdir_p(work);
    char *out = join(work, "stdout.txt");
    char *err = join(work, "stderr.txt");
    char *argv[] = {nirea, "check", file, NULL};
    int status = run(argv, out, err);
    char *source = read_all(file);
    Lines lines = split_lines(source);
    Marker *markers = calloc((size_t)lines.count + 1, sizeof(Marker));
    int nmarkers = 0;
    for (int i = 0; i < lines.count; i++) {
        char *m = strstr(lines.items[i], "// error");
        if (!m) continue;
        char *rest = m + strlen("// error");
        if (*rest != '\0' && *rest != ':' && *rest != ' ') continue;
        while (*rest == ' ') rest++;
        char *text = NULL;
        if (*rest == ':') {
            text = rest + 1;
            while (*text == ' ') text++;
            if (!*text) text = NULL;
        }
        markers[nmarkers++] = (Marker){i + 1, text, false};
    }
    bool ok = true;
    if (status == 0) {
        printf("expected compilation to fail, but it succeeded\n");
        ok = false;
    } else if (status != 1) {
        printf("compiler exited with status %d (crash?)\n", status);
        ok = false;
    }
    char *errtext = read_all(err);
    Lines diags = split_lines(errtext ? errtext : "");
    for (int i = 0; i < diags.count; i++) {
        char *d = diags.items[i];
        char *tag = strstr(d, ": error: ");
        if (!tag) continue;
        /* path:line:col: error: message */
        char *colon2 = tag;
        while (colon2 > d && *(colon2 - 1) != ':') colon2--; /* start of col */
        char *colon1 = colon2 - 1;
        while (colon1 > d && *(colon1 - 1) != ':') colon1--; /* start of line */
        int line = atoi(colon1);
        if (line <= 0 || colon1 == d) {
            if (nmarkers > 0) {
                printf("unexpected error without a location: %s\n", d);
                ok = false;
            }
            continue;
        }
        bool matched = false;
        for (int k = 0; k < nmarkers; k++) {
            if (markers[k].line != line) continue;
            matched = true;
            if (!markers[k].text || strstr(tag, markers[k].text)) markers[k].seen = true;
        }
        if (!matched && nmarkers > 0) {
            printf("unexpected error on line %d: %s\n", line, d);
            ok = false;
        }
    }
    for (int k = 0; k < nmarkers; k++) {
        if (!markers[k].seen) {
            printf("missing expected error on line %d%s%s\n", markers[k].line, markers[k].text ? " containing: " : "",
                   markers[k].text ? markers[k].text : "");
            ok = false;
        }
    }
    if (!ok) print_file("compiler output", err);
    return ok ? 0 : 1;
}

static int test_project(char *nirea, char *dir, char *work) {
    mkdir_p(work);
    char *exe = join(work, "program");
    char *cdir = join(work, "c");
    char *out = join(work, "stdout.txt");
    char *err = join(work, "stderr.txt");
    struct stat st;
    if (stat(join(dir, "expect-error"), &st) == 0) {
        char *argv[] = {nirea, "check", dir, NULL};
        int status = run(argv, out, err);
        if (status != 1) {
            printf("expected compilation to fail with status 1, got %d\n", status);
            print_file("compiler output", err);
            return 1;
        }
        return 0;
    }
    char *build_argv[] = {nirea, "build", dir, "-o", exe, "--emit-c", cdir, NULL};
    int status = run(build_argv, out, err);
    if (status != 0) {
        printf("compilation failed (status %d)\n", status);
        print_file("compiler output", err);
        return 1;
    }
    char *expected_text = read_all(join(dir, "expected.txt"));
    Expectations ex = {0};
    ex.stdout_lines = split_lines(expected_text ? expected_text : "");
    char *prog_argv[] = {exe, NULL};
    status = run(prog_argv, out, err);
    return check_run_result(&ex, status, out, err);
}

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "usage: nirea-test run|error|project NIREA PATH WORKDIR\n");
        return 2;
    }
    char *nirea = realpath(argv[2], NULL);
    if (!nirea) {
        fprintf(stderr, "can't find compiler %s\n", argv[2]);
        return 2;
    }
    if (strcmp(argv[1], "run") == 0) return test_run(nirea, argv[3], argv[4]);
    if (strcmp(argv[1], "error") == 0) return test_error(nirea, argv[3], argv[4]);
    if (strcmp(argv[1], "project") == 0) return test_project(nirea, argv[3], argv[4]);
    fprintf(stderr, "unknown mode %s\n", argv[1]);
    return 2;
}
