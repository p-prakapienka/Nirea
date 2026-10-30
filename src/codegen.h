#ifndef NIREA_CODEGEN_H
#define NIREA_CODEGEN_H

#include "ast.h"

/* Generates the C translation unit for a checked program. */
char *codegen_program(Program *prog);

#endif
