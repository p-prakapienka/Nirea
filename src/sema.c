/* Name resolution and type checking (spec sections 5-12). */
#include "sema.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

typedef struct ScopeEntry {
    const char *name;
    Var *var;
    int depth;
} ScopeEntry;

typedef struct Ctx {
    Program *prog;
    SourceFile *file;
    Decl *ns;
    Decl *fn;
    Decl *self; /* struct whose method or constructor is being checked */
    Type *ret_type;
    VEC(ScopeEntry) scope;
    int depth;
    int loop_depth;
} Ctx;

static const char *name_print;
static const char *name_println;
static const char *name_copy;
static const char *name_equals;
static const char *name_length;
static const char *name_main;
static const char *name_string;

static int next_decl_id;

/* ---------------------------------------------------------------------- */
/* Helpers                                                                 */

static const char *decl_kind_name(Decl *d) {
    switch (d->kind) {
    case D_FUNC: return "function";
    case D_EXTERN_FUNC: return "extern function";
    case D_STRUCT: return "struct";
    case D_INTERFACE: return "interface";
    case D_ENUM: return "enum";
    case D_CONST: return "constant";
    case D_NAMESPACE: return "namespace";
    case D_FIELD: return "field";
    case D_METHOD: return "method";
    case D_CTOR: return "constructor";
    case D_IFACE_METHOD: return "interface method";
    }
    return "declaration";
}

static const char *qualified_name(Decl *d) {
    if (d->ns) return fmt_str("%s.%s", d->ns->name, d->name);
    return d->name;
}

static bool is_error(Type *t) {
    return !t || t->kind == TY_ERROR;
}

static bool file_sees(SourceFile *from, SourceFile *to) {
    if (from == to) return true;
    for (int i = 0; i < from->import_count; i++) {
        if (from->imported && from->imported[i] == to) return true;
    }
    return false;
}

/* Implicit conversions (spec 6.2). */
static bool implicit_ok(Type *from, Type *to) {
    if (from == to) return true;
    switch (from->kind) {
    case TY_I8:
        return to->kind == TY_I16 || to->kind == TY_I32 || to->kind == TY_I64 || to->kind == TY_F32 || to->kind == TY_F64;
    case TY_I16: return to->kind == TY_I32 || to->kind == TY_I64 || to->kind == TY_F32 || to->kind == TY_F64;
    case TY_I32: return to->kind == TY_I64 || to->kind == TY_F64;
    case TY_U8:
        return to->kind == TY_U16 || to->kind == TY_U32 || to->kind == TY_U64 || to->kind == TY_I16 ||
               to->kind == TY_I32 || to->kind == TY_I64 || to->kind == TY_F32 || to->kind == TY_F64;
    case TY_U16:
        return to->kind == TY_U32 || to->kind == TY_U64 || to->kind == TY_I32 || to->kind == TY_I64 ||
               to->kind == TY_F32 || to->kind == TY_F64;
    case TY_U32: return to->kind == TY_U64 || to->kind == TY_I64 || to->kind == TY_F64;
    case TY_F32: return to->kind == TY_F64;
    default: return false;
    }
}

static Expr *wrap_convert(Expr *e, Type *to) {
    Expr *c = NEW(Expr);
    c->kind = E_CONVERT;
    c->loc = e->loc;
    c->base = e;
    c->type = to;
    c->cast_type = to;
    return c;
}

/* Converts *pe to target or reports an error. */
static void coerce(Expr **pe, Type *target, const char *what) {
    Expr *e = *pe;
    if (is_error(e->type) || is_error(target)) return;
    if (e->type == target) return;
    if (implicit_ok(e->type, target)) {
        *pe = wrap_convert(e, target);
        return;
    }
    if (type_is_numeric(e->type) && type_is_numeric(target)) {
        error_at(e->loc, "%s: can't convert %s to %s implicitly; use a cast, e.g. (%s)", what, type_name(e->type),
                 type_name(target), type_name(target));
    } else {
        error_at(e->loc, "%s: expected %s, found %s", what, type_name(target), type_name(e->type));
    }
}

/* A literal, or arithmetic on literals only (computed by the parser). */
static bool is_untyped_literal(Expr *e) {
    return e->untyped_literal;
}

static bool is_int_literal_expr(Expr *e) {
    return e->int_literal;
}

static bool int_fits(uint64_t magnitude, bool negative, Type *t) {
    if (negative && magnitude == 0) negative = false;
    switch (t->kind) {
    case TY_I8: return negative ? magnitude <= 128 : magnitude <= 127;
    case TY_I16: return negative ? magnitude <= 32768 : magnitude <= 32767;
    case TY_I32: return negative ? magnitude <= 2147483648ULL : magnitude <= 2147483647ULL;
    case TY_I64: return negative ? magnitude <= 9223372036854775808ULL : magnitude <= 9223372036854775807ULL;
    case TY_U8: return !negative && magnitude <= 255;
    case TY_U16: return !negative && magnitude <= 65535;
    case TY_U32: return !negative && magnitude <= 4294967295ULL;
    case TY_U64: return !negative;
    case TY_F32: return magnitude <= (1ULL << 24) || (double)(float)magnitude == (double)magnitude;
    case TY_F64: {
        if (magnitude <= (1ULL << 53)) return true;
        double d = (double)magnitude;
        return d < 18446744073709551616.0 && (uint64_t)d == magnitude;
    }
    default: return false;
    }
}

static const char *int_literal_text(Expr *e) {
    return fmt_str("%s%llu", e->negative && e->int_value ? "-" : "", (unsigned long long)e->int_value);
}

/* ---------------------------------------------------------------------- */
/* Scopes                                                                  */

static void push_scope(Ctx *ctx) {
    ctx->depth++;
}

static void pop_scope(Ctx *ctx) {
    while (ctx->scope.len > 0 && ctx->scope.data[ctx->scope.len - 1].depth == ctx->depth) ctx->scope.len--;
    ctx->depth--;
}

static ScopeEntry *find_scope(Ctx *ctx, const char *name) {
    for (int i = ctx->scope.len - 1; i >= 0; i--) {
        if (ctx->scope.data[i].name == name) return &ctx->scope.data[i];
    }
    return NULL;
}

static bool declare_name(Ctx *ctx, const char *name, Loc loc, Var *var) {
    ScopeEntry *prev = find_scope(ctx, name);
    if (prev) {
        error_at(loc, "'%s' is already declared in this function", name);
        note_at(prev->var->loc, "'%s' was declared here", name);
        return false;
    }
    ScopeEntry e = {name, var, ctx->depth};
    vec_push(ctx->scope, e);
    return true;
}

static void add_local(Ctx *ctx, Var *v) {
    v->index = ctx->fn->locals.len;
    vec_push(ctx->fn->locals, v);
}

/* ---------------------------------------------------------------------- */
/* Declaration lookup                                                      */

static Decl *lookup_global(Ctx *ctx, const char *name, Loc loc, bool report) {
    Decl *d = map_get(&ctx->prog->globals, name);
    if (!d) return NULL;
    if (!file_sees(ctx->file, d->file)) {
        if (report) {
            error_at(loc, "'%s' is declared in %s, which isn't imported here", name, d->file->path);
            return (Decl *)-1;
        }
        return NULL;
    }
    return d;
}

/* Looks up an unqualified declaration name: own namespace first, then globals. */
static Decl *lookup_decl(Ctx *ctx, const char *name, Loc loc, bool report) {
    if (ctx->ns) {
        Decl *d = map_get(&ctx->ns->member_map, name);
        if (d) return d;
    }
    return lookup_global(ctx, name, loc, report);
}

static Decl *lookup_qualified(Ctx *ctx, const char *qualifier, const char *name, Loc loc) {
    Decl *ns = lookup_decl(ctx, qualifier, loc, true);
    if (ns == (Decl *)-1) return (Decl *)-1;
    if (!ns) {
        error_at(loc, "unknown namespace '%s'", qualifier);
        return (Decl *)-1;
    }
    if (ns->kind != D_NAMESPACE) {
        error_at(loc, "'%s' is a %s, not a namespace", qualifier, decl_kind_name(ns));
        return (Decl *)-1;
    }
    Decl *d = map_get(&ns->member_map, name);
    if (!d) {
        error_at(loc, "namespace '%s' has no member '%s'", qualifier, name);
        return (Decl *)-1;
    }
    return d;
}

/* ---------------------------------------------------------------------- */
/* Types                                                                   */

static bool const_int_value(Decl *c, int64_t *out);

/* Upper bound on the size of a type in bytes, saturating at UINT64_MAX. Every
 * field is counted as aligned to 8 bytes. */
static uint64_t type_size_bound(Type *t) {
    switch (t->kind) {
    case TY_BOOL:
    case TY_CHAR:
    case TY_I8:
    case TY_U8: return 1;
    case TY_I16:
    case TY_U16: return 2;
    case TY_I32:
    case TY_U32:
    case TY_F32:
    case TY_ENUM: return 4;
    case TY_I64:
    case TY_U64:
    case TY_F64: return 8;
    case TY_STRING:
    case TY_INTERFACE: return 16;
    case TY_ARRAY: {
        uint64_t elem = type_size_bound(t->elem);
        if (elem != 0 && (uint64_t)t->length > UINT64_MAX / elem) return UINT64_MAX;
        return elem * (uint64_t)t->length;
    }
    case TY_STRUCT: {
        uint64_t total = 8;
        Decl *s = t->decl;
        if (s->state == 1) return 8; /* cycle: reported separately */
        s->state = 1;
        for (int i = 0; i < s->members.len; i++) {
            Decl *m = s->members.data[i];
            if (m->kind != D_FIELD || !m->field_type) continue;
            uint64_t f = type_size_bound(m->field_type);
            f = f > UINT64_MAX - 7 ? UINT64_MAX : (f + 7) / 8 * 8;
            total = f > UINT64_MAX - total ? UINT64_MAX : total + f;
        }
        s->state = 0;
        return total;
    }
    default: return 0;
    }
}

#define MAX_TYPE_SIZE 0x7fffffffULL

static Type *resolve_type(Ctx *ctx, TypeExpr *te, bool allow_interface, const char *what) {
    if (te->is_void) {
        error_at(te->loc, "%s can't have type void", what);
        return type_error;
    }
    Type *t;
    if (te->prim) {
        t = te->prim;
    } else {
        Decl *d = te->qualifier ? lookup_qualified(ctx, te->qualifier, te->name, te->loc)
                                : lookup_decl(ctx, te->name, te->loc, true);
        if (d == (Decl *)-1) return type_error;
        if (!d) {
            error_at(te->loc, "unknown type '%s'", te->name);
            return type_error;
        }
        if (d->kind != D_STRUCT && d->kind != D_ENUM && d->kind != D_INTERFACE) {
            error_at(te->loc, "'%s' is a %s, not a type", qualified_name(d), decl_kind_name(d));
            return type_error;
        }
        t = d->type;
    }
    if (t->kind == TY_INTERFACE && (te->dims.len > 0 || !allow_interface)) {
        error_at(te->loc, "interface types can only be used as references, written %s*", type_name(t));
        return type_error;
    }
    for (int i = te->dims.len - 1; i >= 0; i--) {
        ArrayDim *dim = &te->dims.data[i];
        int64_t length = 0;
        if (dim->is_literal) {
            if (dim->value == 0 || dim->value > 0x7fffffffULL) {
                error_at(dim->loc, "array size must be between 1 and 2147483647");
                return type_error;
            }
            length = (int64_t)dim->value;
        } else {
            Decl *c = dim->qualifier ? lookup_qualified(ctx, dim->qualifier, dim->name, dim->loc)
                                     : lookup_decl(ctx, dim->name, dim->loc, true);
            if (c == (Decl *)-1) return type_error;
            if (!c || c->kind != D_CONST) {
                error_at(dim->loc, "array size must be an integer literal or integer constant");
                return type_error;
            }
            if (!const_int_value(c, &length)) {
                error_at(dim->loc, "array size constant '%s' must be an integer", c->name);
                return type_error;
            }
            if (length <= 0 || length > 0x7fffffffLL) {
                error_at(dim->loc, "array size must be between 1 and 2147483647");
                return type_error;
            }
        }
        t = type_array(t, length);
        if (type_size_bound(t) > MAX_TYPE_SIZE) {
            error_at(te->loc, "type %s is too large: values are limited to %llu bytes", type_name(t),
                     (unsigned long long)MAX_TYPE_SIZE);
            return type_error;
        }
    }
    return t;
}

/* ---------------------------------------------------------------------- */
/* Collection                                                              */

static void declare_member(Map *map, Decl *d, const char *container) {
    Decl *prev = map_get(map, d->name);
    if (prev) {
        error_at(d->loc, "'%s' is already declared in %s", d->name, container);
        note_at(prev->loc, "previous declaration of '%s'", d->name);
        return;
    }
    map_put(map, d->name, d);
}

static void collect_decl(Program *prog, Decl *d) {
    d->id = ++next_decl_id;
    switch (d->kind) {
    case D_STRUCT: {
        d->type = NEW(Type);
        d->type->kind = TY_STRUCT;
        d->type->decl = d;
        vec_push(prog->all_structs, d);
        const char *container = fmt_str("struct '%s'", d->name);
        int field_index = 0;
        for (int i = 0; i < d->members.len; i++) {
            Decl *m = d->members.data[i];
            m->id = ++next_decl_id;
            if (m->kind == D_CTOR) {
                if (d->ctor) {
                    error_at(m->loc, "struct '%s' already has a constructor; overloading isn't supported", d->name);
                    note_at(d->ctor->loc, "first constructor declared here");
                } else {
                    d->ctor = m;
                }
                vec_push(prog->all_functions, m);
                continue;
            }
            if ((m->name == name_copy || m->name == name_equals) && m->kind == D_METHOD) {
                error_at(m->loc, "'%s' is built in for every struct and can't be declared as a method", m->name);
                continue;
            }
            if (m->kind == D_FIELD) m->field_index = field_index++;
            if (m->kind == D_METHOD) vec_push(prog->all_functions, m);
            declare_member(&d->member_map, m, container);
        }
        break;
    }
    case D_INTERFACE: {
        d->type = NEW(Type);
        d->type->kind = TY_INTERFACE;
        d->type->decl = d;
        vec_push(prog->all_interfaces, d);
        const char *container = fmt_str("interface '%s'", d->name);
        for (int i = 0; i < d->members.len; i++) {
            Decl *m = d->members.data[i];
            m->id = ++next_decl_id;
            m->field_index = i;
            declare_member(&d->member_map, m, container);
        }
        break;
    }
    case D_ENUM: {
        d->type = NEW(Type);
        d->type->kind = TY_ENUM;
        d->type->decl = d;
        vec_push(prog->all_enums, d);
        for (int i = 0; i < d->values.len; i++) {
            for (int j = 0; j < i; j++) {
                if (d->values.data[j].name == d->values.data[i].name) {
                    error_at(d->values.data[i].loc, "duplicate enum value '%s'", d->values.data[i].name);
                    break;
                }
            }
        }
        break;
    }
    case D_FUNC: vec_push(prog->all_functions, d); break;
    case D_EXTERN_FUNC: vec_push(prog->all_externs, d); break;
    case D_CONST: vec_push(prog->all_consts, d); break;
    case D_NAMESPACE: {
        const char *container = fmt_str("namespace '%s'", d->name);
        for (int i = 0; i < d->members.len; i++) {
            Decl *m = d->members.data[i];
            collect_decl(prog, m);
            declare_member(&d->member_map, m, container);
        }
        break;
    }
    default: break;
    }
}

static void collect_program(Program *prog) {
    for (int f = 0; f < prog->files.len; f++) {
        SourceFile *file = prog->files.data[f];
        for (int i = 0; i < file->decl_count; i++) {
            Decl *d = file->decls[i];
            if (d->name == name_print || d->name == name_println) {
                error_at(d->loc, "'%s' is a built-in function and can't be redeclared", d->name);
            }
            Decl *prev = map_get(&prog->globals, d->name);
            if (prev) {
                if (prev->kind == D_NAMESPACE && d->kind == D_NAMESPACE) {
                    error_at(d->loc, "namespace '%s' is declared more than once", d->name);
                } else {
                    error_at(d->loc, "'%s' is already declared", d->name);
                }
                note_at(prev->loc, "previous declaration of '%s'", d->name);
                collect_decl(prog, d);
                continue;
            }
            map_put(&prog->globals, d->name, d);
            collect_decl(prog, d);
        }
    }
}

/* ---------------------------------------------------------------------- */
/* Constants                                                               */

static Type *check_expr(Ctx *ctx, Expr *e, Type *expected);

static bool const_int_value(Decl *c, int64_t *out) {
    if (!c->const_type || !type_is_integer(c->const_type)) return false;
    Expr *v = c->const_value;
    while (v && v->kind == E_CONVERT) v = v->base;
    if (!v || v->kind != E_INT) return false;
    if (v->negative) {
        if (v->int_value > 9223372036854775808ULL) return false;
        *out = v->int_value == 9223372036854775808ULL ? INT64_MIN : -(int64_t)v->int_value;
    } else {
        if (v->int_value > (uint64_t)INT64_MAX) return false;
        *out = (int64_t)v->int_value;
    }
    return true;
}

static void check_const(Program *prog, Decl *c) {
    Ctx ctx = {0};
    ctx.prog = prog;
    ctx.file = c->file;
    ctx.ns = c->ns;
    Type *t = resolve_type(&ctx, c->const_type_expr, false, "a constant");
    c->const_type = t;
    if (is_error(t)) return;
    if (t->kind == TY_STRUCT || t->kind == TY_ARRAY) {
        error_at(c->const_type_expr->loc, "constants must have a primitive, enum or String type");
        c->const_type = type_error;
        return;
    }
    Expr *v = c->const_value;
    bool syntax_ok = v->kind == E_INT || v->kind == E_FLOAT || v->kind == E_CHAR || v->kind == E_STRING ||
                     v->kind == E_BOOL || v->kind == E_MEMBER;
    if (syntax_ok) check_expr(&ctx, v, t);
    if (!syntax_ok || !(v->kind == E_ENUM_VALUE || v->kind == E_INT || v->kind == E_FLOAT || v->kind == E_CHAR ||
                        v->kind == E_STRING || v->kind == E_BOOL)) {
        error_at(v->loc, "a constant's value must be a literal or an enum value");
        c->const_type = type_error;
        return;
    }
    if (is_error(v->type)) {
        c->const_type = type_error;
        return;
    }
    coerce(&c->const_value, t, fmt_str("value of constant '%s'", c->name));
}

/* ---------------------------------------------------------------------- */
/* Signatures                                                              */

/* Names an extern "C" function can't have: C keywords, `main`, and the
 * prefixes the compiler uses for generated C (nr_, nr<digits>, nri<digits>). */
static bool reserved_c_name(const char *name) {
    static const char *const words[] = {
        "auto",     "break",    "case",     "char",   "const",    "continue", "default",  "do",
        "double",   "else",     "enum",     "extern", "float",    "for",      "goto",     "if",
        "inline",   "int",      "long",     "register", "restrict", "return", "short",    "signed",
        "sizeof",   "static",   "struct",   "switch", "typedef",  "union",    "unsigned", "void",
        "volatile", "while",    "_Alignas", "_Alignof", "_Atomic", "_Bool",   "_Complex", "_Generic",
        "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local", "bool", "true", "false", "main",
    };
    for (size_t i = 0; i < sizeof words / sizeof words[0]; i++) {
        if (strcmp(name, words[i]) == 0) return true;
    }
    if (strncmp(name, "nr_", 3) == 0) return true;
    if (strncmp(name, "nr", 2) == 0) {
        const char *rest = name + 2;
        if (*rest == 'i') rest++;
        if (*rest >= '0' && *rest <= '9') return true;
    }
    return false;
}

static void resolve_signature(Program *prog, Decl *fn) {
    Ctx ctx = {0};
    ctx.prog = prog;
    ctx.file = fn->file;
    ctx.ns = fn->ns;
    bool is_extern = fn->kind == D_EXTERN_FUNC;
    if (is_extern && reserved_c_name(fn->name)) {
        error_at(fn->loc, "'%s' can't be the name of an extern \"C\" function: it is reserved in C or by the compiler",
                 fn->name);
    }
    if (fn->kind == D_CTOR) {
        fn->ret_type = type_void;
    } else if (fn->ret_expr->is_void) {
        fn->ret_type = type_void;
    } else {
        fn->ret_type = resolve_type(&ctx, fn->ret_expr, fn->ret_is_ref, "a return value");
        if (fn->ret_type->kind == TY_INTERFACE && !fn->ret_is_ref) fn->ret_type = type_error;
        if (fn->ret_is_ref && !is_error(fn->ret_type) && !type_is_owned(fn->ret_type) &&
            fn->ret_type->kind != TY_INTERFACE) {
            error_at(fn->ret_expr->loc,
                     "only struct, array, String and interface types have references; return %s by value instead",
                     type_name(fn->ret_type));
            fn->ret_type = type_error;
        }
    }
    if (fn->ret_is_ref && fn->ret_expr->is_void) {
        error_at(fn->ret_expr->loc, "void can't be returned by reference; remove the '*'");
    }
    if (is_extern && !is_error(fn->ret_type) &&
        !(fn->ret_type == type_void || type_is_numeric(fn->ret_type) || fn->ret_type == type_bool ||
          fn->ret_type == type_char)) {
        error_at(fn->ret_expr->loc, "extern \"C\" functions can only return primitive types or void");
    }
    for (int i = 0; i < fn->params.len; i++) {
        Var *p = fn->params.data[i];
        for (int j = 0; j < i; j++) {
            if (fn->params.data[j]->name == p->name) {
                error_at(p->loc, "duplicate parameter name '%s'", p->name);
                break;
            }
        }
        p->type = resolve_type(&ctx, p->type_expr, !is_extern, "a parameter");
        if (!is_error(p->type)) {
            if (p->is_ref && !type_is_owned(p->type) && p->type->kind != TY_INTERFACE) {
                error_at(p->loc, "only struct, array, String and interface parameters can be references; %s is always passed by value",
                         type_name(p->type));
            } else if (!p->is_ref && p->type->kind == TY_INTERFACE) {
                error_at(p->loc, "interface parameters must be references: write %s* %s", type_name(p->type), p->name);
            }
        }
        if (is_extern && !is_error(p->type)) {
            if (p->type == type_string) {
                if (!p->is_ref) {
                    error_at(p->loc, "C functions receive strings by reference: write String* %s", p->name);
                }
            } else if (p->is_ref) {
                error_at(p->loc, "extern \"C\" functions can only take String references (String*)");
                p->type = type_error;
            } else if (!(type_is_numeric(p->type) || p->type == type_bool || p->type == type_char)) {
                error_at(p->type_expr->loc, "extern \"C\" function parameters must have a primitive type or String*");
                p->type = type_error;
            }
        }
    }
}

static void resolve_struct_fields(Program *prog, Decl *s) {
    Ctx ctx = {0};
    ctx.prog = prog;
    ctx.file = s->file;
    ctx.ns = s->ns;
    for (int i = 0; i < s->members.len; i++) {
        Decl *m = s->members.data[i];
        if (m->kind == D_FIELD) {
            m->field_type = resolve_type(&ctx, m->field_type_expr, false, "a field");
        }
    }
    for (int i = 0; i < s->implements.len; i++) {
        ImplementsRef *ref = &s->implements.data[i];
        Decl *d = ref->qualifier ? lookup_qualified(&ctx, ref->qualifier, ref->name, ref->loc)
                                 : lookup_decl(&ctx, ref->name, ref->loc, true);
        if (d == (Decl *)-1) continue;
        if (!d) {
            error_at(ref->loc, "unknown interface '%s'", ref->name);
            continue;
        }
        if (d->kind != D_INTERFACE) {
            error_at(ref->loc, "'%s' is a %s, not an interface; structs can only implement interfaces",
                     qualified_name(d), decl_kind_name(d));
            continue;
        }
        for (int j = 0; j < i; j++) {
            if (s->implements.data[j].decl == d) {
                error_at(ref->loc, "struct '%s' lists interface '%s' twice", s->name, d->name);
                d = NULL;
                break;
            }
        }
        ref->decl = d;
    }
}

/* Detects structs that contain themselves through fields or arrays. */
static bool struct_contains(Type *t, Decl *target) {
    while (t->kind == TY_ARRAY) t = t->elem;
    if (t->kind != TY_STRUCT) return false;
    Decl *s = t->decl;
    if (s == target) return true;
    if (s->state == 1) return false;
    s->state = 1;
    bool found = false;
    for (int i = 0; i < s->members.len && !found; i++) {
        Decl *m = s->members.data[i];
        if (m->kind == D_FIELD && m->field_type) found = struct_contains(m->field_type, target);
    }
    s->state = 0;
    return found;
}

static void check_struct_cycles(Program *prog) {
    for (int i = 0; i < prog->all_structs.len; i++) {
        Decl *s = prog->all_structs.data[i];
        bool cyclic = false;
        for (int j = 0; j < s->members.len; j++) {
            Decl *m = s->members.data[j];
            if (m->kind == D_FIELD && m->field_type && struct_contains(m->field_type, s)) cyclic = true;
        }
        if (!cyclic && type_size_bound(s->type) > MAX_TYPE_SIZE) {
            error_at(s->loc, "struct '%s' is too large: values are limited to %llu bytes", s->name,
                     (unsigned long long)MAX_TYPE_SIZE);
        }
        for (int j = 0; j < s->members.len; j++) {
            Decl *m = s->members.data[j];
            if (m->kind == D_FIELD && m->field_type && struct_contains(m->field_type, s)) {
                error_at(m->loc, "struct '%s' contains itself through field '%s'", s->name, m->name);
                m->field_type = type_error;
            }
        }
    }
}

static bool same_signature(Decl *a, Decl *b) {
    if (a->ret_type != b->ret_type || a->ret_is_ref != b->ret_is_ref || a->params.len != b->params.len) return false;
    for (int i = 0; i < a->params.len; i++) {
        if (a->params.data[i]->type != b->params.data[i]->type) return false;
        if (a->params.data[i]->is_ref != b->params.data[i]->is_ref) return false;
    }
    return true;
}

static void check_implementations(Program *prog) {
    for (int i = 0; i < prog->all_structs.len; i++) {
        Decl *s = prog->all_structs.data[i];
        for (int j = 0; j < s->implements.len; j++) {
            Decl *iface = s->implements.data[j].decl;
            if (!iface) continue;
            for (int k = 0; k < iface->members.len; k++) {
                Decl *im = iface->members.data[k];
                Decl *m = map_get(&s->member_map, im->name);
                if (!m || m->kind != D_METHOD) {
                    error_at(s->implements.data[j].loc, "struct '%s' doesn't implement method '%s' of interface '%s'",
                             s->name, im->name, iface->name);
                    continue;
                }
                if (m->is_private) {
                    error_at(m->loc, "method '%s' implements interface '%s' and must be public", m->name, iface->name);
                }
                if (!same_signature(m, im)) {
                    error_at(m->loc, "method '%s' doesn't match the signature declared in interface '%s'", m->name,
                             iface->name);
                    note_at(im->loc, "declared here");
                }
            }
        }
    }
}

static bool struct_implements(Decl *s, Decl *iface) {
    for (int i = 0; i < s->implements.len; i++) {
        if (s->implements.data[i].decl == iface) return true;
    }
    return false;
}

/* Wraps a struct value used as a reference to an interface it implements. */
static void wrap_to_iface(Expr **pe, Type *iface) {
    Expr *conv = NEW(Expr);
    conv->kind = E_TO_IFACE;
    conv->loc = (*pe)->loc;
    conv->base = *pe;
    conv->type = iface;
    conv->iface_impl_struct = (*pe)->type->decl;
    *pe = conv;
}

/* ---------------------------------------------------------------------- */
/* Expressions                                                             */

static Type *check_value(Ctx *ctx, Expr *e, Type *expected);
static void check_transfer_source(Ctx *ctx, Expr *e, bool is_argument);
static void check_switch(Ctx *ctx, SwitchData *sw, Loc loc, bool is_expr, Expr *e, Type *expected);

static Type *set_type(Expr *e, Type *t) {
    e->type = t;
    return t;
}

static void check_private(Ctx *ctx, Decl *member, Loc loc) {
    if (member->is_private && ctx->self != member->owner) {
        error_at(loc, "'%s' is private to struct '%s'", member->name, member->owner->name);
    }
}

/* Resolves a bare name. The result may be a value (E_VAR, E_FIELD, E_CONST)
 * or a reference to a declaration (E_NAMESPACE_REF, E_TYPE_REF,
 * E_FUNC_REF). */
static void resolve_name(Ctx *ctx, Expr *e) {
    if (e->name == name_string) {
        error_at(e->loc, "'String' is a type; the only function on it is String.of(value)");
        e->kind = E_FUNC_REF;
        e->decl = NULL;
        e->name = NULL;
        e->type = type_error;
        return;
    }
    ScopeEntry *se = find_scope(ctx, e->name);
    if (se) {
        e->kind = E_VAR;
        e->var = se->var;
        e->type = se->var->type;
        return;
    }
    if (ctx->self) {
        Decl *m = map_get(&ctx->self->member_map, e->name);
        if (m && m->kind == D_FIELD) {
            Expr *self = NEW(Expr);
            self->kind = E_THIS;
            self->loc = e->loc;
            self->type = ctx->self->type;
            e->kind = E_FIELD;
            e->base = self;
            e->decl = m;
            e->type = m->field_type;
            return;
        }
        if (m && m->kind == D_METHOD) {
            e->kind = E_FUNC_REF;
            e->decl = m;
            e->type = type_error;
            return;
        }
    }
    Decl *d = lookup_decl(ctx, e->name, e->loc, true);
    if (d == (Decl *)-1) {
        e->kind = E_FUNC_REF;
        e->decl = NULL;
        e->type = type_error;
        e->name = NULL;
        return;
    }
    if (!d) {
        if (e->name == name_print || e->name == name_println) {
            e->kind = E_FUNC_REF;
            e->decl = NULL;
            e->type = type_error;
            return;
        }
        error_at(e->loc, "unknown name '%s'", e->name);
        e->kind = E_FUNC_REF;
        e->decl = NULL;
        e->name = NULL;
        e->type = type_error;
        return;
    }
    e->decl = d;
    switch (d->kind) {
    case D_CONST:
        e->kind = E_CONST;
        e->type = d->const_type ? d->const_type : type_error;
        return;
    case D_NAMESPACE: e->kind = E_NAMESPACE_REF; break;
    case D_STRUCT:
    case D_ENUM:
    case D_INTERFACE: e->kind = E_TYPE_REF; break;
    default: e->kind = E_FUNC_REF; break;
    }
    e->type = type_error;
}

static bool is_decl_ref(Expr *e) {
    return e->kind == E_NAMESPACE_REF || e->kind == E_TYPE_REF || e->kind == E_FUNC_REF;
}

static void member_of_decl(Ctx *ctx, Expr *e, Decl *container) {
    (void)ctx;
    if (container->kind == D_NAMESPACE) {
        Decl *d = map_get(&container->member_map, e->name);
        if (!d) {
            error_at(e->loc, "namespace '%s' has no member '%s'", container->name, e->name);
            e->kind = E_FUNC_REF;
            e->decl = NULL;
            e->name = NULL;
            e->type = type_error;
            return;
        }
        e->decl = d;
        e->type = type_error;
        switch (d->kind) {
        case D_CONST:
            e->kind = E_CONST;
            e->type = d->const_type ? d->const_type : type_error;
            return;
        case D_STRUCT:
        case D_ENUM:
        case D_INTERFACE: e->kind = E_TYPE_REF; return;
        default: e->kind = E_FUNC_REF; return;
        }
    }
    if (container->kind == D_ENUM) {
        for (int i = 0; i < container->values.len; i++) {
            if (container->values.data[i].name == e->name) {
                e->kind = E_ENUM_VALUE;
                e->decl = container;
                e->enum_index = i;
                e->type = container->type;
                return;
            }
        }
        error_at(e->loc, "enum '%s' has no value '%s'", qualified_name(container), e->name);
    } else {
        error_at(e->loc, "'%s' is a %s; use a value of that type to access '%s'", qualified_name(container),
                 decl_kind_name(container), e->name);
    }
    e->kind = E_FUNC_REF;
    e->decl = NULL;
    e->name = NULL;
    e->type = type_error;
}

/* Resolves an E_NAME or E_MEMBER chain without requiring a value. */
static void resolve_path(Ctx *ctx, Expr *e) {
    if (e->kind == E_NAME) {
        resolve_name(ctx, e);
        return;
    }
    if (e->kind != E_MEMBER) {
        check_expr(ctx, e, NULL);
        return;
    }
    Expr *base = e->base;
    if (base->kind == E_NAME || base->kind == E_MEMBER) {
        resolve_path(ctx, base);
    } else {
        check_expr(ctx, base, NULL);
    }
    if (is_decl_ref(base)) {
        if (!base->decl) {
            e->kind = E_FUNC_REF;
            e->decl = NULL;
            e->name = NULL;
            e->type = type_error;
            return;
        }
        if (base->kind == E_FUNC_REF) {
            error_at(e->loc, "'%s' is a %s and has no members", base->decl->name, decl_kind_name(base->decl));
            e->kind = E_FUNC_REF;
            e->decl = NULL;
            e->name = NULL;
            e->type = type_error;
            return;
        }
        member_of_decl(ctx, e, base->decl);
        return;
    }
    Type *bt = base->type;
    if (is_error(bt)) {
        e->type = type_error;
        e->kind = E_FIELD;
        return;
    }
    if (bt->kind == TY_STRUCT) {
        Decl *m = map_get(&bt->decl->member_map, e->name);
        if (m && m->kind == D_FIELD) {
            check_private(ctx, m, e->loc);
            e->kind = E_FIELD;
            e->decl = m;
            e->type = m->field_type;
            return;
        }
        if (m && m->kind == D_METHOD) {
            error_at(e->loc, "method '%s' must be called: %s()", e->name, e->name);
        } else if (e->name == name_copy) {
            error_at(e->loc, "'copy' is a method and must be called: copy()");
        } else {
            error_at(e->loc, "struct '%s' has no field '%s'", type_name(bt), e->name);
        }
        e->kind = E_FIELD;
        e->type = type_error;
        return;
    }
    if (bt->kind == TY_ARRAY && e->name == name_length) {
        e->kind = E_LENGTH;
        e->type = type_i32;
        return;
    }
    if (bt->kind == TY_INTERFACE) {
        error_at(e->loc, "interface '%s' has no fields; call one of its methods", type_name(bt));
    } else {
        error_at(e->loc, "a value of type %s has no member '%s'", type_name(bt), e->name);
    }
    e->kind = E_FIELD;
    e->type = type_error;
}

static void check_args(Ctx *ctx, Expr *call, Decl *fn, const char *fn_desc) {
    int nparams = fn->params.len;
    int nargs = call->args.len;
    if (nargs < nparams || (nargs > nparams && !fn->variadic)) {
        error_at(call->loc, "%s expects %d argument%s, but %d %s given", fn_desc, nparams, nparams == 1 ? "" : "s",
                 nargs, nargs == 1 ? "was" : "were");
    }
    for (int i = 0; i < nargs; i++) {
        Expr *arg = call->args.data[i];
        if (i >= nparams) {
            if (!fn->variadic) {
                check_value(ctx, arg, NULL);
                continue;
            }
            Type *t = check_value(ctx, arg, NULL);
            if (!is_error(t) && !(type_is_numeric(t) || t == type_bool || t == type_char || t == type_string)) {
                error_at(arg->loc, "variadic arguments must be primitive values or strings, found %s", type_name(t));
            }
            continue;
        }
        Type *pt = fn->params.data[i]->type;
        if (is_error(pt)) {
            check_value(ctx, arg, NULL);
            continue;
        }
        const char *what = fmt_str("argument %d of %s", i + 1, fn_desc);
        if (pt->kind == TY_INTERFACE) {
            Type *at = check_value(ctx, arg, NULL);
            if (is_error(at) || at == pt) continue;
            if (at->kind == TY_STRUCT && struct_implements(at->decl, pt->decl)) {
                wrap_to_iface(&call->args.data[i], pt);
            } else if (at->kind == TY_STRUCT) {
                error_at(arg->loc, "%s: struct '%s' doesn't implement interface '%s'", what, type_name(at),
                         type_name(pt));
            } else {
                error_at(arg->loc, "%s: expected a struct implementing interface '%s', found %s", what, type_name(pt),
                         type_name(at));
            }
            continue;
        }
        if (type_is_owned(pt)) {
            Type *at = check_value(ctx, arg, pt);
            if (!is_error(at) && at != pt) {
                error_at(arg->loc, "%s: expected %s, found %s", what, type_name(pt), type_name(at));
            } else if (!fn->params.data[i]->is_ref) {
                check_transfer_source(ctx, arg, true); /* value parameter: ownership is transferred */
            }
            continue;
        }
        check_value(ctx, arg, pt);
        coerce(&call->args.data[i], pt, what);
    }
}

static const char *callable_desc(Decl *d) {
    switch (d->kind) {
    case D_CTOR: return fmt_str("constructor of '%s'", qualified_name(d->owner));
    case D_METHOD:
    case D_IFACE_METHOD: return fmt_str("method '%s'", d->name);
    default: return fmt_str("function '%s'", qualified_name(d));
    }
}

static Type *check_print(Ctx *ctx, Expr *e, const char *name) {
    e->kind = E_CALL_PRINT;
    e->name = name;
    bool is_println = name == name_println;
    if (e->args.len > 1 || (!is_println && e->args.len != 1)) {
        error_at(e->loc, "'%s' expects %s argument", name, is_println ? "at most one" : "one");
    }
    for (int i = 0; i < e->args.len; i++) {
        Type *t = check_value(ctx, e->args.data[i], NULL);
        if (!is_error(t) && !(type_is_numeric(t) || t == type_bool || t == type_char || t == type_string ||
                              t->kind == TY_ENUM)) {
            error_at(e->args.data[i]->loc, "can't print a value of type %s", type_name(t));
        }
    }
    return set_type(e, type_void);
}

static Type *call_struct_ctor(Ctx *ctx, Expr *e, Decl *s) {
    e->kind = E_CALL_CTOR;
    e->decl = s;
    if (s->ctor) {
        check_args(ctx, e, s->ctor, callable_desc(s->ctor));
    } else if (e->args.len > 0) {
        error_at(e->loc, "struct '%s' has no constructor; '%s()' takes no arguments and gives a zero value",
                 qualified_name(s), s->name);
        for (int i = 0; i < e->args.len; i++) check_value(ctx, e->args.data[i], NULL);
    }
    return set_type(e, s->type);
}

static Type *call_decl(Ctx *ctx, Expr *e, Decl *d) {
    switch (d->kind) {
    case D_FUNC:
    case D_EXTERN_FUNC:
        e->kind = E_CALL_FUNC;
        e->decl = d;
        check_args(ctx, e, d, callable_desc(d));
        return set_type(e, d->ret_type);
    case D_STRUCT: return call_struct_ctor(ctx, e, d);
    default:
        error_at(e->loc, "'%s' is a %s and can't be called", qualified_name(d), decl_kind_name(d));
        for (int i = 0; i < e->args.len; i++) check_value(ctx, e->args.data[i], NULL);
        return set_type(e, type_error);
    }
}

/* Built-in String methods (spec 6.5). Parameter kinds: 'i' int, 'c' char,
 * 's' String. All parameters are values. */
typedef struct StringMethod {
    const char *name;
    const char *params;
    Type **result;
} StringMethod;

static const StringMethod string_methods[] = {
    {"length", "", &type_i32},       {"charAt", "i", &type_char},       {"setCharAt", "ic", &type_void},
    {"append", "s", &type_void},     {"appendChar", "c", &type_void},   {"substring", "ii", &type_string},
    {"indexOf", "s", &type_i32},     {"toInt", "", &type_i32},          {"toInt64", "", &type_i64},
    {"toFloat64", "", &type_f64},
};

static void check_string_args(Ctx *ctx, Expr *e, const char *params, const char *desc) {
    int n = (int)strlen(params);
    if (e->args.len != n) {
        error_at(e->loc, "%s expects %d argument%s, but %d %s given", desc, n, n == 1 ? "" : "s", e->args.len,
                 e->args.len == 1 ? "was" : "were");
    }
    for (int i = 0; i < e->args.len; i++) {
        if (i >= n) {
            check_value(ctx, e->args.data[i], NULL);
            continue;
        }
        const char *what = fmt_str("argument %d of %s", i + 1, desc);
        Type *pt = params[i] == 'i' ? type_i32 : params[i] == 'c' ? type_char : type_string;
        Type *at = check_value(ctx, e->args.data[i], pt);
        if (params[i] == 's') {
            if (!is_error(at) && at != type_string) error_at(e->args.data[i]->loc, "%s: expected String, found %s", what, type_name(at));
        } else {
            coerce(&e->args.data[i], pt, what);
        }
    }
}

static Type *check_string_method(Ctx *ctx, Expr *e, Expr *receiver, const char *name, Loc loc) {
    for (size_t i = 0; i < sizeof string_methods / sizeof string_methods[0]; i++) {
        if (strcmp(string_methods[i].name, name) != 0) continue;
        e->kind = E_CALL_STRING;
        e->base = receiver;
        e->name = name;
        check_string_args(ctx, e, string_methods[i].params, fmt_str("String method '%s'", name));
        return set_type(e, *string_methods[i].result);
    }
    error_at(loc, "String has no method '%s'", name);
    for (int i = 0; i < e->args.len; i++) check_value(ctx, e->args.data[i], NULL);
    return set_type(e, type_error);
}

/* String.of(value): text of a primitive or enum value (spec 6.5). */
static Type *check_string_of(Ctx *ctx, Expr *e, const char *name, Loc loc) {
    if (strcmp(name, "of") != 0) {
        error_at(loc, "String has no function '%s'; did you mean String.of?", name);
        for (int i = 0; i < e->args.len; i++) check_value(ctx, e->args.data[i], NULL);
        return set_type(e, type_error);
    }
    e->kind = E_CALL_STRING;
    e->base = NULL;
    e->name = intern_cstr("of");
    if (e->args.len != 1) error_at(e->loc, "String.of expects one argument");
    for (int i = 0; i < e->args.len; i++) {
        Type *t = check_value(ctx, e->args.data[i], NULL);
        if (!is_error(t) && !(type_is_numeric(t) || t == type_bool || t == type_char || t->kind == TY_ENUM)) {
            error_at(e->args.data[i]->loc, "String.of needs a primitive or enum value, found %s%s", type_name(t),
                     t == type_string ? "; use copy() to copy a String" : "");
        }
    }
    return set_type(e, type_string);
}

static Type *call_method_on(Ctx *ctx, Expr *e, Expr *receiver, const char *name, Loc loc) {
    Type *rt = receiver->type;
    if (is_error(rt)) {
        for (int i = 0; i < e->args.len; i++) check_value(ctx, e->args.data[i], NULL);
        return set_type(e, type_error);
    }
    if (name == name_copy && type_is_owned(rt)) {
        e->kind = E_COPY;
        e->base = receiver;
        if (e->args.len) error_at(e->loc, "copy() takes no arguments");
        return set_type(e, rt);
    }
    if (name == name_equals && type_is_owned(rt)) {
        /* compares the values both sides refer to, in place (spec 6.9) */
        e->kind = E_EQUALS;
        e->base = receiver;
        if (e->args.len != 1) error_at(e->loc, "equals() expects one argument");
        for (int i = 0; i < e->args.len; i++) {
            Type *at = check_value(ctx, e->args.data[i], rt);
            if (i == 0 && !is_error(at) && at != rt) {
                error_at(e->args.data[i]->loc, "equals() compares two values of the same type: expected %s, found %s",
                         type_name(rt), type_name(at));
            }
        }
        return set_type(e, type_bool);
    }
    if (rt->kind == TY_STRING) return check_string_method(ctx, e, receiver, name, loc);
    if (rt->kind == TY_STRUCT) {
        Decl *m = map_get(&rt->decl->member_map, name);
        if (!m || m->kind != D_METHOD) {
            if (m && m->kind == D_FIELD) {
                error_at(loc, "'%s' is a field of struct '%s', not a method", name, type_name(rt));
            } else {
                error_at(loc, "struct '%s' has no method '%s'", type_name(rt), name);
            }
            for (int i = 0; i < e->args.len; i++) check_value(ctx, e->args.data[i], NULL);
            return set_type(e, type_error);
        }
        check_private(ctx, m, loc);
        e->kind = E_CALL_METHOD;
        e->base = receiver;
        e->decl = m;
        check_args(ctx, e, m, callable_desc(m));
        return set_type(e, m->ret_type);
    }
    if (rt->kind == TY_INTERFACE) {
        Decl *m = map_get(&rt->decl->member_map, name);
        if (!m) {
            error_at(loc, "interface '%s' has no method '%s'", type_name(rt), name);
            for (int i = 0; i < e->args.len; i++) check_value(ctx, e->args.data[i], NULL);
            return set_type(e, type_error);
        }
        e->kind = E_CALL_IFACE;
        e->base = receiver;
        e->decl = m;
        e->iface_method_index = m->field_index;
        check_args(ctx, e, m, callable_desc(m));
        return set_type(e, m->ret_type);
    }
    error_at(loc, "a value of type %s has no method '%s'", type_name(rt), name);
    for (int i = 0; i < e->args.len; i++) check_value(ctx, e->args.data[i], NULL);
    return set_type(e, type_error);
}

static Type *check_call(Ctx *ctx, Expr *e) {
    Expr *callee = e->base;
    if (callee->kind == E_NAME) {
        const char *name = callee->name;
        ScopeEntry *se = find_scope(ctx, name);
        if (se) {
            error_at(callee->loc, "'%s' is a variable, not a function", name);
            for (int i = 0; i < e->args.len; i++) check_value(ctx, e->args.data[i], NULL);
            return set_type(e, type_error);
        }
        if (ctx->self) {
            Decl *m = map_get(&ctx->self->member_map, name);
            if (m && m->kind == D_METHOD) {
                Expr *self = NEW(Expr);
                self->kind = E_THIS;
                self->loc = callee->loc;
                self->type = ctx->self->type;
                return call_method_on(ctx, e, self, name, callee->loc);
            }
            if (m && m->kind == D_FIELD) {
                error_at(callee->loc, "'%s' is a field, not a method", name);
                for (int i = 0; i < e->args.len; i++) check_value(ctx, e->args.data[i], NULL);
                return set_type(e, type_error);
            }
        }
        if (name == name_print || name == name_println) {
            Decl *shadow = lookup_decl(ctx, name, callee->loc, false);
            if (!shadow) return check_print(ctx, e, name);
        }
        Decl *d = lookup_decl(ctx, name, callee->loc, true);
        if (d == (Decl *)-1 || !d) {
            if (!d) error_at(callee->loc, "unknown function '%s'", name);
            for (int i = 0; i < e->args.len; i++) check_value(ctx, e->args.data[i], NULL);
            return set_type(e, type_error);
        }
        return call_decl(ctx, e, d);
    }
    if (callee->kind == E_MEMBER) {
        Expr *base = callee->base;
        if (base->kind == E_NAME && base->name == name_string) return check_string_of(ctx, e, callee->name, callee->loc);
        if (base->kind == E_NAME || base->kind == E_MEMBER) {
            resolve_path(ctx, base);
        } else {
            check_expr(ctx, base, NULL);
        }
        if (is_decl_ref(base)) {
            if (!base->decl) {
                for (int i = 0; i < e->args.len; i++) check_value(ctx, e->args.data[i], NULL);
                return set_type(e, type_error);
            }
            if (base->decl->kind == D_NAMESPACE) {
                Decl *d = map_get(&base->decl->member_map, callee->name);
                if (!d) {
                    error_at(callee->loc, "namespace '%s' has no member '%s'", base->decl->name, callee->name);
                    for (int i = 0; i < e->args.len; i++) check_value(ctx, e->args.data[i], NULL);
                    return set_type(e, type_error);
                }
                return call_decl(ctx, e, d);
            }
            error_at(callee->loc, "'%s' is a %s; methods are called on values", qualified_name(base->decl),
                     decl_kind_name(base->decl));
            for (int i = 0; i < e->args.len; i++) check_value(ctx, e->args.data[i], NULL);
            return set_type(e, type_error);
        }
        if (base->type == type_void) {
            error_at(base->loc, "this expression has no value");
            base->type = type_error;
        }
        return call_method_on(ctx, e, base, callee->name, callee->loc);
    }
    check_expr(ctx, callee, NULL);
    error_at(e->loc, "this expression can't be called");
    for (int i = 0; i < e->args.len; i++) check_value(ctx, e->args.data[i], NULL);
    return set_type(e, type_error);
}

static Type *common_numeric(Expr *e, Type *lt, Type *rt, const char *opname) {
    if (is_error(lt) || is_error(rt)) return type_error;
    if (lt == rt) return lt;
    if (implicit_ok(lt, rt)) {
        e->base = wrap_convert(e->base, rt);
        return rt;
    }
    if (implicit_ok(rt, lt)) {
        e->index = wrap_convert(e->index, lt);
        return lt;
    }
    error_at(e->loc, "operands of '%s' have different types, %s and %s; use a cast", opname, type_name(lt),
             type_name(rt));
    return type_error;
}

/* Checks both operands of a binary operator, propagating literal types. */
static void check_operands(Ctx *ctx, Expr *e, Type *hint) {
    bool lu = is_untyped_literal(e->base);
    bool ru = is_untyped_literal(e->index);
    if (hint && !type_is_numeric(hint)) hint = NULL;
    if (lu && ru) {
        check_value(ctx, e->base, hint);
        check_value(ctx, e->index, hint ? hint : NULL);
        if (!hint && e->base->type != e->index->type) {
            /* e.g. 1 + 2.5: retype the integer literal as the float type */
            if (type_is_float(e->index->type) && type_is_integer(e->base->type)) check_value(ctx, e->base, e->index->type);
            else if (type_is_float(e->base->type) && type_is_integer(e->index->type))
                check_value(ctx, e->index, e->base->type);
        }
    } else if (lu) {
        Type *rt = check_value(ctx, e->index, hint);
        check_value(ctx, e->base, type_is_numeric(rt) || rt == type_char ? rt : hint);
    } else {
        Type *lt = check_value(ctx, e->base, hint);
        check_value(ctx, e->index, ru && (type_is_numeric(lt) || lt == type_char) ? lt : hint);
    }
}

/* A reference is a T* parameter or variable, `this`, or the result of a call
 * that returns a reference (spec 9). */
static bool is_reference(Expr *e) {
    switch (e->kind) {
    case E_VAR: return e->var->is_ref;
    case E_THIS: return true;
    default: return is_ref_call(e);
    }
}

static const char *reference_name(Expr *e) {
    switch (e->kind) {
    case E_THIS: return "this";
    case E_VAR: return fmt_str("'%s'", e->var->name);
    default: return fmt_str("the result of '%s'", e->decl->name);
    }
}

static Type *check_binary(Ctx *ctx, Expr *e, Type *expected) {
    int op = e->op;
    const char *opname = token_kind_name(op);
    opname = fmt_str("%.*s", (int)strlen(opname) - 2, opname + 1);
    switch (op) {
    case TOK_ANDAND:
    case TOK_OROR: {
        check_value(ctx, e->base, type_bool);
        coerce(&e->base, type_bool, fmt_str("left operand of '%s'", opname));
        check_value(ctx, e->index, type_bool);
        coerce(&e->index, type_bool, fmt_str("right operand of '%s'", opname));
        return set_type(e, type_bool);
    }
    case TOK_EQ:
    case TOK_NE: {
        check_operands(ctx, e, NULL);
        Type *lt = e->base->type, *rt = e->index->type;
        if (is_error(lt) || is_error(rt)) return set_type(e, type_bool);
        for (int side = 0; side < 2; side++) {
            Expr *operand = side ? e->index : e->base;
            if (is_reference(operand)) {
                error_at(operand->loc,
                         "%s is a reference, and references can't be compared with '%s'; use 'a.equals(b)' to compare "
                         "the values they refer to",
                         reference_name(operand), opname);
                return set_type(e, type_bool);
            }
        }
        if (type_is_numeric(lt) && type_is_numeric(rt)) {
            common_numeric(e, lt, rt, opname);
            return set_type(e, type_bool);
        }
        if (lt != rt) {
            error_at(e->loc, "can't compare %s with %s", type_name(lt), type_name(rt));
        } else if (lt->kind == TY_INTERFACE) {
            error_at(e->loc, "interface values can't be compared with '%s'; compare fields of the objects instead", opname);
        }
        return set_type(e, type_bool);
    }
    case TOK_LT:
    case TOK_LE:
    case TOK_GT:
    case TOK_GE: {
        check_operands(ctx, e, NULL);
        Type *lt = e->base->type, *rt = e->index->type;
        if (is_error(lt) || is_error(rt)) return set_type(e, type_bool);
        if (lt == type_char && rt == type_char) return set_type(e, type_bool);
        if (!type_is_numeric(lt) || !type_is_numeric(rt)) {
            error_at(e->loc, "'%s' needs numeric or char operands, found %s and %s", opname, type_name(lt),
                     type_name(rt));
            return set_type(e, type_bool);
        }
        common_numeric(e, lt, rt, opname);
        return set_type(e, type_bool);
    }
    case TOK_SHL:
    case TOK_SHR: {
        Type *hint = expected && type_is_integer(expected) ? expected : NULL;
        Type *lt = check_value(ctx, e->base, hint);
        Type *rt = check_value(ctx, e->index, NULL);
        if (!is_error(lt) && !type_is_integer(lt)) {
            error_at(e->base->loc, "'%s' needs an integer to shift, found %s", opname, type_name(lt));
            lt = type_error;
        }
        if (!is_error(rt) && !type_is_integer(rt)) {
            error_at(e->index->loc, "the shift amount must be an integer, found %s", type_name(rt));
        }
        return set_type(e, lt);
    }
    case TOK_AMP:
    case TOK_PIPE:
    case TOK_CARET: {
        check_operands(ctx, e, expected);
        Type *lt = e->base->type, *rt = e->index->type;
        if (is_error(lt) || is_error(rt)) return set_type(e, type_error);
        if (!type_is_integer(lt) || !type_is_integer(rt)) {
            error_at(e->loc, "'%s' needs integer operands, found %s and %s", opname, type_name(lt), type_name(rt));
            return set_type(e, type_error);
        }
        return set_type(e, common_numeric(e, lt, rt, opname));
    }
    default: {
        /* `1 / 2` divides integers even where a float is expected, as in C
         * and Java; only the result is converted. */
        Type *hint = expected;
        if ((op == TOK_SLASH || op == TOK_PERCENT) && hint && type_is_float(hint) && is_int_literal_expr(e->base) &&
            is_int_literal_expr(e->index)) {
            hint = NULL;
        }
        check_operands(ctx, e, hint);
        Type *lt = e->base->type, *rt = e->index->type;
        if (is_error(lt) || is_error(rt)) return set_type(e, type_error);
        if (op == TOK_PLUS && (lt == type_string || rt == type_string)) {
            /* concatenation creates a new String (spec 6.5) */
            if (lt == type_string && rt == type_string) return set_type(e, type_string);
            Expr *other = lt == type_string ? e->index : e->base;
            error_at(other->loc, "'+' concatenates two strings, but this is %s; convert it with String.of(...)",
                     type_name(other->type));
            return set_type(e, type_error);
        }
        if (!type_is_numeric(lt) || !type_is_numeric(rt)) {
            error_at(e->loc, "'%s' needs numeric operands, found %s and %s", opname, type_name(lt), type_name(rt));
            return set_type(e, type_error);
        }
        return set_type(e, common_numeric(e, lt, rt, opname));
    }
    }
}

static Type *check_cast(Ctx *ctx, Expr *e) {
    Type *target = e->cast_type;
    Expr *operand = e->base;
    Type *t;
    if (is_untyped_literal(operand) && type_is_numeric(target)) {
        /* Use the target type if the literal fits it exactly, else the default. */
        bool fits = false;
        if (operand->kind == E_INT) fits = int_fits(operand->int_value, operand->negative, target);
        else if (operand->kind == E_FLOAT) fits = type_is_float(target);
        if (fits) {
            t = check_value(ctx, operand, target);
        } else {
            t = check_value(ctx, operand, NULL);
        }
    } else {
        t = check_value(ctx, operand, NULL);
    }
    if (is_error(t)) return set_type(e, target);
    bool ok = false;
    if (t == target) ok = true;
    else if (type_is_numeric(t) && type_is_numeric(target)) ok = true;
    else if (t == type_char && type_is_integer(target)) ok = true;
    else if (type_is_integer(t) && target == type_char) ok = true;
    if (!ok) error_at(e->loc, "can't cast %s to %s", type_name(t), type_name(target));
    return set_type(e, target);
}

static Type *check_literal_int(Expr *e, Type *expected) {
    if (expected && (type_is_numeric(expected))) {
        if (!int_fits(e->int_value, e->negative, expected)) {
            error_at(e->loc, "integer literal %s doesn't fit in %s", int_literal_text(e), type_name(expected));
            return set_type(e, type_error);
        }
        return set_type(e, expected);
    }
    e->literal_untyped = true;
    if (!int_fits(e->int_value, e->negative, type_i32)) {
        error_at(e->loc, "integer literal %s doesn't fit in int; give it a wider type, e.g. int64", int_literal_text(e));
        return set_type(e, type_error);
    }
    return set_type(e, type_i32);
}

static Type *check_literal_float(Expr *e, Type *expected) {
    if (expected == type_f32) {
        /* Round the literal's text to float directly; rounding through
         * double first can give a different float (spec 6.3). */
        float f = e->str ? strtof(e->str, NULL) : (float)e->float_value;
        if (e->str && e->negative) f = -f;
        e->float_value = f;
        if (isinf(f)) {
            error_at(e->loc, "floating-point literal doesn't fit in float");
            return set_type(e, type_error);
        }
        return set_type(e, type_f32);
    }
    if (isinf(e->float_value)) {
        error_at(e->loc, "floating-point literal doesn't fit in float64");
        return set_type(e, type_error);
    }
    if (expected != type_f64) e->literal_untyped = true;
    return set_type(e, type_f64);
}

static Type *check_expr(Ctx *ctx, Expr *e, Type *expected) {
    switch (e->kind) {
    case E_INT: return check_literal_int(e, expected);
    case E_FLOAT: return check_literal_float(e, expected);
    case E_CHAR: return set_type(e, type_char);
    case E_STRING: return set_type(e, type_string);
    case E_BOOL: return set_type(e, type_bool);
    case E_THIS:
        if (!ctx->self) {
            error_at(e->loc, "'this' can only be used inside methods and constructors");
            return set_type(e, type_error);
        }
        return set_type(e, ctx->self->type);
    case E_NAME:
    case E_MEMBER:
        resolve_path(ctx, e);
        return e->type;
    case E_INDEX: {
        Type *bt = check_value(ctx, e->base, NULL);
        Type *it = check_value(ctx, e->index, NULL);
        if (!is_error(it) && !type_is_integer(it)) error_at(e->index->loc, "array index must be an integer, found %s", type_name(it));
        if (is_error(bt)) return set_type(e, type_error);
        if (bt->kind != TY_ARRAY) {
            error_at(e->loc, "a value of type %s can't be indexed", type_name(bt));
            return set_type(e, type_error);
        }
        return set_type(e, bt->elem);
    }
    case E_CALL: return check_call(ctx, e);
    case E_UNARY: {
        if (e->op == TOK_BANG) {
            check_value(ctx, e->base, type_bool);
            coerce(&e->base, type_bool, "operand of '!'");
            return set_type(e, type_bool);
        }
        Type *hint = expected && type_is_numeric(expected) ? expected : NULL;
        Type *t = check_value(ctx, e->base, hint);
        if (is_error(t)) return set_type(e, type_error);
        if (e->op == TOK_TILDE) {
            if (!type_is_integer(t)) {
                error_at(e->loc, "'~' needs an integer operand, found %s", type_name(t));
                return set_type(e, type_error);
            }
            return set_type(e, t);
        }
        if (!type_is_numeric(t)) {
            error_at(e->loc, "unary '-' needs a numeric operand, found %s", type_name(t));
            return set_type(e, type_error);
        }
        if (type_is_integer(t) && !type_is_signed(t)) {
            error_at(e->loc, "unary '-' isn't allowed on unsigned type %s", type_name(t));
            return set_type(e, type_error);
        }
        return set_type(e, t);
    }
    case E_BINARY: return check_binary(ctx, e, expected);
    case E_CAST: return check_cast(ctx, e);
    case E_SWITCH:
        check_switch(ctx, e->sw, e->loc, true, e, expected);
        return e->type;
    case E_ARRAY_INIT:
        error_at(e->loc, "an array initialiser '{...}' can only initialise a variable declaration");
        return set_type(e, type_error);
    case E_NEW_ARRAY: {
        /* T[N](): a zero-filled array (spec 6.6) */
        TypeExpr *te = e->cast_expr;
        if (!te->prim && !te->qualifier && find_scope(ctx, te->name)) {
            error_at(e->loc, "'%s' is a variable, and an array element can't be called", te->name);
            return set_type(e, type_error);
        }
        return set_type(e, resolve_type(ctx, te, false, "an array element"));
    }
    default:
        /* Already resolved (e.g. an implicit field reference). */
        return e->type ? e->type : set_type(e, type_error);
    }
}

static Type *check_value(Ctx *ctx, Expr *e, Type *expected) {
    Type *t = check_expr(ctx, e, expected);
    if (is_decl_ref(e)) {
        if (e->decl) {
            Decl *d = e->decl;
            if (d->kind == D_FUNC || d->kind == D_METHOD || d->kind == D_EXTERN_FUNC) {
                error_at(e->loc, "%s '%s' must be called", decl_kind_name(d), d->name);
            } else {
                error_at(e->loc, "'%s' is a %s, not a value", qualified_name(d), decl_kind_name(d));
            }
        } else if (e->name) {
            error_at(e->loc, "'%s' is a built-in function and must be called", e->name);
        }
        e->decl = NULL;
        e->name = NULL;
        return set_type(e, type_error);
    }
    if (t == type_void) {
        error_at(e->loc, "this call doesn't return a value");
        return set_type(e, type_error);
    }
    return t;
}

/* ---------------------------------------------------------------------- */
/* Ownership transfers and assignment targets (spec 7.1, 7.3)             */

static const char *place_description(Expr *e) {
    switch (e->kind) {
    case E_FIELD: return "a field";
    case E_INDEX: return "an array element";
    case E_THIS: return "'this'";
    case E_VAR:
        if (!e->var->is_ref) return "a variable";
        return e->var->kind == VAR_PARAM ? "a reference parameter" : "a reference variable";
    default: return "the value of this expression";
    }
}

/* Validates an owned value in a position where ownership is transferred:
 * variable initialiser, assignment, return, value argument, array
 * initialiser element or switch arm. Only whole local variables (including
 * value parameters) and call results can go to a new owner (spec 7.1, 7.3). */
/* True for expressions that produce a new object nothing else owns. */
static bool is_new_object(Expr *e) {
    switch (e->kind) {
    case E_CALL_FUNC:
    case E_CALL_METHOD:
    case E_CALL_IFACE: return !is_ref_call(e);
    case E_CALL_CTOR:
    case E_CALL_STRING:
    case E_COPY:
    case E_NEW_ARRAY:
    case E_STRING:
    case E_CONST:
    case E_BINARY: return true;
    default: return false;
    }
}

/* Marks existing objects in e to be copied instead of transferred. */
static void mark_implicit_copy(Expr *e) {
    if (e->kind == E_SWITCH) {
        e->transfers_ownership = true; /* the result gets an owner (spec 10.1) */
        for (int i = 0; i < e->sw->arms.len; i++) {
            if (e->sw->arms.data[i].expr) mark_implicit_copy(e->sw->arms.data[i].expr);
        }
        return;
    }
    if (!is_new_object(e)) e->implicit_copy = true;
}

static void check_transfer_source(Ctx *ctx, Expr *e, bool is_argument) {
    (void)ctx;
    if (is_error(e->type) || !type_is_owned(e->type)) return;
    /* Value parameters receive copies of any object (spec 7.3). */
    if (is_argument) {
        mark_implicit_copy(e);
        return;
    }
    switch (e->kind) {
    case E_VAR:
        if (e->var->is_ref) {
            if (e->var->kind == VAR_PARAM) {
                error_at(e->loc, "'%s' refers to the caller's object, which stays with its owner; use %s.copy()",
                         e->var->name, e->var->name);
            } else {
                error_at(e->loc, "'%s' is a reference, and its object stays with its owner; use %s.copy()",
                         e->var->name, e->var->name);
            }
            return;
        }
        e->transfers_ownership = true;
        return;
    case E_CALL_FUNC:
    case E_CALL_METHOD:
    case E_CALL_IFACE:
        if (is_ref_call(e)) {
            error_at(e->loc, "'%s' returns a reference, and its object stays with its owner; use .copy()", e->decl->name);
        }
        return;
    case E_CALL_CTOR:
    case E_CALL_STRING:
    case E_COPY:
    case E_NEW_ARRAY:
    case E_STRING:
    case E_CONST:
    case E_BINARY: return; /* new objects; E_BINARY is a string concatenation */
    case E_SWITCH:
        e->transfers_ownership = true; /* the result gets an owner (spec 10.1) */
        for (int i = 0; i < e->sw->arms.len; i++) {
            if (e->sw->arms.data[i].expr) check_transfer_source(ctx, e->sw->arms.data[i].expr, is_argument);
        }
        return;
    default:
        if (is_argument) {
            error_at(e->loc, "%s stays with its owner and can't be passed by value; pass a copy with .copy(), or declare "
                             "the parameter as a reference (%s*)",
                     place_description(e), type_name(e->type));
        } else {
            error_at(e->loc, "%s stays with its owner and can't be given to a new owner; use .copy()", place_description(e));
        }
        return;
    }
}

static bool assignable_base(Expr *e) {
    switch (e->kind) {
    case E_VAR:
    case E_THIS: return true;
    case E_FIELD:
    case E_INDEX: return assignable_base(e->base);
    default: return is_ref_call(e);
    }
}

static bool check_assign_target(Ctx *ctx, Expr *target) {
    (void)ctx;
    if (is_error(target->type)) return false;
    Expr *t = target;
    switch (t->kind) {
    case E_VAR:
        if (t->var->is_ref && t->var->kind == VAR_LOCAL) {
            error_at(target->loc,
                     "can't assign to reference '%s' as a whole; a reference is set when it is declared (%s)", t->var->name,
                     t->var->type == type_string ? fmt_str("use %s.append(...) to add to its String", t->var->name)
                                                 : "assign its fields instead");
            return false;
        }
        if (t->var->kind == VAR_PARAM && t->var->is_ref) {
            if (t->var->type == type_string) {
                error_at(target->loc,
                         "can't assign to reference parameter '%s' as a whole; it refers to the caller's String (use "
                         "%s.append(...) to add to it)",
                         t->var->name, t->var->name);
            } else {
                error_at(target->loc,
                         "can't assign to reference parameter '%s' as a whole; it refers to the caller's value (assign "
                         "its fields instead)",
                         t->var->name);
            }
            return false;
        }
        return true;
    case E_THIS: error_at(target->loc, "can't assign to 'this'"); return false;
    case E_FIELD:
    case E_INDEX:
        if (!assignable_base(t->base)) {
            error_at(target->loc, "can't assign to a part of a temporary value");
            return false;
        }
        return true;
    case E_CONST: error_at(target->loc, "can't assign to constant '%s'", t->decl->name); return false;
    case E_LENGTH: error_at(target->loc, "an array's length can't be changed"); return false;
    default:
        if (is_ref_call(t)) {
            error_at(target->loc, "can't assign to the result of '%s' as a whole; it refers to an existing object (assign "
                                  "its fields instead)",
                     t->decl->name);
            return false;
        }
        error_at(target->loc, "the left side of an assignment must be a variable, field or array element");
        return false;
    }
}

/* ---------------------------------------------------------------------- */
/* References (spec 9)                                                     */

/* The parameter that argument i of a call is passed to, if any. */
static Var *call_param_of(Expr *call, int i) {
    Decl *fn = call->decl;
    if (call->kind == E_CALL_CTOR) fn = fn->ctor;
    if (!fn || i >= fn->params.len) return NULL;
    return fn->params.data[i];
}

/* True if every object a reference-returning call can refer to satisfies
 * pred: the object it is called on and its reference arguments (spec 9.2). */
static bool ref_call_sources_all(Expr *call, bool (*pred)(Expr *)) {
    if (call->kind != E_CALL_FUNC && !pred(call->base)) return false;
    for (int i = 0; i < call->args.len; i++) {
        Var *p = call_param_of(call, i);
        if (p && p->is_ref && !pred(call->args.data[i])) return false;
    }
    return true;
}

/* True if e names an existing object rather than a temporary one: a
 * variable, parameter, `this`, a field or element of one, or the result of
 * a call that returns a reference to such objects (spec 9.1). */
static bool is_existing_object(Expr *e) {
    switch (e->kind) {
    case E_VAR:
    case E_THIS: return true;
    case E_FIELD:
    case E_INDEX:
    case E_TO_IFACE: return is_existing_object(e->base);
    default: return is_ref_call(e) && ref_call_sources_all(e, is_existing_object);
    }
}

/* True if the object e refers to outlives the function being checked: it is
 * reached through `this`, a reference parameter, or a reference that is
 * (spec 9.2). */
static bool outlives_call(Expr *e) {
    switch (e->kind) {
    case E_THIS: return true;
    case E_VAR: return e->var->is_ref && (e->var->kind == VAR_PARAM || e->var->ref_to_caller);
    case E_FIELD:
    case E_INDEX:
    case E_TO_IFACE: return outlives_call(e->base);
    default: return is_ref_call(e) && ref_call_sources_all(e, outlives_call);
    }
}

/* Describes the local object a reference returned by e would refer to. */
static const char *local_object_description(Expr *e) {
    while (e->kind == E_FIELD || e->kind == E_INDEX || e->kind == E_TO_IFACE) e = e->base;
    if (e->kind == E_VAR) {
        Var *v = e->var;
        if (v->is_ref) return fmt_str("'%s', which refers to an object of this function", v->name);
        if (v->kind == VAR_PARAM) return fmt_str("value parameter '%s', which holds a copy", v->name);
        return fmt_str("local variable '%s'", v->name);
    }
    return fmt_str("the result of '%s', which refers to an object of this function", e->decl->name);
}

/* Checks a value used as a reference of type t: the initialiser of a
 * reference variable or the value returned by a function that returns a
 * reference (spec 9.1, 9.2). A struct becomes a reference to an interface it
 * implements. */
static bool check_reference_value(Ctx *ctx, Expr **pe, Type *t, const char *what, const char *hint) {
    Type *et = check_value(ctx, *pe, t);
    if (is_error(et) || is_error(t)) return false;
    if (et != t) {
        if (t->kind == TY_INTERFACE && et->kind == TY_STRUCT && struct_implements(et->decl, t->decl)) {
            wrap_to_iface(pe, t);
        } else {
            error_at((*pe)->loc, "%s: expected %s, found %s", what, type_name(t), type_name(et));
            return false;
        }
    }
    if ((*pe)->kind == E_SWITCH) {
        error_at((*pe)->loc,
                 "%s must name an existing object directly (a variable, parameter, this, a part of one, or a call that "
                 "returns a reference), not choose one with a switch expression",
                 what);
        return false;
    }
    if (!is_existing_object(*pe)) {
        error_at((*pe)->loc,
                 "%s must refer to an existing object, but this is a temporary value, destroyed at the end of the "
                 "statement%s",
                 what, hint);
        return false;
    }
    return true;
}

static void check_reference_decl(Ctx *ctx, Stmt *s) {
    Var *v = s->var;
    v->type = resolve_type(ctx, v->type_expr, true, "a variable");
    if (!is_error(v->type) && !type_is_owned(v->type) && v->type->kind != TY_INTERFACE) {
        error_at(v->loc, "only struct, array, String and interface types have references; %s is always a value, so "
                         "declare '%s' without '*'",
                 type_name(v->type), v->name);
        v->type = type_error;
    }
    if (!s->init) {
        error_at(v->loc, "reference '%s' must be initialised with the object it refers to: %s* %s = ...;", v->name,
                 type_name(v->type), v->name);
    } else if (s->init->kind == E_ARRAY_INIT) {
        error_at(s->init->loc, "reference '%s' must refer to an existing object, not an array initialiser", v->name);
    } else if (check_reference_value(ctx, &s->init, v->type, fmt_str("reference '%s'", v->name),
                                     "; store the object in a variable and refer to that")) {
        v->ref_to_caller = outlives_call(s->init);
    }
    if (declare_name(ctx, v->name, v->loc, v)) add_local(ctx, v);
}

/* ---------------------------------------------------------------------- */
/* Switch                                                                  */

static void check_switch(Ctx *ctx, SwitchData *sw, Loc loc, bool is_expr, Expr *e, Type *expected) {
    Type *st = check_value(ctx, sw->subject, NULL);
    bool bad = is_error(st);
    if (!bad) {
        if (st->kind == TY_ENUM) sw->kind = SW_ENUM;
        else if (type_is_integer(st)) sw->kind = SW_INT;
        else if (st == type_char) sw->kind = SW_CHAR;
        else if (st == type_bool) sw->kind = SW_BOOL;
        else if (st == type_string) sw->kind = SW_STRING;
        else {
            error_at(sw->subject->loc, "can't switch on a value of type %s", type_name(st));
            bad = true;
        }
        if (!bad && is_reference(sw->subject)) {
            error_at(sw->subject->loc, "can't switch on %s: it is a reference, and a switch compares its subject with values",
                     reference_name(sw->subject));
            bad = true;
        }
    }
    Loc default_loc = {0};
    for (int i = 0; i < sw->arms.len; i++) {
        SwitchArm *arm = &sw->arms.data[i];
        if (arm->is_default) {
            if (sw->has_default) error_at(arm->loc, "duplicate 'default' branch");
            sw->has_default = true;
            default_loc = arm->loc;
            continue;
        }
        if (bad) continue;
        for (int j = 0; j < arm->values.len; j++) {
            CaseValue *cv = &arm->values.data[j];
            Expr *v = cv->expr;
            bool kind_ok = false;
            switch (sw->kind) {
            case SW_ENUM:
                if (v->kind == E_NAME) {
                    kind_ok = true;
                    Decl *en = st->decl;
                    int found = -1;
                    for (int k = 0; k < en->values.len; k++) {
                        if (en->values.data[k].name == v->name) found = k;
                    }
                    if (found < 0) {
                        error_at(cv->loc, "'%s' isn't a value of enum '%s'", v->name, type_name(st));
                        continue;
                    }
                    cv->ivalue = found;
                }
                break;
            case SW_INT:
                if (v->kind == E_INT) {
                    kind_ok = true;
                    if (!int_fits(v->int_value, v->negative, st)) {
                        error_at(cv->loc, "case value %s doesn't fit in %s", int_literal_text(v), type_name(st));
                        continue;
                    }
                    cv->uvalue = v->int_value;
                    cv->is_negative = v->negative && v->int_value != 0;
                }
                break;
            case SW_CHAR:
                if (v->kind == E_CHAR) {
                    kind_ok = true;
                    cv->uvalue = v->int_value;
                }
                break;
            case SW_BOOL:
                if (v->kind == E_BOOL) {
                    kind_ok = true;
                    cv->uvalue = v->bool_value;
                }
                break;
            case SW_STRING:
                if (v->kind == E_STRING) {
                    kind_ok = true;
                    cv->svalue = v->str;
                    cv->slen = v->str_len;
                }
                break;
            }
            if (!kind_ok) {
                if (v->kind == E_NAME) {
                    error_at(cv->loc, "case value '%s' must be a literal of type %s", v->name, type_name(st));
                } else {
                    error_at(cv->loc, "case value doesn't match the switch type %s", type_name(st));
                }
                continue;
            }
            v->type = st;
            /* duplicates */
            for (int a = 0; a <= i; a++) {
                SwitchArm *other = &sw->arms.data[a];
                int limit = a == i ? j : other->values.len;
                for (int b = 0; b < limit; b++) {
                    CaseValue *ov = &other->values.data[b];
                    if (!ov->expr->type || ov->expr->type != st) continue;
                    bool same;
                    if (sw->kind == SW_STRING) {
                        same = ov->slen == cv->slen && memcmp(ov->svalue, cv->svalue, cv->slen) == 0;
                    } else if (sw->kind == SW_ENUM) {
                        same = ov->ivalue == cv->ivalue;
                    } else {
                        same = ov->uvalue == cv->uvalue && ov->is_negative == cv->is_negative;
                    }
                    if (same) {
                        error_at(cv->loc, "duplicate case value");
                        goto next_value;
                    }
                }
            }
        next_value:;
        }
    }
    (void)default_loc;

    /* Exhaustiveness (spec 10.2). */
    if (!bad && !sw->has_default) {
        if (sw->kind == SW_ENUM) {
            Decl *en = st->decl;
            StrBuf missing = {0};
            int count = 0;
            for (int k = 0; k < en->values.len; k++) {
                bool covered = false;
                for (int i = 0; i < sw->arms.len && !covered; i++) {
                    SwitchArm *arm = &sw->arms.data[i];
                    for (int j = 0; j < arm->values.len; j++) {
                        if (arm->values.data[j].expr->type == st && arm->values.data[j].ivalue == k) covered = true;
                    }
                }
                if (!covered) {
                    sb_printf(&missing, "%s%s", count ? ", " : "", en->values.data[k].name);
                    count++;
                }
            }
            if (count) {
                error_at(loc, "switch over enum '%s' isn't exhaustive: missing %s", type_name(st), sb_cstr(&missing));
            }
        } else if (sw->kind == SW_BOOL) {
            bool seen[2] = {false, false};
            for (int i = 0; i < sw->arms.len; i++) {
                for (int j = 0; j < sw->arms.data[i].values.len; j++) {
                    CaseValue *cv = &sw->arms.data[i].values.data[j];
                    if (cv->expr->type == st) seen[cv->uvalue ? 1 : 0] = true;
                }
            }
            if (!seen[0] || !seen[1]) {
                error_at(loc, "switch over boolean isn't exhaustive: missing %s", !seen[1] ? "true" : "false");
            }
        } else {
            error_at(loc, "a switch over %s needs a 'default' branch", type_name(st));
        }
    }

    /* Arms */
    if (is_expr) {
        if (sw->arms.len == 0) {
            error_at(loc, "a switch expression needs at least one branch");
            e->type = type_error;
            return;
        }
        Type *result = expected && !is_error(expected) ? expected : NULL;
        if (!result) {
            /* Infer from the first arm that isn't an untyped literal. */
            int first = 0;
            for (int i = 0; i < sw->arms.len; i++) {
                if (!is_untyped_literal(sw->arms.data[i].expr)) {
                    first = i;
                    break;
                }
            }
            result = check_value(ctx, sw->arms.data[first].expr, NULL);
            for (int i = 0; i < sw->arms.len; i++) {
                if (i == first) continue;
                check_value(ctx, sw->arms.data[i].expr, result);
                if (!type_is_owned(result)) coerce(&sw->arms.data[i].expr, result, "switch branch");
                else if (!is_error(sw->arms.data[i].expr->type) && sw->arms.data[i].expr->type != result)
                    error_at(sw->arms.data[i].expr->loc, "switch branch: expected %s, found %s", type_name(result),
                             type_name(sw->arms.data[i].expr->type));
            }
        } else {
            for (int i = 0; i < sw->arms.len; i++) {
                check_value(ctx, sw->arms.data[i].expr, result);
                if (!type_is_owned(result)) coerce(&sw->arms.data[i].expr, result, "switch branch");
                else if (!is_error(sw->arms.data[i].expr->type) && sw->arms.data[i].expr->type != result)
                    error_at(sw->arms.data[i].expr->loc, "switch branch: expected %s, found %s", type_name(result),
                             type_name(sw->arms.data[i].expr->type));
            }
        }
        if (result && result->kind == TY_INTERFACE) {
            error_at(loc, "a switch expression can't produce an interface value");
            result = type_error;
        }
        e->type = result ? result : type_error;
    }
}

/* ---------------------------------------------------------------------- */
/* Statements                                                              */

static void check_stmt(Ctx *ctx, Stmt *s);

static void check_block_scoped(Ctx *ctx, Stmt *s) {
    push_scope(ctx);
    if (s->kind == S_BLOCK) {
        for (int i = 0; i < s->stmts.len; i++) check_stmt(ctx, s->stmts.data[i]);
    } else {
        check_stmt(ctx, s);
    }
    pop_scope(ctx);
}

static void check_condition(Ctx *ctx, Expr **pe, const char *what) {
    Type *t = check_value(ctx, *pe, type_bool);
    if (!is_error(t) && t != type_bool) {
        error_at((*pe)->loc, "%s must be boolean, found %s", what, type_name(t));
        return;
    }
}

static void check_array_init(Ctx *ctx, Expr *init, Type *t) {
    if (is_error(t)) return;
    if (t->kind != TY_ARRAY) {
        error_at(init->loc, "an initialiser '{...}' needs an array type, not %s", type_name(t));
        return;
    }
    init->type = t;
    if (init->args.len != t->length) {
        error_at(init->loc, "array of type %s needs exactly %lld elements, but the initialiser has %d", type_name(t),
                 (long long)t->length, init->args.len);
    }
    for (int i = 0; i < init->args.len; i++) {
        Expr *el = init->args.data[i];
        if (el->kind == E_ARRAY_INIT) {
            check_array_init(ctx, el, t->elem);
            continue;
        }
        Type *et = check_value(ctx, el, t->elem);
        if (type_is_owned(t->elem)) {
            if (!is_error(et) && et != t->elem) {
                error_at(el->loc, "array element: expected %s, found %s", type_name(t->elem), type_name(et));
            } else {
                check_transfer_source(ctx, el, false);
            }
        } else {
            coerce(&init->args.data[i], t->elem, "array element");
        }
    }
}

static void check_initialiser(Ctx *ctx, Var *v, Expr **pinit) {
    Expr *init = *pinit;
    Type *t = v->type;
    if (init->kind == E_ARRAY_INIT) {
        check_array_init(ctx, init, t);
        return;
    }
    Type *it = check_value(ctx, init, t);
    if (is_error(t)) return;
    if (type_is_owned(t)) {
        if (!is_error(it) && it != t) {
            error_at(init->loc, "initialiser of '%s': expected %s, found %s", v->name, type_name(t), type_name(it));
            return;
        }
        check_transfer_source(ctx, init, false);
        return;
    }
    coerce(pinit, t, fmt_str("initialiser of '%s'", v->name));
}

static void check_var_decl(Ctx *ctx, Stmt *s) {
    Var *v = s->var;
    if (v->is_ref) {
        check_reference_decl(ctx, s);
        return;
    }
    v->type = resolve_type(ctx, v->type_expr, false, "a variable");
    if (s->init) check_initialiser(ctx, v, &s->init); /* without one, it must be assigned before use (spec 6.8) */
    if (declare_name(ctx, v->name, v->loc, v)) add_local(ctx, v);
}

static void check_assign(Ctx *ctx, Stmt *s) {
    Type *tt = check_value(ctx, s->expr, NULL);
    bool target_ok = check_assign_target(ctx, s->expr);
    if (s->op == TOK_ASSIGN) {
        Type *vt = check_value(ctx, s->value, is_error(tt) ? NULL : tt);
        if (!target_ok || is_error(tt)) return;
        if (type_is_owned(tt)) {
            if (!is_error(vt) && vt != tt) {
                error_at(s->value->loc, "assignment: expected %s, found %s", type_name(tt), type_name(vt));
                return;
            }
            check_transfer_source(ctx, s->value, false);
            return;
        }
        if (tt->kind == TY_INTERFACE) return;
        coerce(&s->value, tt, "assignment");
        return;
    }
    /* compound assignment */
    const char *opname = token_kind_name(s->op);
    opname = fmt_str("%.*s", (int)strlen(opname) - 2, opname + 1);
    bool is_shift = s->op == TOK_SHL_ASSIGN || s->op == TOK_SHR_ASSIGN;
    bool is_bitwise = s->op == TOK_AMP_ASSIGN || s->op == TOK_PIPE_ASSIGN || s->op == TOK_CARET_ASSIGN;
    Type *vt = check_value(ctx, s->value, is_shift ? NULL : tt);
    if (is_error(tt) || is_error(vt)) return;
    if (tt == type_string) {
        /* a += b appends b to a (spec 6.5) */
        if (s->op != TOK_PLUS_ASSIGN) {
            error_at(s->loc, "'%s' doesn't apply to strings; only '+=' does", opname);
        } else if (vt != type_string) {
            error_at(s->value->loc, "'+=' appends a String, but this is %s; convert it with String.of(...)", type_name(vt));
        }
        return;
    }
    if (is_shift || is_bitwise) {
        if (!type_is_integer(tt)) {
            error_at(s->loc, "'%s' needs an integer variable, found %s", opname, type_name(tt));
            return;
        }
        if (is_shift) {
            if (!type_is_integer(vt)) error_at(s->value->loc, "the shift amount must be an integer, found %s", type_name(vt));
            return;
        }
    } else if (!type_is_numeric(tt)) {
        error_at(s->loc, "'%s' needs a numeric variable, found %s", opname, type_name(tt));
        return;
    }
    if (!type_is_numeric(vt)) {
        error_at(s->value->loc, "'%s' needs a numeric value, found %s", opname, type_name(vt));
        return;
    }
    if (vt != tt && !implicit_ok(vt, tt)) {
        error_at(s->value->loc, "'%s': can't convert %s to %s implicitly; use a cast", opname, type_name(vt), type_name(tt));
        return;
    }
    coerce(&s->value, tt, opname);
}

static void check_stmt(Ctx *ctx, Stmt *s) {
    switch (s->kind) {
    case S_BLOCK: check_block_scoped(ctx, s); return;
    case S_VAR: check_var_decl(ctx, s); return;
    case S_EXPR: {
        Expr *e = s->expr;
        Type *t = check_expr(ctx, e, NULL);
        (void)t;
        if (is_decl_ref(e)) {
            check_value(ctx, e, NULL);
            return;
        }
        if (e->kind != E_CALL_FUNC && e->kind != E_CALL_METHOD && e->kind != E_CALL_IFACE && e->kind != E_CALL_CTOR &&
            e->kind != E_CALL_PRINT && e->kind != E_CALL_STRING && e->kind != E_COPY && e->kind != E_EQUALS &&
            !is_error(e->type)) {
            error_at(e->loc, "this expression isn't a statement; only calls can be used as statements");
        }
        return;
    }
    case S_ASSIGN: check_assign(ctx, s); return;
    case S_INCDEC: {
        Type *t = check_value(ctx, s->expr, NULL);
        if (!check_assign_target(ctx, s->expr)) return;
        if (!is_error(t) && !type_is_numeric(t)) {
            error_at(s->loc, "'%s' needs a numeric variable, found %s", s->op == TOK_PLUSPLUS ? "++" : "--", type_name(t));
        }
        return;
    }
    case S_IF:
        check_condition(ctx, &s->expr, "the condition of 'if'");
        check_block_scoped(ctx, s->then_branch);
        if (s->else_branch) check_block_scoped(ctx, s->else_branch);
        return;
    case S_WHILE:
        check_condition(ctx, &s->expr, "the condition of 'while'");
        ctx->loop_depth++;
        check_block_scoped(ctx, s->body);
        ctx->loop_depth--;
        return;
    case S_DO:
        ctx->loop_depth++;
        check_block_scoped(ctx, s->body);
        ctx->loop_depth--;
        check_condition(ctx, &s->expr, "the condition of 'do-while'");
        return;
    case S_FOR:
        push_scope(ctx);
        if (s->for_init) check_stmt(ctx, s->for_init);
        if (s->expr) check_condition(ctx, &s->expr, "the condition of 'for'");
        if (s->for_update) check_stmt(ctx, s->for_update);
        ctx->loop_depth++;
        check_block_scoped(ctx, s->body);
        ctx->loop_depth--;
        pop_scope(ctx);
        return;
    case S_SWITCH:
        check_switch(ctx, s->sw, s->loc, false, NULL, NULL);
        for (int i = 0; i < s->sw->arms.len; i++) check_block_scoped(ctx, s->sw->arms.data[i].body);
        return;
    case S_BREAK:
        if (ctx->loop_depth == 0) error_at(s->loc, "'break' outside a loop");
        return;
    case S_CONTINUE:
        if (ctx->loop_depth == 0) error_at(s->loc, "'continue' outside a loop");
        return;
    case S_RETURN: {
        Type *rt = ctx->ret_type;
        if (!s->expr) {
            if (rt != type_void && !is_error(rt)) error_at(s->loc, "missing return value; the function returns %s", type_name(rt));
            return;
        }
        if (rt == type_void) {
            check_expr(ctx, s->expr, NULL);
            error_at(s->expr->loc, "%s can't return a value",
                     ctx->fn->kind == D_CTOR ? "a constructor" : "a void function");
            return;
        }
        if (ctx->fn->ret_is_ref) {
            if (check_reference_value(ctx, &s->expr, rt, "the returned reference", "") && !outlives_call(s->expr)) {
                error_at(s->expr->loc,
                         "can't return a reference to %s: it is destroyed when the function returns; return this, a "
                         "reference parameter or a part of one",
                         local_object_description(s->expr));
            }
            return;
        }
        Type *t = check_value(ctx, s->expr, rt);
        if (is_error(rt)) return;
        if (type_is_owned(rt)) {
            if (!is_error(t) && t != rt) {
                error_at(s->expr->loc, "return value: expected %s, found %s", type_name(rt), type_name(t));
                return;
            }
            check_transfer_source(ctx, s->expr, false);
            return;
        }
        coerce(&s->expr, rt, "return value");
        return;
    }
    }
}

/* ---------------------------------------------------------------------- */
/* Return paths                                                            */

static bool is_true_literal(Expr *e) {
    return e && e->kind == E_BOOL && e->bool_value;
}

/* How control can leave a statement, other than by returning. A non-void
 * function returns on every path iff its body can't complete normally. */
enum { EXIT_NORMAL = 1, EXIT_BREAK = 2, EXIT_CONTINUE = 4 };

static int exits(Stmt *s) {
    if (!s) return EXIT_NORMAL;
    switch (s->kind) {
    case S_RETURN: return 0;
    case S_BREAK: return EXIT_BREAK;
    case S_CONTINUE: return EXIT_CONTINUE;
    case S_BLOCK: {
        int current = EXIT_NORMAL;
        for (int i = 0; i < s->stmts.len && (current & EXIT_NORMAL); i++) {
            current = (current & ~EXIT_NORMAL) | exits(s->stmts.data[i]);
        }
        return current;
    }
    case S_IF: return exits(s->then_branch) | (s->else_branch ? exits(s->else_branch) : EXIT_NORMAL);
    case S_SWITCH: {
        int all = s->sw->arms.len ? 0 : EXIT_NORMAL;
        for (int i = 0; i < s->sw->arms.len; i++) all |= exits(s->sw->arms.data[i].body);
        return all;
    }
    case S_WHILE:
    case S_FOR: {
        bool forever = s->kind == S_FOR ? (!s->expr || is_true_literal(s->expr)) : is_true_literal(s->expr);
        int body = exits(s->body);
        return (!forever || (body & EXIT_BREAK)) ? EXIT_NORMAL : 0;
    }
    case S_DO: {
        int body = exits(s->body);
        bool reaches_condition = body & (EXIT_NORMAL | EXIT_CONTINUE);
        return ((body & EXIT_BREAK) || (reaches_condition && !is_true_literal(s->expr))) ? EXIT_NORMAL : 0;
    }
    default: return EXIT_NORMAL;
    }
}

/* ---------------------------------------------------------------------- */
/* Functions                                                               */

static void check_function(Program *prog, Decl *fn) {
    Ctx ctx = {0};
    ctx.prog = prog;
    ctx.file = fn->file;
    ctx.ns = fn->ns;
    ctx.fn = fn;
    ctx.self = (fn->kind == D_METHOD || fn->kind == D_CTOR) ? fn->owner : NULL;
    ctx.ret_type = fn->ret_type;
    push_scope(&ctx);
    for (int i = 0; i < fn->params.len; i++) {
        Var *p = fn->params.data[i];
        declare_name(&ctx, p->name, p->loc, p);
        add_local(&ctx, p);
    }
    for (int i = 0; i < fn->body->stmts.len; i++) check_stmt(&ctx, fn->body->stmts.data[i]);
    pop_scope(&ctx);
    if (fn->ret_type != type_void && !is_error(fn->ret_type) && (exits(fn->body) & EXIT_NORMAL)) {
        error_at(fn->loc, "%s '%s' doesn't return a value on every path", decl_kind_name(fn), fn->name);
    }
}

void sema_check_program(Program *prog) {
    name_print = intern_cstr("print");
    name_println = intern_cstr("println");
    name_copy = intern_cstr("copy");
    name_equals = intern_cstr("equals");
    name_length = intern_cstr("length");
    name_main = intern_cstr("main");
    name_string = intern_cstr("String");

    collect_program(prog);

    for (int i = 0; i < prog->all_consts.len; i++) check_const(prog, prog->all_consts.data[i]);
    for (int i = 0; i < prog->all_structs.len; i++) resolve_struct_fields(prog, prog->all_structs.data[i]);
    for (int i = 0; i < prog->all_interfaces.len; i++) {
        Decl *iface = prog->all_interfaces.data[i];
        for (int j = 0; j < iface->members.len; j++) resolve_signature(prog, iface->members.data[j]);
    }
    for (int i = 0; i < prog->all_functions.len; i++) resolve_signature(prog, prog->all_functions.data[i]);
    for (int i = 0; i < prog->all_externs.len; i++) resolve_signature(prog, prog->all_externs.data[i]);
    check_struct_cycles(prog);
    check_implementations(prog);

    Decl *main_fn = map_get(&prog->globals, name_main);
    if (!main_fn || main_fn->kind != D_FUNC) {
        Loc none = {0};
        error_at(none, "the program has no main function; declare 'void main()' or 'int main()' at the top level");
    } else {
        if (main_fn->params.len > 0) error_at(main_fn->loc, "main can't take parameters");
        if (main_fn->ret_type != type_void && main_fn->ret_type != type_i32 && !is_error(main_fn->ret_type)) {
            error_at(main_fn->loc, "main must return void or int");
        }
        prog->main = main_fn;
    }

    for (int i = 0; i < prog->all_functions.len; i++) check_function(prog, prog->all_functions.data[i]);
}
