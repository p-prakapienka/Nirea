#ifndef NIREA_LEXER_H
#define NIREA_LEXER_H

#include "common.h"

typedef enum TokenKind {
    TOK_EOF,
    TOK_IDENT,
    TOK_INT,
    TOK_FLOAT,
    TOK_CHAR,
    TOK_STRING,
    TOK_IMPORT_PATH,

    /* keywords */
    TOK_ALIAS,
    TOK_BOOLEAN,
    TOK_BREAK,
    TOK_CASE,
    TOK_CHAR_KW,
    TOK_CONST,
    TOK_CONTINUE,
    TOK_DEFAULT,
    TOK_DO,
    TOK_ELSE,
    TOK_ENUM,
    TOK_EXTERN,
    TOK_FALSE,
    TOK_FLOAT_KW,
    TOK_FLOAT64,
    TOK_FOR,
    TOK_IF,
    TOK_IMPORT,
    TOK_INT_KW,
    TOK_INT8,
    TOK_INT16,
    TOK_INT64,
    TOK_INTERFACE,
    TOK_IS,
    TOK_NAMESPACE,
    TOK_PRIVATE,
    TOK_RETURN,
    TOK_STRING_KW,
    TOK_STRUCT,
    TOK_SWITCH,
    TOK_THIS,
    TOK_TRUE,
    TOK_UINT,
    TOK_UINT8,
    TOK_UINT16,
    TOK_UINT64,
    TOK_VOID,
    TOK_WHILE,

    /* punctuation */
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_SEMI,
    TOK_COMMA,
    TOK_DOT,
    TOK_ELLIPSIS,
    TOK_ARROW,
    TOK_COLON,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_PERCENT,
    TOK_AMP,
    TOK_PIPE,
    TOK_CARET,
    TOK_TILDE,
    TOK_BANG,
    TOK_ASSIGN,
    TOK_EQ,
    TOK_NE,
    TOK_LT,
    TOK_LE,
    TOK_GT,
    TOK_GE,
    TOK_SHL,
    TOK_SHR,
    TOK_ANDAND,
    TOK_OROR,
    TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN,
    TOK_STAR_ASSIGN,
    TOK_SLASH_ASSIGN,
    TOK_PERCENT_ASSIGN,
    TOK_AMP_ASSIGN,
    TOK_PIPE_ASSIGN,
    TOK_CARET_ASSIGN,
    TOK_SHL_ASSIGN,
    TOK_SHR_ASSIGN,
    TOK_PLUSPLUS,
    TOK_MINUSMINUS,

    TOK_COUNT
} TokenKind;

typedef struct Token {
    TokenKind kind;
    Loc loc;
    const char *text; /* interned: identifier name, import path or decoded string bytes */
    size_t text_len;  /* length of decoded string bytes */
    uint64_t int_value;
    double float_value;
} Token;

typedef VEC(Token) TokenVec;

/* Tokenises the whole file. Reports errors through error_at. */
TokenVec lex_file(SourceFile *file);

const char *token_kind_name(TokenKind kind);

#endif
