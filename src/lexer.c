#include "lexer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct Lexer {
    SourceFile *file;
    const char *src;
    size_t pos;
    size_t len;
    int line;
    int col;
    TokenVec tokens;
} Lexer;

static const struct {
    const char *word;
    TokenKind kind;
} keywords[] = {
    {"alias", TOK_ALIAS},         {"boolean", TOK_BOOLEAN},     {"break", TOK_BREAK},
    {"case", TOK_CASE},           {"char", TOK_CHAR_KW},        {"const", TOK_CONST},
    {"continue", TOK_CONTINUE},   {"default", TOK_DEFAULT},     {"do", TOK_DO},
    {"else", TOK_ELSE},           {"enum", TOK_ENUM},           {"extern", TOK_EXTERN},
    {"false", TOK_FALSE},         {"float", TOK_FLOAT_KW},      {"float64", TOK_FLOAT64},
    {"for", TOK_FOR},             {"if", TOK_IF},               {"import", TOK_IMPORT},
    {"int", TOK_INT_KW},          {"int8", TOK_INT8},           {"int16", TOK_INT16},
    {"int64", TOK_INT64},         {"interface", TOK_INTERFACE}, {"namespace", TOK_NAMESPACE},
    {"private", TOK_PRIVATE},     {"return", TOK_RETURN},       {"String", TOK_STRING_KW},
    {"struct", TOK_STRUCT},       {"switch", TOK_SWITCH},       {"this", TOK_THIS},
    {"true", TOK_TRUE},           {"uint", TOK_UINT},           {"uint8", TOK_UINT8},
    {"uint16", TOK_UINT16},       {"uint64", TOK_UINT64},       {"void", TOK_VOID},
    {"while", TOK_WHILE},
};

static const char *kind_names[TOK_COUNT] = {
    [TOK_EOF] = "end of file",
    [TOK_IDENT] = "identifier",
    [TOK_INT] = "integer literal",
    [TOK_FLOAT] = "floating-point literal",
    [TOK_CHAR] = "character literal",
    [TOK_STRING] = "string literal",
    [TOK_IMPORT_PATH] = "import path",
    [TOK_LPAREN] = "'('",
    [TOK_RPAREN] = "')'",
    [TOK_LBRACE] = "'{'",
    [TOK_RBRACE] = "'}'",
    [TOK_LBRACKET] = "'['",
    [TOK_RBRACKET] = "']'",
    [TOK_SEMI] = "';'",
    [TOK_COMMA] = "','",
    [TOK_DOT] = "'.'",
    [TOK_ELLIPSIS] = "'...'",
    [TOK_ARROW] = "'->'",
    [TOK_COLON] = "':'",
    [TOK_PLUS] = "'+'",
    [TOK_MINUS] = "'-'",
    [TOK_STAR] = "'*'",
    [TOK_SLASH] = "'/'",
    [TOK_PERCENT] = "'%'",
    [TOK_AMP] = "'&'",
    [TOK_PIPE] = "'|'",
    [TOK_CARET] = "'^'",
    [TOK_TILDE] = "'~'",
    [TOK_BANG] = "'!'",
    [TOK_ASSIGN] = "'='",
    [TOK_EQ] = "'=='",
    [TOK_NE] = "'!='",
    [TOK_LT] = "'<'",
    [TOK_LE] = "'<='",
    [TOK_GT] = "'>'",
    [TOK_GE] = "'>='",
    [TOK_SHL] = "'<<'",
    [TOK_SHR] = "'>>'",
    [TOK_ANDAND] = "'&&'",
    [TOK_OROR] = "'||'",
    [TOK_PLUS_ASSIGN] = "'+='",
    [TOK_MINUS_ASSIGN] = "'-='",
    [TOK_STAR_ASSIGN] = "'*='",
    [TOK_SLASH_ASSIGN] = "'/='",
    [TOK_PERCENT_ASSIGN] = "'%='",
    [TOK_AMP_ASSIGN] = "'&='",
    [TOK_PIPE_ASSIGN] = "'|='",
    [TOK_CARET_ASSIGN] = "'^='",
    [TOK_SHL_ASSIGN] = "'<<='",
    [TOK_SHR_ASSIGN] = "'>>='",
    [TOK_PLUSPLUS] = "'++'",
    [TOK_MINUSMINUS] = "'--'",
};

const char *token_kind_name(TokenKind kind) {
    if (kind >= TOK_ALIAS && kind <= TOK_WHILE) {
        for (size_t i = 0; i < sizeof keywords / sizeof keywords[0]; i++) {
            if (keywords[i].kind == kind) return fmt_str("'%s'", keywords[i].word);
        }
    }
    return kind_names[kind] ? kind_names[kind] : "token";
}

static char peek_at(Lexer *lx, size_t offset) {
    size_t p = lx->pos + offset;
    return p < lx->len ? lx->src[p] : '\0';
}

static char peek(Lexer *lx) {
    return peek_at(lx, 0);
}

static char advance(Lexer *lx) {
    char c = lx->src[lx->pos++];
    if (c == '\n') {
        lx->line++;
        lx->col = 1;
    } else {
        lx->col++;
    }
    return c;
}

static Loc here(Lexer *lx) {
    return (Loc){lx->file, lx->line, lx->col};
}

static Token *push(Lexer *lx, TokenKind kind, Loc loc) {
    Token t = {0};
    t.kind = kind;
    t.loc = loc;
    vec_push(lx->tokens, t);
    return &lx->tokens.data[lx->tokens.len - 1];
}

static void skip_space(Lexer *lx) {
    for (;;) {
        char c = peek(lx);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f') {
            advance(lx);
        } else if (c == '/' && peek_at(lx, 1) == '/') {
            while (lx->pos < lx->len && peek(lx) != '\n') advance(lx);
        } else if (c == '/' && peek_at(lx, 1) == '*') {
            Loc start = here(lx);
            advance(lx);
            advance(lx);
            for (;;) {
                if (lx->pos >= lx->len) {
                    error_at(start, "unterminated comment");
                    return;
                }
                if (peek(lx) == '*' && peek_at(lx, 1) == '/') {
                    advance(lx);
                    advance(lx);
                    break;
                }
                advance(lx);
            }
        } else {
            return;
        }
    }
}

static int hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* Reads one character of a char or string literal, decoding escapes.
 * Returns -1 on error (already reported). */
static int read_literal_char(Lexer *lx) {
    Loc loc = here(lx);
    char c = advance(lx);
    if (c != '\\') return (unsigned char)c;
    if (lx->pos >= lx->len) {
        error_at(loc, "unterminated escape sequence");
        return -1;
    }
    char e = advance(lx);
    switch (e) {
    case 'n': return '\n';
    case 't': return '\t';
    case 'r': return '\r';
    case '0': return '\0';
    case '\\': return '\\';
    case '\'': return '\'';
    case '"': return '"';
    case 'x': {
        int h1 = hex_value(peek(lx));
        int h2 = h1 >= 0 ? hex_value(peek_at(lx, 1)) : -1;
        if (h1 < 0 || h2 < 0) {
            error_at(loc, "'\\x' must be followed by two hexadecimal digits");
            return -1;
        }
        advance(lx);
        advance(lx);
        return h1 * 16 + h2;
    }
    default:
        error_at(loc, "unknown escape sequence '\\%c'", e);
        return -1;
    }
}

static void lex_string(Lexer *lx, Loc loc) {
    advance(lx); /* opening quote */
    StrBuf sb = {0};
    for (;;) {
        if (lx->pos >= lx->len || peek(lx) == '\n') {
            error_at(loc, "unterminated string literal");
            break;
        }
        if (peek(lx) == '"') {
            advance(lx);
            break;
        }
        int c = read_literal_char(lx);
        if (c >= 0) sb_putc(&sb, (char)c);
    }
    Token *t = push(lx, TOK_STRING, loc);
    t->text = intern(sb.data ? sb.data : "", sb.len);
    t->text_len = sb.len;
    free(sb.data);
}

static void lex_char(Lexer *lx, Loc loc) {
    advance(lx); /* opening quote */
    int value = 0;
    if (peek(lx) == '\'' || peek(lx) == '\n' || lx->pos >= lx->len) {
        error_at(loc, "empty character literal");
    } else {
        value = read_literal_char(lx);
        if (value < 0) value = 0;
    }
    if (peek(lx) != '\'') {
        error_at(loc, "character literal must hold exactly one byte");
        while (lx->pos < lx->len && peek(lx) != '\'' && peek(lx) != '\n') advance(lx);
    }
    if (peek(lx) == '\'') advance(lx);
    Token *t = push(lx, TOK_CHAR, loc);
    t->int_value = (uint64_t)value;
}

static bool is_ident_start(char c) {
    return isalpha((unsigned char)c) || c == '_';
}

static bool is_ident_char(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

static void lex_number(Lexer *lx, Loc loc) {
    size_t start = lx->pos;
    int base = 10;
    if (peek(lx) == '0' && (peek_at(lx, 1) == 'x' || peek_at(lx, 1) == 'X')) {
        base = 16;
    } else if (peek(lx) == '0' && (peek_at(lx, 1) == 'b' || peek_at(lx, 1) == 'B')) {
        base = 2;
    }
    if (base != 10) {
        advance(lx);
        advance(lx);
        uint64_t value = 0;
        bool overflow = false;
        int digits = 0;
        bool prev_digit = false;
        while (is_ident_char(peek(lx))) {
            char c = advance(lx);
            if (c == '_') {
                int next_value = hex_value(peek(lx));
                if (!prev_digit || next_value < 0 || next_value >= base) error_at(loc, "'_' must separate digits");
                prev_digit = false;
                continue;
            }
            prev_digit = true;
            int d = hex_value(c);
            if (d < 0 || d >= base) {
                error_at(loc, "invalid digit '%c' in %s literal", c, base == 16 ? "hexadecimal" : "binary");
                continue;
            }
            if (value > (UINT64_MAX - (uint64_t)d) / (uint64_t)base) overflow = true;
            value = value * (uint64_t)base + (uint64_t)d;
            digits++;
        }
        if (digits == 0) error_at(loc, "missing digits in %s literal", base == 16 ? "hexadecimal" : "binary");
        if (overflow) error_at(loc, "integer literal is too large");
        Token *t = push(lx, TOK_INT, loc);
        t->int_value = value;
        return;
    }

    bool is_float = false;
    while (isdigit((unsigned char)peek(lx)) || peek(lx) == '_') advance(lx);
    if (peek(lx) == '.' && isdigit((unsigned char)peek_at(lx, 1))) {
        is_float = true;
        advance(lx);
        while (isdigit((unsigned char)peek(lx)) || peek(lx) == '_') advance(lx);
    }
    if (peek(lx) == 'e' || peek(lx) == 'E') {
        char sign = peek_at(lx, 1);
        size_t digit_offset = (sign == '+' || sign == '-') ? 2 : 1;
        if (isdigit((unsigned char)peek_at(lx, digit_offset))) {
            is_float = true;
            advance(lx);
            if (digit_offset == 2) advance(lx);
            while (isdigit((unsigned char)peek(lx)) || peek(lx) == '_') advance(lx);
        }
    }
    if (is_ident_char(peek(lx))) {
        Loc bad = here(lx);
        while (is_ident_char(peek(lx))) advance(lx);
        error_at(bad, "invalid suffix on numeric literal");
    }

    /* Copy without underscores. */
    StrBuf sb = {0};
    for (size_t i = start; i < lx->pos; i++) {
        char c = lx->src[i];
        if (c == '_') {
            bool prev_digit = i > start && isxdigit((unsigned char)lx->src[i - 1]);
            bool next_digit = i + 1 < lx->pos && isdigit((unsigned char)lx->src[i + 1]);
            if (!prev_digit || !next_digit) error_at(loc, "'_' must separate digits");
            continue;
        }
        sb_putc(&sb, c);
    }
    const char *text = sb_cstr(&sb);

    if (is_float) {
        Token *t = push(lx, TOK_FLOAT, loc);
        t->float_value = strtod(text, NULL);
        t->text = intern(text, strlen(text));
        t->text_len = strlen(text);
    } else {
        if (text[0] == '0' && text[1] != '\0') error_at(loc, "a decimal literal can't start with '0'");
        uint64_t value = 0;
        bool overflow = false;
        for (const char *p = text; *p; p++) {
            uint64_t d = (uint64_t)(*p - '0');
            if (value > (UINT64_MAX - d) / 10) overflow = true;
            value = value * 10 + d;
        }
        if (overflow) error_at(loc, "integer literal is too large");
        Token *t = push(lx, TOK_INT, loc);
        t->int_value = value;
    }
    free(sb.data);
}

static void lex_import_path(Lexer *lx) {
    while (peek(lx) == ' ' || peek(lx) == '\t') advance(lx);
    Loc loc = here(lx);
    size_t start = lx->pos;
    while (lx->pos < lx->len && peek(lx) != ';' && peek(lx) != '\n') advance(lx);
    size_t end = lx->pos;
    while (end > start && (lx->src[end - 1] == ' ' || lx->src[end - 1] == '\t' || lx->src[end - 1] == '\r')) end--;
    if (end == start) {
        error_at(loc, "expected a file path after 'import'");
        return;
    }
    Token *t = push(lx, TOK_IMPORT_PATH, loc);
    t->text = intern(lx->src + start, end - start);
    t->text_len = end - start;
}

typedef struct Punct {
    const char *text;
    TokenKind kind;
} Punct;

/* Longest first. */
static const Punct puncts[] = {
    {"<<=", TOK_SHL_ASSIGN}, {">>=", TOK_SHR_ASSIGN}, {"...", TOK_ELLIPSIS},
    {"->", TOK_ARROW},       {"==", TOK_EQ},          {"!=", TOK_NE},
    {"<=", TOK_LE},          {">=", TOK_GE},          {"<<", TOK_SHL},
    {">>", TOK_SHR},         {"&&", TOK_ANDAND},      {"||", TOK_OROR},
    {"+=", TOK_PLUS_ASSIGN}, {"-=", TOK_MINUS_ASSIGN}, {"*=", TOK_STAR_ASSIGN},
    {"/=", TOK_SLASH_ASSIGN}, {"%=", TOK_PERCENT_ASSIGN}, {"&=", TOK_AMP_ASSIGN},
    {"|=", TOK_PIPE_ASSIGN}, {"^=", TOK_CARET_ASSIGN}, {"++", TOK_PLUSPLUS},
    {"--", TOK_MINUSMINUS},  {"(", TOK_LPAREN},       {")", TOK_RPAREN},
    {"{", TOK_LBRACE},       {"}", TOK_RBRACE},       {"[", TOK_LBRACKET},
    {"]", TOK_RBRACKET},     {";", TOK_SEMI},         {",", TOK_COMMA},
    {".", TOK_DOT},          {":", TOK_COLON},        {"+", TOK_PLUS},
    {"-", TOK_MINUS},        {"*", TOK_STAR},         {"/", TOK_SLASH},
    {"%", TOK_PERCENT},      {"&", TOK_AMP},          {"|", TOK_PIPE},
    {"^", TOK_CARET},        {"~", TOK_TILDE},        {"!", TOK_BANG},
    {"=", TOK_ASSIGN},       {"<", TOK_LT},           {">", TOK_GT},
};

TokenVec lex_file(SourceFile *file) {
    Lexer lx = {0};
    lx.file = file;
    lx.src = file->text;
    lx.len = file->len;
    lx.line = 1;
    lx.col = 1;

    for (;;) {
        skip_space(&lx);
        if (lx.pos >= lx.len) break;
        Loc loc = here(&lx);
        char c = peek(&lx);

        if (is_ident_start(c)) {
            size_t start = lx.pos;
            while (is_ident_char(peek(&lx))) advance(&lx);
            size_t n = lx.pos - start;
            TokenKind kind = TOK_IDENT;
            for (size_t i = 0; i < sizeof keywords / sizeof keywords[0]; i++) {
                if (strlen(keywords[i].word) == n && memcmp(keywords[i].word, lx.src + start, n) == 0) {
                    kind = keywords[i].kind;
                    break;
                }
            }
            Token *t = push(&lx, kind, loc);
            t->text = intern(lx.src + start, n);
            t->text_len = n;
            if (kind == TOK_IMPORT) lex_import_path(&lx);
            continue;
        }
        if (isdigit((unsigned char)c)) {
            lex_number(&lx, loc);
            continue;
        }
        if (c == '"') {
            lex_string(&lx, loc);
            continue;
        }
        if (c == '\'') {
            lex_char(&lx, loc);
            continue;
        }

        bool matched = false;
        for (size_t i = 0; i < sizeof puncts / sizeof puncts[0]; i++) {
            size_t n = strlen(puncts[i].text);
            if (lx.pos + n <= lx.len && memcmp(lx.src + lx.pos, puncts[i].text, n) == 0) {
                for (size_t k = 0; k < n; k++) advance(&lx);
                push(&lx, puncts[i].kind, loc);
                matched = true;
                break;
            }
        }
        if (!matched) {
            if ((unsigned char)c >= 0x80) {
                error_at(loc, "unexpected non-ASCII character outside a string or comment");
                while (lx.pos < lx.len && (unsigned char)peek(&lx) >= 0x80) advance(&lx);
            } else {
                error_at(loc, "unexpected character '%c'", c);
                advance(&lx);
            }
        }
    }
    push(&lx, TOK_EOF, here(&lx));
    return lx.tokens;
}
