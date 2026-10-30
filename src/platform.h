/* Operating-system services: paths, directories, processes and temporary
 * files, for POSIX systems and Windows. */
#ifndef NIREA_PLATFORM_H
#define NIREA_PLATFORM_H

#include "common.h"

/* True for '/', and on Windows also for '\\'. */
bool is_path_separator(char c);

/* True for "/x", and on Windows also for "\\x", "C:\\x" and "C:/x". */
bool path_is_absolute(const char *path);

bool file_exists(const char *path);
bool dir_exists(const char *path);
char *path_join(const char *a, const char *b); /* b itself if it is absolute */
char *path_dirname(const char *path);
char *path_realpath(const char *path); /* NULL if missing */
bool make_dirs(const char *path);

/* The names in a directory, except those starting with '.'. Returns false if
 * the directory can't be read. */
bool list_dir(const char *dir, StrVec *names);

/* Removes a directory and everything in it. */
void remove_dir_tree(const char *dir);

/* Creates a new, empty temporary directory and returns its path. */
char *make_temp_dir(void);

/* The path of an executable: on Windows, ".exe" is added when the file name
 * has no extension. */
char *executable_path(const char *path);

/* The path to run an executable by: a bare file name is taken from the
 * current directory, not from PATH. */
char *command_path(const char *exe);

/* The C compilers to try, in order, when neither --cc nor $CC names one:
 * cc on POSIX systems; gcc (MinGW-w64), then clang on Windows. NULL ends
 * the list. */
const char *const *default_c_compilers(void);

/* The C compiler argument that links the math library, or NULL if the
 * C library already includes it. */
const char *math_library_flag(void);

/* Runs a program, searching PATH for argv[0], and waits for it. Stores its
 * exit status (or 128 + signal number) in *status. Returns false with errno
 * set if it can't be started; errno is ENOENT if the program doesn't exist. */
bool run_process(const char *const *argv, int *status);

#endif
