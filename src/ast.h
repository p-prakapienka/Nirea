/* Syntax tree and semantic types. The parser builds the tree; semantic
 * analysis resolves names in place and fills in types. */
#ifndef NIREA_AST_H
#define NIREA_AST_H

#include "common.h"

typedef struct Decl Decl;
typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Type Type;
typedef struct Var Var;

typedef VEC(Decl *) DeclVec;
typedef VEC(Expr *) ExprVec;
typedef VEC(Stmt *) StmtVec;
typedef VEC(Var *) VarVec;

/* ---- Semantic types ---- */

typedef enum TypeKind {
    TY_ERROR, /* result of an earlier error; compatible with everything */
    TY_VOID,
    TY_BOOL,
    TY_CHAR,
    TY_I8,
    TY_I16,
    TY_I32,
    TY_I64,
    TY_U8,
    TY_U16,
    TY_U32,
    TY_U64,
    TY_F32,
    TY_F64,
    TY_STRING,
    TY_ENUM,
    TY_STRUCT,
    TY_INTERFACE,
    TY_ARRAY,
} TypeKind;

struct Type {
    TypeKind kind;
    Decl *decl;     /* enum, struct, interface */
    Type *elem;     /* array */
    int64_t length; /* array */
    const char *cname;
    bool emitted;   /* codegen bookkeeping */
    int drop_state; /* codegen: 0 unknown, 1 needs no cleanup, 2 needs cleanup */
};

extern Type *type_error, *type_void, *type_bool, *type_char, *type_i8, *type_i16, *type_i32, *type_i64,
    *type_u8, *type_u16, *type_u32, *type_u64, *type_f32, *type_f64, *type_string;

Type *type_array(Type *elem, int64_t length);
int type_array_count(void);
Type *type_array_at(int index);
bool type_is_integer(Type *t);
bool type_is_signed(Type *t);
bool type_is_float(Type *t);
bool type_is_numeric(Type *t);
bool type_is_owned(Type *t); /* struct, array or String */
int type_int_bits(Type *t);
const char *type_name(Type *t);

/* ---- Type syntax ---- */

typedef struct ArrayDim {
    Loc loc;
    bool is_literal;
    uint64_t value;        /* is_literal */
    const char *qualifier; /* namespace or NULL */
    const char *name;      /* constant name */
} ArrayDim;

typedef VEC(ArrayDim) ArrayDimVec;

typedef struct TypeExpr {
    Loc loc;
    Type *prim; /* primitive or String, else NULL */
    const char *qualifier;
    const char *name;
    ArrayDimVec dims;
    bool is_void;
} TypeExpr;

/* ---- Variables ---- */

typedef enum VarKind {
    VAR_LOCAL,
    VAR_PARAM,
} VarKind;

struct Var {
    VarKind kind;
    const char *name;
    Loc loc;
    TypeExpr *type_expr;
    Type *type;
    bool is_ref; /* reference parameter, written T* (spec 7.3) */
    int index;   /* per function, for ownership analysis */
    const char *cname;
};

typedef struct Alias {
    const char *name;
    Loc loc;
    Expr *expr;
} Alias;

/* ---- Expressions ---- */

typedef enum ExprKind {
    /* parsed */
    E_INT,
    E_FLOAT,
    E_CHAR,
    E_STRING,
    E_BOOL,
    E_NAME,   /* name */
    E_MEMBER, /* base.name */
    E_INDEX,  /* base[index] */
    E_CALL,   /* callee(args) */
    E_THIS,
    E_UNARY,
    E_BINARY,
    E_CAST,
    E_SWITCH,
    E_ARRAY_INIT, /* {a, b, c}, only as a declaration initialiser */

    /* resolved */
    E_VAR,         /* local variable or parameter */
    E_FIELD,       /* base.field */
    E_ENUM_VALUE,  /* Enum.VALUE */
    E_CONST,       /* reference to a constant */
    E_ALIAS,       /* use of an alias */
    E_CALL_FUNC,   /* free function or extern function */
    E_CALL_METHOD, /* base.method(args), static dispatch */
    E_CALL_IFACE,  /* base.method(args) through an interface reference */
    E_CALL_CTOR,   /* Struct(args) */
    E_CALL_PRINT,  /* print/println */
    E_CALL_STRING, /* built-in String method (base = receiver) or String.of (no base) */
    E_COPY,        /* base.copy() */
    E_EQUALS,      /* base.equals(arg): structural comparison in place */
    E_LENGTH,      /* base.length */
    E_CONVERT,     /* implicit conversion of operand to type */
    E_TO_IFACE,    /* struct value passed as an interface reference */

    /* placeholders for names that resolved to something that isn't a value */
    E_NAMESPACE_REF,
    E_TYPE_REF,
    E_FUNC_REF,
} ExprKind;

typedef enum SwitchKind {
    SW_ENUM,
    SW_INT,
    SW_CHAR,
    SW_BOOL,
    SW_STRING,
} SwitchKind;

typedef struct CaseValue {
    Loc loc;
    Expr *expr; /* literal, negated literal or bare name (enum value) */
    /* resolved */
    int64_t ivalue;
    uint64_t uvalue;
    bool is_negative;
    const char *svalue;
    size_t slen;
} CaseValue;

typedef VEC(CaseValue) CaseValueVec;

typedef struct SwitchArm {
    Loc loc;
    bool is_default;
    CaseValueVec values;
    Expr *expr; /* switch expression arm */
    Stmt *body; /* switch statement arm */
} SwitchArm;

typedef VEC(SwitchArm) SwitchArmVec;

typedef struct SwitchData {
    Expr *subject;
    SwitchArmVec arms;
    SwitchKind kind;
    bool has_default;
} SwitchData;

struct Expr {
    ExprKind kind;
    Loc loc;
    Type *type;
    bool parenthesised;

    /* literals */
    uint64_t int_value;
    bool negative; /* negated literal */
    double float_value;
    bool bool_value;
    const char *str; /* string literal bytes or names */
    size_t str_len;

    const char *name; /* E_NAME, E_MEMBER, method name */

    Expr *base;  /* member, index, call receiver, unary/cast operand, convert */
    Expr *index; /* E_INDEX; right operand of E_BINARY */
    ExprVec args;
    int op; /* TokenKind for unary/binary */

    Type *cast_type;     /* E_CAST target, E_CONVERT target */
    TypeExpr *cast_expr; /* E_CAST target syntax */

    SwitchData *sw;

    /* resolved */
    Var *var;
    Decl *decl;   /* field, enum, const, function, method, struct, namespace */
    int enum_index;
    Alias *alias;
    int iface_method_index;
    Decl *iface_impl_struct; /* E_TO_IFACE: struct being converted */
    bool transfers_ownership; /* E_VAR whose object goes to a new owner */
    bool implicit_copy;       /* existing object copied where it would otherwise transfer (spec 6.5, 7.3) */
    bool literal_untyped;    /* literal whose type came from the default, not context */
    bool untyped_literal;    /* literal, or arithmetic on literals only (set by the parser) */
    bool int_literal;        /* the same, with integer literals only */
    int depth;               /* height of the expression tree, to bound recursion */
};

/* ---- Statements ---- */

typedef enum StmtKind {
    S_BLOCK,
    S_VAR,
    S_ALIAS,
    S_EXPR,
    S_ASSIGN,
    S_INCDEC,
    S_IF,
    S_WHILE,
    S_DO,
    S_FOR,
    S_SWITCH,
    S_BREAK,
    S_CONTINUE,
    S_RETURN,
} StmtKind;

struct Stmt {
    StmtKind kind;
    Loc loc;
    StmtVec stmts;  /* block */
    Var *var;       /* S_VAR */
    Expr *init;     /* S_VAR initialiser */
    Alias *alias;   /* S_ALIAS */
    Expr *expr;     /* S_EXPR, S_RETURN value, condition, assignment target */
    Expr *value;    /* S_ASSIGN right-hand side */
    int op;         /* S_ASSIGN operator token (TOK_ASSIGN, TOK_PLUS_ASSIGN, ...), S_INCDEC token */
    Stmt *then_branch;
    Stmt *else_branch;
    Stmt *body;
    Stmt *for_init;
    Stmt *for_update;
    SwitchData *sw;
    int loop_id; /* codegen labels */
};

/* ---- Declarations ---- */

typedef enum DeclKind {
    D_FUNC,
    D_EXTERN_FUNC,
    D_STRUCT,
    D_INTERFACE,
    D_ENUM,
    D_CONST,
    D_NAMESPACE,
    D_FIELD,
    D_METHOD,
    D_CTOR,
    D_IFACE_METHOD,
} DeclKind;

typedef struct ImportDecl {
    Loc loc;
    const char *path;
} ImportDecl;

typedef struct EnumValue {
    const char *name;
    Loc loc;
} EnumValue;

typedef VEC(EnumValue) EnumValueVec;

typedef struct ImplementsRef {
    Loc loc;
    const char *qualifier;
    const char *name;
    Decl *decl;
} ImplementsRef;

typedef VEC(ImplementsRef) ImplementsVec;

struct Decl {
    DeclKind kind;
    const char *name;
    Loc loc;
    SourceFile *file;
    Decl *ns;    /* enclosing namespace or NULL */
    Decl *owner; /* struct or interface for members */
    bool is_private;
    int id;
    const char *cname;

    /* functions, methods, constructors, extern functions, interface methods */
    TypeExpr *ret_expr;
    Type *ret_type;
    VarVec params;
    bool variadic;
    Stmt *body;
    VarVec locals; /* every local and parameter, for ownership analysis */
    const char *extern_abi;

    /* struct / interface / namespace / enum */
    DeclVec members;  /* fields, methods, ctor; interface methods; namespace members */
    Map member_map;   /* name -> Decl */
    ImplementsVec implements;
    Decl *ctor;
    Type *type;
    EnumValueVec values;
    int field_index;
    int state; /* cycle detection */

    /* fields */
    TypeExpr *field_type_expr;
    Type *field_type;

    /* constants */
    TypeExpr *const_type_expr;
    Type *const_type;
    Expr *const_value;
};

typedef struct Program {
    VEC(SourceFile *) files;
    Map globals; /* name -> Decl (top-level, including namespaces) */
    DeclVec all_structs;
    DeclVec all_interfaces;
    DeclVec all_enums;
    DeclVec all_functions; /* free functions, methods, constructors */
    DeclVec all_externs;
    DeclVec all_consts;
    VEC(Type *) array_types;
    Decl *main;
} Program;

#endif
