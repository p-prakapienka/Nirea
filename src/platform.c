/* Operating-system services for POSIX systems and Windows. Everything that
 * differs between them lives in this file. */
#include "platform.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <dirent.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
extern char **environ;
#endif

#ifndef S_ISREG
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

/* ---- Paths ---- */

bool is_path_separator(char c) {
#ifdef _WIN32
    if (c == '\\') return true;
#endif
    return c == '/';
}

bool path_is_absolute(const char *path) {
#ifdef _WIN32
    if (((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) && path[1] == ':') return true;
#endif
    return is_path_separator(path[0]);
}

/* The last separator in path, or NULL. */
static const char *last_separator(const char *path) {
    const char *last = NULL;
    for (const char *p = path; *p; p++) {
        if (is_path_separator(*p)) last = p;
    }
    return last;
}

bool file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

bool dir_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

char *path_join(const char *a, const char *b) {
    if (!a || !*a || path_is_absolute(b)) return xstrdup(b);
    size_t la = strlen(a);
    if (is_path_separator(a[la - 1])) return fmt_str("%s%s", a, b);
    return fmt_str("%s/%s", a, b);
}

char *path_dirname(const char *path) {
    const char *slash = last_separator(path);
    if (!slash) return xstrdup(".");
    if (slash == path) return xstrdup("/");
#ifdef _WIN32
    if (slash == path + 2 && path[1] == ':') return xstrndup(path, 3); /* "C:\" */
#endif
    return xstrndup(path, (size_t)(slash - path));
}

char *path_realpath(const char *path) {
#ifdef _WIN32
    char buf[4096];
    if (!file_exists(path) && !dir_exists(path)) return NULL;
    if (!_fullpath(buf, path, sizeof buf)) return NULL;
    return xstrdup(buf);
#else
    char *r = realpath(path, NULL);
    if (!r) return NULL;
    char *copy = xstrdup(r);
    free(r);
    return copy;
#endif
}

static int make_dir(const char *path) {
#ifdef _WIN32
    return _mkdir(path);
#else
    return mkdir(path, 0777);
#endif
}

bool make_dirs(const char *path) {
    char *p = xstrdup(path);
    size_t len = strlen(p);
    for (size_t i = 1; i <= len; i++) {
        if (is_path_separator(p[i]) || p[i] == '\0') {
            char saved = p[i];
            p[i] = '\0';
            bool is_drive = i == 2 && p[1] == ':'; /* "C:" */
            if (!is_drive && !dir_exists(p) && make_dir(p) != 0 && errno != EEXIST) {
                free(p);
                return false;
            }
            p[i] = saved;
        }
    }
    free(p);
    return true;
}

char *executable_path(const char *path) {
#ifdef _WIN32
    const char *slash = last_separator(path);
    const char *name = slash ? slash + 1 : path;
    if (!strchr(name, '.')) return fmt_str("%s.exe", path);
#endif
    return xstrdup(path);
}

char *command_path(const char *exe) {
    if (last_separator(exe)) return xstrdup(exe);
#ifdef _WIN32
    return fmt_str(".\\%s", exe);
#else
    return fmt_str("./%s", exe);
#endif
}

/* ---- Directories ---- */

#ifdef _WIN32

bool list_dir(const char *dir, StrVec *names) {
    WIN32_FIND_DATAA data;
    HANDLE h = FindFirstFileA(path_join(dir, "*"), &data);
    if (h == INVALID_HANDLE_VALUE) return false;
    do {
        if (data.cFileName[0] != '.') vec_push(*names, xstrdup(data.cFileName));
    } while (FindNextFileA(h, &data));
    FindClose(h);
    return true;
}

void remove_dir_tree(const char *dir) {
    WIN32_FIND_DATAA data;
    HANDLE h = FindFirstFileA(path_join(dir, "*"), &data);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0) continue;
            char *path = path_join(dir, data.cFileName);
            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) remove_dir_tree(path);
            else DeleteFileA(path);
        } while (FindNextFileA(h, &data));
        FindClose(h);
    }
    RemoveDirectoryA(dir);
}

char *make_temp_dir(void) {
    char base[MAX_PATH + 1];
    DWORD n = GetTempPathA(sizeof base, base);
    if (n == 0 || n > sizeof base) fatal("can't find the temporary directory");
    for (unsigned i = 0; i < 1000; i++) {
        char *dir = fmt_str("%snirea-%lu-%u", base, (unsigned long)GetCurrentProcessId(), i);
        if (CreateDirectoryA(dir, NULL)) return dir;
        if (GetLastError() != ERROR_ALREADY_EXISTS) break;
    }
    fatal("can't create a temporary directory in %s", base);
    return NULL;
}

#else

bool list_dir(const char *dir, StrVec *names) {
    DIR *d = opendir(dir);
    if (!d) return false;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (ent->d_name[0] != '.') vec_push(*names, xstrdup(ent->d_name));
    }
    closedir(d);
    return true;
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

char *make_temp_dir(void) {
    const char *base = getenv("TMPDIR");
    if (!base || !*base) base = "/tmp";
    char *templ = path_join(base, "nirea-XXXXXX");
    if (!mkdtemp(templ)) fatal("can't create a temporary directory in %s: %s", base, strerror(errno));
    return templ;
}

#endif

/* ---- Processes ---- */

const char *const *default_c_compilers(void) {
#ifdef _WIN32
    static const char *const compilers[] = {"gcc", "clang", NULL};
#else
    static const char *const compilers[] = {"cc", NULL};
#endif
    return compilers;
}

const char *math_library_flag(void) {
#ifdef _WIN32
    return NULL;
#else
    return "-lm";
#endif
}

#ifdef _WIN32

/* Quotes one argument for a Windows command line, which the C runtime of the
 * started program splits back into arguments. */
static const char *quote_arg(const char *arg) {
    if (*arg && !strpbrk(arg, " \t\n\v\"")) return arg;
    StrBuf sb = {0};
    sb_putc(&sb, '"');
    for (const char *p = arg;; p++) {
        size_t backslashes = 0;
        while (*p == '\\') {
            p++;
            backslashes++;
        }
        if (!*p) {
            /* double them, so the closing quote isn't escaped */
            for (size_t i = 0; i < backslashes * 2; i++) sb_putc(&sb, '\\');
            break;
        }
        if (*p == '"') {
            for (size_t i = 0; i < backslashes * 2 + 1; i++) sb_putc(&sb, '\\');
        } else {
            for (size_t i = 0; i < backslashes; i++) sb_putc(&sb, '\\');
        }
        sb_putc(&sb, *p);
    }
    sb_putc(&sb, '"');
    return sb_cstr(&sb);
}

bool run_process(const char *const *argv, int *status) {
    fflush(stdout);
    fflush(stderr);
    /* a name without a directory is searched for, with .exe added */
    char *program = executable_path(argv[0]);
    if (!last_separator(program)) {
        char found[MAX_PATH + 1];
        DWORD n = SearchPathA(NULL, program, NULL, sizeof found, found, NULL);
        if (n == 0 || n > sizeof found) {
            errno = ENOENT;
            return false;
        }
        program = xstrdup(found);
    } else if (!file_exists(program)) {
        errno = ENOENT;
        return false;
    }
    StrBuf cmd = {0};
    for (int i = 0; argv[i]; i++) {
        if (i) sb_putc(&cmd, ' ');
        sb_append(&cmd, quote_arg(argv[i]));
    }
    STARTUPINFOA si;
    memset(&si, 0, sizeof si);
    si.cb = sizeof si;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    PROCESS_INFORMATION pi;
    if (!CreateProcessA(program, (char *)sb_cstr(&cmd), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        DWORD err = GetLastError();
        errno = err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND ? ENOENT : EACCES;
        return false;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 1;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    *status = (int)code;
    return true;
}

#else

bool run_process(const char *const *argv, int *status) {
    fflush(stdout);
    fflush(stderr);
    pid_t pid;
    int err = posix_spawnp(&pid, argv[0], NULL, NULL, (char *const *)argv, environ);
    if (err != 0) {
        errno = err;
        return false;
    }
    int ws;
    while (waitpid(pid, &ws, 0) < 0) {
        if (errno != EINTR) return false;
    }
    if (WIFEXITED(ws)) *status = WEXITSTATUS(ws);
    else if (WIFSIGNALED(ws)) *status = 128 + WTERMSIG(ws);
    else *status = 1;
    return true;
}

#endif
