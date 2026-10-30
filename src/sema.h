#ifndef NIREA_SEMA_H
#define NIREA_SEMA_H

#include "ast.h"

/* Resolves names and checks types for the whole program. Reports errors
 * through error_at; check error_count afterwards. */
void sema_check_program(Program *prog);

/* Checks ownership and move rules (spec section 7). Run after sema succeeds. */
void ownership_check_program(Program *prog);

#endif
