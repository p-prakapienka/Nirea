#ifndef NIREA_PARSER_H
#define NIREA_PARSER_H

#include "ast.h"
#include "lexer.h"

/* Parses a lexed file, filling file->imports and file->decls. Stops at the
 * first syntax error in the file. Returns false if there was an error. */
bool parse_file(SourceFile *file, TokenVec tokens);

#endif
