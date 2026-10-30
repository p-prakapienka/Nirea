#include "parser.h"

#include <setjmp.h>
#include <string.h>

typedef struct Parser {
    SourceFile *file;
    Token *toks;
    int pos;
    int count;
    jmp_buf bail;
} Parser;

static Token *cur(Parser *p) {
    return &p->toks[p->pos];
}

static Token *peek_tok(Parser *p, int offset) {
    int i = p->pos + offset;
    if (i >= p->count) i = p->count - 1;
    return &p->toks[i];
}

static bool at(Parser *p, TokenKind kind) {
    return cur(p)->kind == kind;
}

static Token *next(Parser *p) {
    Token *t = cur(p);
    if (p->pos < p->count - 1) p->pos++;
    return t;
}

static bool accept(Parser *p, TokenKind kind) {
    if (!at(p, kind)) return false;
    next(p);
    return true;
}

static const char *describe(Token *t) {
    switch (t->kind) {
    case TOK_IDENT: return fmt_str("identifier '%s'", t->text);
    case TOK_INT: return "integer literal";
    case TOK_FLOAT: return "floating-point literal";
    case TOK_STRING: return "string literal";
    case TOK_CHAR: return "character literal";
    default: return token_kind_name(t->kind);
    }
}

static _Noreturn void fail_at(Parser *p, Loc loc, const char *msg) {
    error_at(loc, "%s", msg);
    longjmp(p->bail, 1);
}

static _Noreturn void fail_expected(Parser *p, const char *what) {
    fail_at(p, cur(p)->loc, fmt_str("expected %s, found %s", what, describe(cur(p))));
}

static Token *expect(Parser *p, TokenKind kind) {
    if (!at(p, kind)) fail_expected(p, token_kind_name(kind));
    return next(p);
}

static const char *expect_ident(Parser *p, const char *what) {
    if (!at(p, TOK_IDENT)) fail_expected(p, what);
    return next(p)->text;
}

static Type *primitive_for(TokenKind kind) {
    switch (kind) {
    case TOK_BOOLEAN: return type_bool;
    case TOK_CHAR_KW: return type_char;
    case TOK_INT8: return type_i8;
    case TOK_INT16: return type_i16;
    case TOK_INT_KW: return type_i32;
    case TOK_INT64: return type_i64;
    case TOK_UINT8: return type_u8;
    case TOK_UINT16: return type_u16;
    case TOK_UINT: return type_u32;
    case TOK_UINT64: return type_u64;
    case TOK_FLOAT_KW: return type_f32;
    case TOK_FLOAT64: return type_f64;
    case TOK_STRING_KW: return type_string;
    default: return NULL;
    }
}

/* ---- Types ---- */

/* Returns the index just past a type starting at index i, or -1. */
static int scan_type(Parser *p, int i) {
    if (i >= p->count) return -1;
    TokenKind k = p->toks[i].kind;
    if (primitive_for(k)) {
        i++;
    } else if (k == TOK_IDENT) {
        i++;
        if (p->toks[i].kind == TOK_DOT && p->toks[i + 1].kind == TOK_IDENT) i += 2;
    } else {
        return -1;
    }
    while (p->toks[i].kind == TOK_LBRACKET) {
        i++;
        if (p->toks[i].kind == TOK_INT) {
            i++;
        } else if (p->toks[i].kind == TOK_IDENT) {
            i++;
            if (p->toks[i].kind == TOK_DOT && p->toks[i + 1].kind == TOK_IDENT) i += 2;
        } else {
            return -1;
        }
        if (p->toks[i].kind != TOK_RBRACKET) return -1;
        i++;
    }
    return i;
}

static bool at_declaration(Parser *p) {
    int end = scan_type(p, p->pos);
    if (end < 0) return false;
    if (p->toks[end].kind == TOK_IDENT) return true;
    /* `Engine* e = ...;` or `Engine* e;`: a reference variable, rejected later */
    return p->toks[end].kind == TOK_STAR && p->toks[end + 1].kind == TOK_IDENT &&
           (p->toks[end + 2].kind == TOK_ASSIGN || p->toks[end + 2].kind == TOK_SEMI);
}

static TypeExpr *parse_type(Parser *p) {
    TypeExpr *te = NEW(TypeExpr);
    te->loc = cur(p)->loc;
    Type *prim = primitive_for(cur(p)->kind);
    if (prim) {
        next(p);
        te->prim = prim;
    } else if (at(p, TOK_IDENT)) {
        te->name = next(p)->text;
        if (at(p, TOK_DOT) && peek_tok(p, 1)->kind == TOK_IDENT) {
            next(p);
            te->qualifier = te->name;
            te->name = next(p)->text;
        }
    } else {
        fail_expected(p, "a type");
    }
    while (at(p, TOK_LBRACKET)) {
        ArrayDim dim = {0};
        dim.loc = cur(p)->loc;
        next(p);
        if (at(p, TOK_INT)) {
            dim.is_literal = true;
            dim.value = next(p)->int_value;
        } else if (at(p, TOK_IDENT)) {
            dim.name = next(p)->text;
            if (at(p, TOK_DOT)) {
                next(p);
                dim.qualifier = dim.name;
                dim.name = expect_ident(p, "a constant name");
            }
        } else {
            fail_expected(p, "an array size");
        }
        expect(p, TOK_RBRACKET);
        vec_push(te->dims, dim);
    }
    return te;
}

static TypeExpr *parse_return_type(Parser *p) {
    if (at(p, TOK_VOID)) {
        TypeExpr *te = NEW(TypeExpr);
        te->loc = next(p)->loc;
        te->is_void = true;
        return te;
    }
    return parse_type(p);
}

/* ---- Expressions ---- */

static Expr *new_expr(ExprKind kind, Loc loc) {
    Expr *e = NEW(Expr);
    e->kind = kind;
    e->loc = loc;
    return e;
}

static Expr *parse_expr(Parser *p);
static Stmt *parse_statement(Parser *p);
static Stmt *parse_block(Parser *p);

static Expr *parse_case_value(Parser *p) {
    Token *t = cur(p);
    switch (t->kind) {
    case TOK_IDENT: {
        Expr *e = new_expr(E_NAME, t->loc);
        e->name = next(p)->text;
        if (at(p, TOK_DOT)) fail_at(p, cur(p)->loc, "enum values in case labels are written without the enum name");
        return e;
    }
    case TOK_MINUS: {
        next(p);
        if (!at(p, TOK_INT)) fail_expected(p, "an integer literal");
        Expr *e = new_expr(E_INT, t->loc);
        e->int_value = next(p)->int_value;
        e->negative = true;
        return e;
    }
    case TOK_INT: {
        Expr *e = new_expr(E_INT, t->loc);
        e->int_value = next(p)->int_value;
        return e;
    }
    case TOK_CHAR: {
        Expr *e = new_expr(E_CHAR, t->loc);
        e->int_value = next(p)->int_value;
        return e;
    }
    case TOK_STRING: {
        Expr *e = new_expr(E_STRING, t->loc);
        e->str = t->text;
        e->str_len = t->text_len;
        next(p);
        return e;
    }
    case TOK_TRUE:
    case TOK_FALSE: {
        Expr *e = new_expr(E_BOOL, t->loc);
        e->bool_value = t->kind == TOK_TRUE;
        next(p);
        return e;
    }
    default:
        fail_expected(p, "a case value (enum value or literal)");
    }
}

/* Parses `switch (subject) { arms }`. Expression arms if is_expr, else statement arms. */
static SwitchData *parse_switch_body(Parser *p, bool is_expr) {
    SwitchData *sw = NEW(SwitchData);
    expect(p, TOK_LPAREN);
    sw->subject = parse_expr(p);
    expect(p, TOK_RPAREN);
    expect(p, TOK_LBRACE);
    while (!at(p, TOK_RBRACE)) {
        SwitchArm arm = {0};
        arm.loc = cur(p)->loc;
        if (accept(p, TOK_DEFAULT)) {
            arm.is_default = true;
        } else if (accept(p, TOK_CASE)) {
            do {
                CaseValue cv = {0};
                cv.loc = cur(p)->loc;
                cv.expr = parse_case_value(p);
                vec_push(arm.values, cv);
            } while (accept(p, TOK_COMMA));
        } else if (at(p, TOK_COLON)) {
            fail_at(p, cur(p)->loc, "switch arms use '->', not ':'");
        } else {
            fail_expected(p, "'case', 'default' or '}'");
        }
        if (at(p, TOK_COLON)) fail_at(p, cur(p)->loc, "switch arms use '->', not ':'");
        expect(p, TOK_ARROW);
        if (is_expr) {
            if (at(p, TOK_LBRACE)) fail_at(p, cur(p)->loc, "an arm of a switch expression must be a single expression");
            arm.expr = parse_expr(p);
            expect(p, TOK_SEMI);
        } else {
            arm.body = parse_statement(p);
        }
        vec_push(sw->arms, arm);
    }
    expect(p, TOK_RBRACE);
    return sw;
}

static Expr *parse_primary(Parser *p) {
    Token *t = cur(p);
    switch (t->kind) {
    case TOK_INT: {
        Expr *e = new_expr(E_INT, t->loc);
        e->int_value = next(p)->int_value;
        return e;
    }
    case TOK_FLOAT: {
        Expr *e = new_expr(E_FLOAT, t->loc);
        e->float_value = next(p)->float_value;
        return e;
    }
    case TOK_CHAR: {
        Expr *e = new_expr(E_CHAR, t->loc);
        e->int_value = next(p)->int_value;
        return e;
    }
    case TOK_STRING: {
        Expr *e = new_expr(E_STRING, t->loc);
        e->str = t->text;
        e->str_len = t->text_len;
        next(p);
        return e;
    }
    case TOK_TRUE:
    case TOK_FALSE: {
        Expr *e = new_expr(E_BOOL, t->loc);
        e->bool_value = t->kind == TOK_TRUE;
        next(p);
        return e;
    }
    case TOK_IDENT: {
        Expr *e = new_expr(E_NAME, t->loc);
        e->name = next(p)->text;
        return e;
    }
    case TOK_THIS:
        next(p);
        return new_expr(E_THIS, t->loc);
    case TOK_LPAREN: {
        next(p);
        Expr *e = parse_expr(p);
        expect(p, TOK_RPAREN);
        e->parenthesised = true;
        return e;
    }
    case TOK_SWITCH: {
        next(p);
        Expr *e = new_expr(E_SWITCH, t->loc);
        e->sw = parse_switch_body(p, true);
        return e;
    }
    case TOK_LBRACE:
        fail_at(p, t->loc, "an array initialiser '{...}' can only initialise a variable declaration");
    default:
        if (primitive_for(t->kind)) fail_at(p, t->loc, fmt_str("unexpected type name %s in expression", describe(t)));
        fail_expected(p, "an expression");
    }
}

static Expr *parse_postfix(Parser *p) {
    Expr *e = parse_primary(p);
    for (;;) {
        if (at(p, TOK_DOT)) {
            Loc loc = next(p)->loc;
            Expr *m = new_expr(E_MEMBER, loc);
            m->base = e;
            m->name = expect_ident(p, "a member name after '.'");
            e = m;
        } else if (at(p, TOK_LBRACKET)) {
            Loc loc = next(p)->loc;
            Expr *ix = new_expr(E_INDEX, loc);
            ix->base = e;
            ix->index = parse_expr(p);
            expect(p, TOK_RBRACKET);
            e = ix;
        } else if (at(p, TOK_LPAREN)) {
            Loc loc = next(p)->loc;
            Expr *call = new_expr(E_CALL, loc);
            call->base = e;
            if (!at(p, TOK_RPAREN)) {
                do {
                    vec_push(call->args, parse_expr(p));
                } while (accept(p, TOK_COMMA));
            }
            expect(p, TOK_RPAREN);
            e = call;
        } else if (at(p, TOK_PLUSPLUS) || at(p, TOK_MINUSMINUS)) {
            return e; /* handled by the statement parser */
        } else {
            return e;
        }
    }
}

static Expr *parse_unary(Parser *p) {
    Token *t = cur(p);
    if (t->kind == TOK_MINUS && (peek_tok(p, 1)->kind == TOK_INT || peek_tok(p, 1)->kind == TOK_FLOAT)) {
        /* Negative literal, so that e.g. -2147483648 fits int. */
        Token *lit = peek_tok(p, 1);
        TokenKind after = peek_tok(p, 2)->kind;
        if (after != TOK_DOT && after != TOK_LBRACKET && after != TOK_LPAREN) {
            next(p);
            next(p);
            Expr *e = new_expr(lit->kind == TOK_INT ? E_INT : E_FLOAT, t->loc);
            e->int_value = lit->int_value;
            e->float_value = -lit->float_value;
            e->negative = true;
            return e;
        }
    }
    if (t->kind == TOK_MINUS || t->kind == TOK_BANG || t->kind == TOK_TILDE) {
        next(p);
        Expr *e = new_expr(E_UNARY, t->loc);
        e->op = t->kind;
        e->base = parse_unary(p);
        return e;
    }
    if (t->kind == TOK_PLUSPLUS || t->kind == TOK_MINUSMINUS) {
        fail_at(p, t->loc, fmt_str("%s is a statement and can't be used inside an expression", token_kind_name(t->kind)));
    }
    if (t->kind == TOK_LPAREN && primitive_for(peek_tok(p, 1)->kind) && peek_tok(p, 2)->kind == TOK_RPAREN) {
        next(p);
        TypeExpr *te = parse_type(p);
        expect(p, TOK_RPAREN);
        Expr *e = new_expr(E_CAST, t->loc);
        e->cast_expr = te;
        e->cast_type = te->prim;
        e->base = parse_unary(p);
        return e;
    }
    return parse_postfix(p);
}

static int binary_precedence(TokenKind kind) {
    switch (kind) {
    case TOK_OROR: return 1;
    case TOK_ANDAND: return 2;
    case TOK_PIPE: return 3;
    case TOK_CARET: return 4;
    case TOK_AMP: return 5;
    case TOK_EQ:
    case TOK_NE: return 6;
    case TOK_LT:
    case TOK_LE:
    case TOK_GT:
    case TOK_GE: return 7;
    case TOK_SHL:
    case TOK_SHR: return 8;
    case TOK_PLUS:
    case TOK_MINUS: return 9;
    case TOK_STAR:
    case TOK_SLASH:
    case TOK_PERCENT: return 10;
    default: return 0;
    }
}

static Expr *parse_binary(Parser *p, int min_prec) {
    Expr *left = parse_unary(p);
    for (;;) {
        TokenKind op = cur(p)->kind;
        int prec = binary_precedence(op);
        if (prec == 0 || prec < min_prec) return left;
        Loc loc = next(p)->loc;
        Expr *right = parse_binary(p, prec + 1);
        Expr *e = new_expr(E_BINARY, loc);
        e->op = op;
        e->base = left;
        e->index = right;
        left = e;
    }
}

static Expr *parse_expr(Parser *p) {
    return parse_binary(p, 1);
}

static Expr *parse_initialiser(Parser *p) {
    if (at(p, TOK_LBRACE)) {
        Expr *e = new_expr(E_ARRAY_INIT, next(p)->loc);
        if (!at(p, TOK_RBRACE)) {
            do {
                if (at(p, TOK_RBRACE)) break;
                vec_push(e->args, parse_initialiser(p));
            } while (accept(p, TOK_COMMA));
        }
        expect(p, TOK_RBRACE);
        return e;
    }
    return parse_expr(p);
}

/* ---- Statements ---- */

static Stmt *new_stmt(StmtKind kind, Loc loc) {
    Stmt *s = NEW(Stmt);
    s->kind = kind;
    s->loc = loc;
    return s;
}

static bool is_assign_op(TokenKind kind) {
    return kind == TOK_ASSIGN || (kind >= TOK_PLUS_ASSIGN && kind <= TOK_SHR_ASSIGN);
}

static void check_no_nested_assignment(Parser *p) {
    if (is_assign_op(cur(p)->kind)) {
        fail_at(p, cur(p)->loc, "assignment is a statement and can't be used inside an expression");
    }
    if (at(p, TOK_PLUSPLUS) || at(p, TOK_MINUSMINUS)) {
        fail_at(p, cur(p)->loc,
                fmt_str("%s is a statement and can't be used inside an expression", token_kind_name(cur(p)->kind)));
    }
}

/* Parses a simple statement without the trailing ';'. */
static Stmt *parse_simple_statement(Parser *p) {
    Token *t = cur(p);
    if (t->kind == TOK_PLUSPLUS || t->kind == TOK_MINUSMINUS) {
        next(p);
        Stmt *s = new_stmt(S_INCDEC, t->loc);
        s->op = t->kind;
        s->expr = parse_unary(p);
        check_no_nested_assignment(p);
        return s;
    }
    Expr *e = parse_expr(p);
    if (is_assign_op(cur(p)->kind)) {
        Token *op = next(p);
        Stmt *s = new_stmt(S_ASSIGN, op->loc);
        s->op = op->kind;
        s->expr = e;
        s->value = parse_expr(p);
        check_no_nested_assignment(p);
        return s;
    }
    if (at(p, TOK_PLUSPLUS) || at(p, TOK_MINUSMINUS)) {
        Token *op = next(p);
        Stmt *s = new_stmt(S_INCDEC, op->loc);
        s->op = op->kind;
        s->expr = e;
        check_no_nested_assignment(p);
        return s;
    }
    Stmt *s = new_stmt(S_EXPR, e->loc);
    s->expr = e;
    return s;
}

static Var *new_var(VarKind kind, const char *name, Loc loc, TypeExpr *te) {
    Var *v = NEW(Var);
    v->kind = kind;
    v->name = name;
    v->loc = loc;
    v->type_expr = te;
    return v;
}

/* Parses `type name [= init]` without the ';'. */
static Stmt *parse_var_decl(Parser *p) {
    TypeExpr *te = parse_type(p);
    if (at(p, TOK_STAR)) {
        fail_at(p, cur(p)->loc, "references (T*) can only be parameter types, not local variables");
    }
    Token *name = expect(p, TOK_IDENT);
    Stmt *s = new_stmt(S_VAR, name->loc);
    s->var = new_var(VAR_LOCAL, name->text, name->loc, te);
    if (accept(p, TOK_ASSIGN)) {
        s->init = parse_initialiser(p);
        check_no_nested_assignment(p);
    }
    return s;
}

static Stmt *parse_block(Parser *p) {
    Stmt *s = new_stmt(S_BLOCK, cur(p)->loc);
    expect(p, TOK_LBRACE);
    while (!at(p, TOK_RBRACE)) {
        if (at(p, TOK_EOF)) fail_expected(p, "'}'");
        vec_push(s->stmts, parse_statement(p));
    }
    next(p);
    return s;
}

static Stmt *parse_statement(Parser *p) {
    Token *t = cur(p);
    switch (t->kind) {
    case TOK_LBRACE:
        return parse_block(p);
    case TOK_SEMI:
        fail_at(p, t->loc, "empty statement; remove the extra ';'");
    case TOK_IF: {
        next(p);
        Stmt *s = new_stmt(S_IF, t->loc);
        expect(p, TOK_LPAREN);
        s->expr = parse_expr(p);
        check_no_nested_assignment(p);
        expect(p, TOK_RPAREN);
        s->then_branch = parse_statement(p);
        if (accept(p, TOK_ELSE)) s->else_branch = parse_statement(p);
        return s;
    }
    case TOK_WHILE: {
        next(p);
        Stmt *s = new_stmt(S_WHILE, t->loc);
        expect(p, TOK_LPAREN);
        s->expr = parse_expr(p);
        check_no_nested_assignment(p);
        expect(p, TOK_RPAREN);
        s->body = parse_statement(p);
        return s;
    }
    case TOK_DO: {
        next(p);
        Stmt *s = new_stmt(S_DO, t->loc);
        s->body = parse_statement(p);
        expect(p, TOK_WHILE);
        expect(p, TOK_LPAREN);
        s->expr = parse_expr(p);
        check_no_nested_assignment(p);
        expect(p, TOK_RPAREN);
        expect(p, TOK_SEMI);
        return s;
    }
    case TOK_FOR: {
        next(p);
        Stmt *s = new_stmt(S_FOR, t->loc);
        expect(p, TOK_LPAREN);
        if (!at(p, TOK_SEMI)) {
            if (at(p, TOK_COLON) || (at_declaration(p) && peek_tok(p, scan_type(p, p->pos) - p->pos + 1)->kind == TOK_COLON)) {
                fail_at(p, cur(p)->loc, "for-each loops aren't supported; use an index loop");
            }
            s->for_init = at_declaration(p) ? parse_var_decl(p) : parse_simple_statement(p);
        }
        expect(p, TOK_SEMI);
        if (!at(p, TOK_SEMI)) {
            s->expr = parse_expr(p);
            check_no_nested_assignment(p);
        }
        expect(p, TOK_SEMI);
        if (!at(p, TOK_RPAREN)) s->for_update = parse_simple_statement(p);
        expect(p, TOK_RPAREN);
        s->body = parse_statement(p);
        return s;
    }
    case TOK_SWITCH: {
        next(p);
        Stmt *s = new_stmt(S_SWITCH, t->loc);
        s->sw = parse_switch_body(p, false);
        return s;
    }
    case TOK_BREAK:
        next(p);
        expect(p, TOK_SEMI);
        return new_stmt(S_BREAK, t->loc);
    case TOK_CONTINUE:
        next(p);
        expect(p, TOK_SEMI);
        return new_stmt(S_CONTINUE, t->loc);
    case TOK_RETURN: {
        next(p);
        Stmt *s = new_stmt(S_RETURN, t->loc);
        if (!at(p, TOK_SEMI)) {
            s->expr = parse_expr(p);
            check_no_nested_assignment(p);
        }
        expect(p, TOK_SEMI);
        return s;
    }
    case TOK_ALIAS: {
        next(p);
        Token *name = expect(p, TOK_IDENT);
        Stmt *s = new_stmt(S_ALIAS, name->loc);
        Alias *a = NEW(Alias);
        a->name = name->text;
        a->loc = name->loc;
        expect(p, TOK_ASSIGN);
        a->expr = parse_expr(p);
        check_no_nested_assignment(p);
        expect(p, TOK_SEMI);
        s->alias = a;
        return s;
    }
    case TOK_CONST:
        fail_at(p, t->loc, "constants can only be declared at the top level of a file or in a namespace");
    case TOK_ELSE:
        fail_at(p, t->loc, "'else' without a matching 'if'");
    case TOK_CASE:
    case TOK_DEFAULT:
        fail_at(p, t->loc, fmt_str("%s outside a switch", token_kind_name(t->kind)));
    case TOK_STRUCT:
    case TOK_INTERFACE:
    case TOK_ENUM:
    case TOK_NAMESPACE:
        fail_at(p, t->loc, fmt_str("%s can't be declared inside a function", token_kind_name(t->kind)));
    default:
        break;
    }
    if (at_declaration(p)) {
        int after = scan_type(p, p->pos);
        if (p->toks[after + 1].kind == TOK_LPAREN) {
            fail_at(p, p->toks[after].loc, "functions can't be declared inside functions");
        }
        Stmt *s = parse_var_decl(p);
        expect(p, TOK_SEMI);
        return s;
    }
    Stmt *s = parse_simple_statement(p);
    expect(p, TOK_SEMI);
    return s;
}

/* ---- Declarations ---- */

static Decl *new_decl(Parser *p, DeclKind kind, const char *name, Loc loc, Decl *ns) {
    Decl *d = NEW(Decl);
    d->kind = kind;
    d->name = name;
    d->loc = loc;
    d->file = p->file;
    d->ns = ns;
    return d;
}

static void parse_params(Parser *p, Decl *fn, bool allow_variadic) {
    expect(p, TOK_LPAREN);
    if (!at(p, TOK_RPAREN)) {
        do {
            if (at(p, TOK_ELLIPSIS)) {
                if (!allow_variadic) fail_at(p, cur(p)->loc, "only extern \"C\" functions can be variadic");
                next(p);
                fn->variadic = true;
                break;
            }
            TypeExpr *te = parse_type(p);
            bool is_ref = false;
            if (at(p, TOK_STAR)) {
                if (allow_variadic) fail_at(p, cur(p)->loc, "extern \"C\" functions can't take references");
                next(p);
                is_ref = true;
            }
            Token *name = expect(p, TOK_IDENT);
            Var *param = new_var(VAR_PARAM, name->text, name->loc, te);
            param->is_ref = is_ref;
            vec_push(fn->params, param);
        } while (accept(p, TOK_COMMA));
    }
    expect(p, TOK_RPAREN);
}

/* References (T*) exist only as parameter types (spec 7.3). */
static void reject_reference(Parser *p, const char *what) {
    if (at(p, TOK_STAR)) {
        fail_at(p, cur(p)->loc, fmt_str("references (T*) can only be parameter types, not %s", what));
    }
}

static Decl *parse_struct(Parser *p, Decl *ns) {
    next(p); /* struct */
    Token *name = expect(p, TOK_IDENT);
    Decl *d = new_decl(p, D_STRUCT, name->text, name->loc, ns);
    if (accept(p, TOK_COLON)) {
        do {
            ImplementsRef ref = {0};
            ref.loc = cur(p)->loc;
            ref.name = expect_ident(p, "an interface name");
            if (accept(p, TOK_DOT)) {
                ref.qualifier = ref.name;
                ref.name = expect_ident(p, "an interface name");
            }
            vec_push(d->implements, ref);
        } while (accept(p, TOK_COMMA));
    }
    expect(p, TOK_LBRACE);
    while (!at(p, TOK_RBRACE)) {
        if (at(p, TOK_EOF)) fail_expected(p, "'}'");
        Token *start = cur(p);
        bool is_private = accept(p, TOK_PRIVATE);
        if (at(p, TOK_STRUCT) || at(p, TOK_INTERFACE) || at(p, TOK_ENUM) || at(p, TOK_NAMESPACE) || at(p, TOK_CONST)) {
            fail_at(p, cur(p)->loc, fmt_str("%s can't be declared inside a struct", token_kind_name(cur(p)->kind)));
        }
        if (at(p, TOK_IDENT) && cur(p)->text == d->name && peek_tok(p, 1)->kind == TOK_LPAREN) {
            Token *ctor_name = next(p);
            if (is_private) fail_at(p, start->loc, "constructors can't be private");
            Decl *c = new_decl(p, D_CTOR, ctor_name->text, ctor_name->loc, ns);
            c->owner = d;
            parse_params(p, c, false);
            c->body = parse_block(p);
            vec_push(d->members, c);
            continue;
        }
        TypeExpr *te = parse_return_type(p);
        reject_reference(p, "fields and return values");
        Token *mname = expect(p, TOK_IDENT);
        if (at(p, TOK_LPAREN)) {
            Decl *m = new_decl(p, D_METHOD, mname->text, mname->loc, ns);
            m->owner = d;
            m->is_private = is_private;
            m->ret_expr = te;
            parse_params(p, m, false);
            m->body = parse_block(p);
            vec_push(d->members, m);
        } else {
            if (te->is_void) fail_at(p, te->loc, "a field can't have type void");
            if (at(p, TOK_ASSIGN)) {
                fail_at(p, cur(p)->loc, "fields can't have initialisers; set them in a constructor");
            }
            expect(p, TOK_SEMI);
            Decl *f = new_decl(p, D_FIELD, mname->text, mname->loc, ns);
            f->owner = d;
            f->is_private = is_private;
            f->field_type_expr = te;
            vec_push(d->members, f);
        }
    }
    next(p);
    return d;
}

static Decl *parse_interface(Parser *p, Decl *ns) {
    next(p); /* interface */
    Token *name = expect(p, TOK_IDENT);
    Decl *d = new_decl(p, D_INTERFACE, name->text, name->loc, ns);
    if (at(p, TOK_COLON)) fail_at(p, cur(p)->loc, "interfaces can't extend other interfaces");
    expect(p, TOK_LBRACE);
    while (!at(p, TOK_RBRACE)) {
        if (at(p, TOK_EOF)) fail_expected(p, "'}'");
        if (at(p, TOK_PRIVATE)) fail_at(p, cur(p)->loc, "interface methods can't be private");
        TypeExpr *te = parse_return_type(p);
        reject_reference(p, "return values");
        Token *mname = expect(p, TOK_IDENT);
        if (!at(p, TOK_LPAREN)) fail_at(p, mname->loc, "interfaces can only declare methods");
        Decl *m = new_decl(p, D_IFACE_METHOD, mname->text, mname->loc, ns);
        m->owner = d;
        m->ret_expr = te;
        parse_params(p, m, false);
        if (at(p, TOK_LBRACE)) fail_at(p, cur(p)->loc, "interface methods can't have a body");
        expect(p, TOK_SEMI);
        vec_push(d->members, m);
    }
    next(p);
    return d;
}

static Decl *parse_enum(Parser *p, Decl *ns) {
    next(p); /* enum */
    Token *name = expect(p, TOK_IDENT);
    Decl *d = new_decl(p, D_ENUM, name->text, name->loc, ns);
    expect(p, TOK_LBRACE);
    if (at(p, TOK_RBRACE)) fail_at(p, cur(p)->loc, "an enum must have at least one value");
    do {
        if (at(p, TOK_RBRACE)) break;
        Token *v = expect(p, TOK_IDENT);
        EnumValue ev = {v->text, v->loc};
        vec_push(d->values, ev);
    } while (accept(p, TOK_COMMA));
    if (at(p, TOK_SEMI)) fail_at(p, cur(p)->loc, "enums can only list values");
    expect(p, TOK_RBRACE);
    return d;
}

static Decl *parse_const(Parser *p, Decl *ns) {
    next(p); /* const */
    TypeExpr *te = parse_type(p);
    Token *name = expect(p, TOK_IDENT);
    Decl *d = new_decl(p, D_CONST, name->text, name->loc, ns);
    d->const_type_expr = te;
    expect(p, TOK_ASSIGN);
    d->const_value = parse_initialiser(p);
    expect(p, TOK_SEMI);
    return d;
}

static void parse_extern(Parser *p, Decl *ns, DeclVec *out) {
    Token *kw = next(p); /* extern */
    if (!at(p, TOK_STRING)) fail_expected(p, "\"C\" after 'extern'");
    Token *abi = next(p);
    if (strcmp(abi->text, "C") != 0) fail_at(p, abi->loc, "only extern \"C\" is supported");
    (void)kw;
    expect(p, TOK_LBRACE);
    while (!at(p, TOK_RBRACE)) {
        if (at(p, TOK_EOF)) fail_expected(p, "'}'");
        TypeExpr *te = parse_return_type(p);
        reject_reference(p, "return values");
        Token *name = expect(p, TOK_IDENT);
        Decl *d = new_decl(p, D_EXTERN_FUNC, name->text, name->loc, ns);
        d->ret_expr = te;
        d->extern_abi = "C";
        if (!at(p, TOK_LPAREN)) fail_at(p, name->loc, "extern \"C\" blocks can only declare functions");
        parse_params(p, d, true);
        if (at(p, TOK_LBRACE)) fail_at(p, cur(p)->loc, "extern functions can't have a body");
        expect(p, TOK_SEMI);
        vec_push(*out, d);
    }
    next(p);
}

static void parse_decl(Parser *p, Decl *ns, DeclVec *out) {
    Token *t = cur(p);
    switch (t->kind) {
    case TOK_NAMESPACE:
        fail_at(p, t->loc, "namespaces can't be nested");
    case TOK_STRUCT:
        vec_push(*out, parse_struct(p, ns));
        return;
    case TOK_INTERFACE:
        vec_push(*out, parse_interface(p, ns));
        return;
    case TOK_ENUM:
        vec_push(*out, parse_enum(p, ns));
        return;
    case TOK_CONST:
        vec_push(*out, parse_const(p, ns));
        return;
    case TOK_EXTERN:
        parse_extern(p, ns, out);
        return;
    case TOK_IMPORT:
        fail_at(p, t->loc, "imports must come before all declarations in a file");
    case TOK_PRIVATE:
        fail_at(p, t->loc, "'private' can only be used on struct members");
    case TOK_ALIAS:
        fail_at(p, t->loc, "aliases can only be declared inside functions");
    default:
        break;
    }
    if (!(t->kind == TOK_VOID || scan_type(p, p->pos) >= 0)) {
        fail_expected(p, "a declaration");
    }
    TypeExpr *te = parse_return_type(p);
    reject_reference(p, "return values and global variables");
    Token *name = expect(p, TOK_IDENT);
    if (at(p, TOK_ASSIGN) || at(p, TOK_SEMI)) {
        fail_at(p, name->loc, "global variables aren't supported; use a constant ('const') or a local variable");
    }
    Decl *fn = new_decl(p, D_FUNC, name->text, name->loc, ns);
    fn->ret_expr = te;
    parse_params(p, fn, false);
    if (at(p, TOK_SEMI)) fail_at(p, cur(p)->loc, "functions need a body");
    fn->body = parse_block(p);
    vec_push(*out, fn);
}

bool parse_file(SourceFile *file, TokenVec tokens) {
    Parser p = {0};
    p.file = file;
    p.toks = tokens.data;
    p.count = tokens.len;
    DeclVec decls = {0};
    VEC(ImportDecl) imports = {0};
    if (setjmp(p.bail)) {
        file->decls = decls.data;
        file->decl_count = decls.len;
        file->imports = imports.data;
        file->import_count = imports.len;
        return false;
    }
    while (at(&p, TOK_IMPORT)) {
        next(&p);
        Token *path = expect(&p, TOK_IMPORT_PATH);
        ImportDecl imp = {path->loc, path->text};
        vec_push(imports, imp);
        expect(&p, TOK_SEMI);
    }
    while (!at(&p, TOK_EOF)) {
        if (at(&p, TOK_NAMESPACE)) {
            Token *kw = next(&p);
            Token *name = expect(&p, TOK_IDENT);
            if (at(&p, TOK_DOT)) fail_at(&p, cur(&p)->loc, "a namespace name can't contain '.'");
            Decl *ns = new_decl(&p, D_NAMESPACE, name->text, name->loc, NULL);
            (void)kw;
            expect(&p, TOK_LBRACE);
            while (!at(&p, TOK_RBRACE)) {
                if (at(&p, TOK_EOF)) fail_expected(&p, "'}'");
                parse_decl(&p, ns, &ns->members);
            }
            next(&p);
            vec_push(decls, ns);
        } else {
            parse_decl(&p, NULL, &decls);
        }
    }
    file->decls = decls.data;
    file->decl_count = decls.len;
    file->imports = imports.data;
    file->import_count = imports.len;
    return true;
}
