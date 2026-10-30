#ifndef NIREA_DRIVER_H
#define NIREA_DRIVER_H

#include "common.h"

typedef struct BuildOptions {
    const char *input;      /* project directory or .ni file */
    const char *output;     /* executable path (-o), optional */
    const char *emit_c_dir; /* where to keep the generated C (--emit-c), optional */
    const char *cc;         /* C compiler (--cc), optional */
    StrVec cc_args;         /* extra arguments for the C compiler (-l, -L) */
    bool check_only;
    bool for_run; /* single-file builds put the executable in a temporary directory */
} BuildOptions;

/* Checks and, unless check_only, builds the program. On success returns 0 and
 * stores the executable path in *exe_out. temp_dir_out receives a temporary
 * directory the caller must remove with remove_dir_tree, or NULL. */
int nirea_build(BuildOptions *opts, char **exe_out, char **temp_dir_out);

/* Runs a program and returns its exit status (or 128 + signal). */
int run_process(const char *const *argv);

void remove_dir_tree(const char *dir);

#endif
