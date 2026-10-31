/* The nirea command-line tool. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver.h"

#define NIREA_VERSION "0.12.0"

static void usage(FILE *f) {
    fprintf(f,
            "Usage:\n"
            "  nirea build [PROJECT_DIR | FILE.ni] [options]   build an executable\n"
            "  nirea run   [PROJECT_DIR | FILE.ni] [options] [-- ARGS...]\n"
            "                                                  build, then run the program\n"
            "  nirea check [PROJECT_DIR | FILE.ni]             check the program without building it\n"
            "\n"
            "A project directory holds sources in src/main/nirea and builds into target/\n"
            "(generated C in target/generated-c, executable target/nirea-app, with .exe on\n"
            "Windows). The default is the current directory.\n"
            "\n"
            "Options:\n"
            "  -o FILE          write the executable to FILE\n"
            "  --emit-c DIR     write the generated C to DIR\n"
            "  --cc COMPILER    C compiler to use (default: $CC, then cc; on Windows gcc,\n"
            "                   then clang)\n"
            "  -lLIB, -LDIR     passed to the C compiler, e.g. to link C libraries\n"
            "  --version        print the compiler version\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(stderr);
        return 2;
    }
    const char *cmd = argv[1];
    if (strcmp(cmd, "--help") == 0 || strcmp(cmd, "-h") == 0 || strcmp(cmd, "help") == 0) {
        usage(stdout);
        return 0;
    }
    if (strcmp(cmd, "--version") == 0 || strcmp(cmd, "version") == 0) {
        printf("nirea %s\n", NIREA_VERSION);
        return 0;
    }
    bool is_build = strcmp(cmd, "build") == 0;
    bool is_run = strcmp(cmd, "run") == 0;
    bool is_check = strcmp(cmd, "check") == 0;
    if (!is_build && !is_run && !is_check) {
        fprintf(stderr, "nirea: error: unknown command '%s'\n\n", cmd);
        usage(stderr);
        return 2;
    }

    BuildOptions opts = {0};
    opts.check_only = is_check;
    int prog_args_start = argc;
    for (int i = 2; i < argc; i++) {
        const char *a = argv[i];
        if (strcmp(a, "--") == 0) {
            prog_args_start = i + 1;
            break;
        }
        bool needs_value = strcmp(a, "-o") == 0 || strcmp(a, "--emit-c") == 0 || strcmp(a, "--cc") == 0 ||
                           strcmp(a, "-l") == 0 || strcmp(a, "-L") == 0;
        if (needs_value && i + 1 >= argc) {
            fprintf(stderr, "nirea: error: option %s needs a value\n", a);
            return 2;
        }
        if (strcmp(a, "-o") == 0) {
            opts.output = argv[++i];
        } else if (strcmp(a, "--emit-c") == 0) {
            opts.emit_c_dir = argv[++i];
        } else if (strcmp(a, "--cc") == 0) {
            opts.cc = argv[++i];
        } else if (strcmp(a, "-l") == 0 || strcmp(a, "-L") == 0) {
            vec_push(opts.cc_args, fmt_str("%s%s", a, argv[i + 1]));
            i++;
        } else if (strncmp(a, "-l", 2) == 0 || strncmp(a, "-L", 2) == 0) {
            vec_push(opts.cc_args, a);
        } else if (a[0] == '-') {
            fprintf(stderr, "nirea: error: unknown option '%s'\n", a);
            return 2;
        } else if (!opts.input) {
            opts.input = a;
        } else {
            fprintf(stderr, "nirea: error: unexpected argument '%s'\n", a);
            return 2;
        }
    }

    opts.for_run = is_run;
    char *exe = NULL;
    char *temp_dir = NULL;
    int status = nirea_build(&opts, &exe, &temp_dir);
    if (status != 0 || !is_run) {
        if (temp_dir) remove_dir_tree(temp_dir);
        return status;
    }
    StrVec pargv = {0};
    vec_push(pargv, command_path(exe));
    for (int i = prog_args_start; i < argc; i++) vec_push(pargv, argv[i]);
    vec_push(pargv, NULL);
    if (!run_process(pargv.data, &status)) {
        fprintf(stderr, "nirea: error: can't run %s: %s\n", exe, strerror(errno));
        status = 127;
    }
    if (temp_dir) remove_dir_tree(temp_dir);
    return status;
}
