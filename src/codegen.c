/* C code generation (spec 13). Expressions are lowered to a sequence of C
 * statements with temporaries so that evaluation is strictly left to right
 * and every checked operation runs in source order.
 *
 * Owned objects that hold heap memory (strings, and structs and arrays that
 * contain them) are destroyed automatically (spec 7.6):
 * - locals at every exit from their block, in reverse declaration order;
 * - temporaries that nothing took ownership of, at the end of the statement
 *   (or of the conditional sub-expression) that created them;
 * - the old object when a variable, field or element is assigned.
 * A local that transfers ownership is reset to its zero value, which needs
 * no cleanup, so the generated code never needs drop flags. */
#include "codegen.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "lexer.h"

typedef struct LoopLabels {
    int id;
    bool brk_used;
    bool cont_used;
    int scope_mark; /* drop_vars length when the loop body started */
    struct LoopLabels *outer;
} LoopLabels;

typedef struct Owned {
    const char *name; /* C lvalue */
    Type *type;
    bool consumed;
} Owned;

typedef VEC(Owned) OwnedVec;
typedef VEC(int) IntVec;

typedef struct CG {
    Program *prog;
    StrBuf *out;
    int indent;
    int tmp;
    int label;
    Decl *fn;
    LoopLabels *loop;
    OwnedVec temps;     /* pending temporaries that may need cleanup */
    OwnedVec drop_vars; /* locals that need cleanup, innermost last */
    IntVec scope_marks; /* drop_vars length at the start of each block */
} CG;

/* ---------------------------------------------------------------------- */
/* Output helpers                                                          */

static void emit(CG *cg, const char *fmt, ...) {
    for (int i = 0; i < cg->indent; i++) sb_append(cg->out, "    ");
    va_list ap;
    va_start(ap, fmt);
    sb_vprintf(cg->out, fmt, ap);
    va_end(ap);
    sb_putc(cg->out, '\n');
}

static const char *new_tmp(CG *cg) {
    return fmt_str("nr_t%d", ++cg->tmp);
}

static const char *ctype(Type *t) {
    return t->cname;
}

static const char *loc_str(Loc loc) {
    StrBuf sb = {0};
    sb_putc(&sb, '"');
    const char *path = loc.file ? loc.file->path : "?";
    for (const char *p = path; *p; p++) {
        if (*p == '"' || *p == '\\') sb_putc(&sb, '\\');
        sb_putc(&sb, *p);
    }
    sb_printf(&sb, ":%d:%d\"", loc.line, loc.col);
    return sb_cstr(&sb);
}

static const char *c_string_literal(const char *s, size_t len) {
    StrBuf sb = {0};
    sb_putc(&sb, '"');
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        switch (c) {
        case '"': sb_append(&sb, "\\\""); break;
        case '\\': sb_append(&sb, "\\\\"); break;
        case '\n': sb_append(&sb, "\\n"); break;
        case '\t': sb_append(&sb, "\\t"); break;
        case '\r': sb_append(&sb, "\\r"); break;
        case '?': sb_append(&sb, "\\?"); break;
        default:
            if (c < 32 || c >= 127) sb_printf(&sb, "\\%03o", c);
            else sb_putc(&sb, (char)c);
        }
    }
    sb_putc(&sb, '"');
    return sb_cstr(&sb);
}

/* Static initialiser for a String holding literal bytes. */
static const char *string_init(const char *s, size_t len) {
    return fmt_str("{(char *)%s, %zu, 0}", c_string_literal(s, len), len);
}

static const char *type_suffix(Type *t) {
    switch (t->kind) {
    case TY_I8: return "i8";
    case TY_I16: return "i16";
    case TY_I32: return "i32";
    case TY_I64: return "i64";
    case TY_U8:
    case TY_CHAR: return "u8";
    case TY_U16: return "u16";
    case TY_U32: return "u32";
    case TY_U64: return "u64";
    case TY_F32: return "f32";
    case TY_F64: return "f64";
    default: return "?";
    }
}

static const char *zero_value(Type *t) {
    switch (t->kind) {
    case TY_STRUCT:
    case TY_ARRAY:
    case TY_STRING: return fmt_str("((%s){0})", ctype(t));
    case TY_BOOL: return "false";
    case TY_F32: return "0.0f";
    case TY_F64: return "0.0";
    default: return fmt_str("((%s)0)", ctype(t));
    }
}

static const char *zero_init(Type *t) {
    switch (t->kind) {
    case TY_STRUCT:
    case TY_ARRAY:
    case TY_STRING: return "{0}";
    default: return zero_value(t);
    }
}

static const char *float_literal(double v, bool is_f32) {
    char buf[64];
    if (is_f32) {
        snprintf(buf, sizeof buf, "%.9g", (double)(float)v);
    } else {
        snprintf(buf, sizeof buf, "%.17g", v);
    }
    if (!strpbrk(buf, ".eEn")) strcat(buf, ".0");
    return fmt_str("%s%s", buf, is_f32 ? "f" : "");
}

static const char *int_literal(Type *t, uint64_t mag, bool neg) {
    if (neg && mag == 0) neg = false;
    if (type_is_float(t)) {
        double d = (double)mag;
        return float_literal(neg ? -d : d, t == type_f32);
    }
    if (type_is_signed(t)) {
        if (neg) {
            if (mag == 9223372036854775808ULL) return fmt_str("((%s)INT64_MIN)", ctype(t));
            return fmt_str("((%s)-INT64_C(%llu))", ctype(t), (unsigned long long)mag);
        }
        if (t == type_i32) return fmt_str("%llu", (unsigned long long)mag);
        return fmt_str("((%s)INT64_C(%llu))", ctype(t), (unsigned long long)mag);
    }
    return fmt_str("((%s)UINT64_C(%llu))", ctype(t), (unsigned long long)mag);
}

/* Returns c wrapped in exactly one pair of parentheses, for if (...). */
static const char *paren(const char *c) {
    if (c[0] == '(') {
        int depth = 0;
        size_t n = strlen(c);
        for (size_t i = 0; i < n; i++) {
            if (c[i] == '(') depth++;
            else if (c[i] == ')') depth--;
            if (depth == 0) {
                if (i == n - 1) return c;
                break;
            }
        }
    }
    return fmt_str("(%s)", c);
}

/* ---------------------------------------------------------------------- */
/* Names                                                                   */

static const char *var_ref(Var *v) {
    if (v->kind == VAR_PARAM && v->is_ref && type_is_owned(v->type)) return fmt_str("(*nr_l_%s)", v->name);
    return fmt_str("nr_l_%s", v->name);
}

static const char *field_name(Decl *f) {
    return fmt_str("f_%s", f->name);
}

static void assign_names(Program *prog) {
    for (int i = 0; i < prog->all_structs.len; i++) {
        Decl *s = prog->all_structs.data[i];
        s->cname = fmt_str("nr%d_%s", s->id, s->name);
        s->type->cname = s->cname;
    }
    for (int i = 0; i < prog->all_interfaces.len; i++) {
        Decl *d = prog->all_interfaces.data[i];
        d->cname = fmt_str("nri%d_%s", d->id, d->name);
        d->type->cname = d->cname;
    }
    for (int i = 0; i < prog->all_enums.len; i++) {
        Decl *d = prog->all_enums.data[i];
        d->cname = fmt_str("nr%d_%s", d->id, d->name);
        d->type->cname = "int32_t";
    }
    for (int i = 0; i < prog->all_consts.len; i++) {
        Decl *d = prog->all_consts.data[i];
        d->cname = fmt_str("nr%d_%s", d->id, d->name);
    }
    for (int i = 0; i < prog->all_functions.len; i++) {
        Decl *d = prog->all_functions.data[i];
        if (d->kind == D_METHOD) d->cname = fmt_str("nr%d_%s_%s", d->id, d->owner->name, d->name);
        else if (d->kind == D_CTOR) d->cname = fmt_str("nr%d_%s_new", d->id, d->owner->name);
        else d->cname = fmt_str("nr%d_%s", d->id, d->name);
    }
    for (int i = 0; i < prog->all_externs.len; i++) {
        Decl *d = prog->all_externs.data[i];
        d->cname = d->name;
    }
}

static const char *vtable_name(Decl *s, Decl *iface) {
    return fmt_str("nr_vt_%d_%d", s->id, iface->id);
}

/* ---------------------------------------------------------------------- */
/* Cleanup, copying and equality of owned types                            */

/* True if values of t own heap memory, directly or through fields. */
static bool needs_drop(Type *t) {
    if (t->kind == TY_STRING) return true;
    if (t->kind == TY_ARRAY) return needs_drop(t->elem);
    if (t->kind != TY_STRUCT) return false;
    if (t->drop_state) return t->drop_state == 2;
    t->drop_state = 1;
    Decl *s = t->decl;
    for (int i = 0; i < s->members.len; i++) {
        Decl *m = s->members.data[i];
        if (m->kind == D_FIELD && needs_drop(m->field_type)) t->drop_state = 2;
    }
    return t->drop_state == 2;
}

/* C statement that destroys the object at lvalue. */
static const char *drop_stmt(Type *t, const char *lvalue) {
    if (t->kind == TY_STRING) return fmt_str("nr_str_free(&%s);", lvalue);
    return fmt_str("nr_drop_%s(&%s);", ctype(t), lvalue);
}

/* C expression for an independent copy of the object at lvalue (spec 7.2). */
static const char *copy_expr(Type *t, const char *lvalue) {
    if (!needs_drop(t)) return lvalue;
    if (t->kind == TY_STRING) return fmt_str("nr_str_copy(&%s)", lvalue);
    return fmt_str("nr_copy_%s(&%s)", ctype(t), lvalue);
}

/* C expression comparing the values at two lvalues (spec 6.9). */
static const char *eq_expr(Type *t, const char *a, const char *b) {
    switch (t->kind) {
    case TY_STRING: return fmt_str("nr_str_eq(&%s, &%s)", a, b);
    case TY_STRUCT:
    case TY_ARRAY: return fmt_str("nr_eq_%s(&%s, &%s)", ctype(t), a, b);
    default: return fmt_str("(%s == %s)", a, b);
    }
}

/* Registers a temporary whose object nothing owns yet. */
static void add_temp(CG *cg, const char *name, Type *t) {
    if (!type_is_owned(t) || !needs_drop(t)) return;
    Owned o = {name, t, false};
    vec_push(cg->temps, o);
}

/* Marks a temporary as taken over by a new owner. */
static void consume(CG *cg, const char *name) {
    for (int i = cg->temps.len - 1; i >= 0; i--) {
        if (!cg->temps.data[i].consumed && strcmp(cg->temps.data[i].name, name) == 0) {
            cg->temps.data[i].consumed = true;
            return;
        }
    }
}

static bool temps_pending(CG *cg, int mark) {
    for (int i = mark; i < cg->temps.len; i++) {
        if (!cg->temps.data[i].consumed) return true;
    }
    return false;
}

/* Destroys the temporaries created since mark that nothing took over. */
static void drop_temps(CG *cg, int mark) {
    for (int i = cg->temps.len - 1; i >= mark; i--) {
        Owned *o = &cg->temps.data[i];
        if (!o->consumed) emit(cg, "%s", drop_stmt(o->type, o->name));
    }
    cg->temps.len = mark;
}

static void push_scope(CG *cg) {
    vec_push(cg->scope_marks, cg->drop_vars.len);
}

static void add_drop_var(CG *cg, const char *name, Type *t) {
    if (!needs_drop(t)) return;
    Owned o = {name, t, false};
    vec_push(cg->drop_vars, o);
}

/* Destroys locals from the innermost one down to index from. */
static void drop_vars_from(CG *cg, int from) {
    for (int i = cg->drop_vars.len - 1; i >= from; i--) {
        emit(cg, "%s", drop_stmt(cg->drop_vars.data[i].type, cg->drop_vars.data[i].name));
    }
}

static void pop_scope(CG *cg) {
    int mark = cg->scope_marks.data[--cg->scope_marks.len];
    drop_vars_from(cg, mark);
    cg->drop_vars.len = mark;
}

/* ---------------------------------------------------------------------- */
/* Expressions                                                             */

static const char *gen_expr(CG *cg, Expr *e);
static const char *gen_place(CG *cg, Expr *e);
static void gen_stmt(CG *cg, Stmt *s);

static bool is_place(Expr *e) {
    switch (e->kind) {
    case E_VAR:
    case E_THIS:
    case E_INDEX:
    case E_FIELD: return true;
    case E_ALIAS: return is_place(e->alias->expr);
    default: return false;
    }
}

/* Returns a C lvalue for a place. Index expressions are evaluated into
 * temporaries and bounds-checked here, so the returned string has no side
 * effects and can be evaluated more than once. */
static const char *gen_place(CG *cg, Expr *e) {
    switch (e->kind) {
    case E_VAR: return var_ref(e->var);
    case E_THIS: return "(*nr_self)";
    case E_FIELD: {
        const char *base = is_place(e->base) ? gen_place(cg, e->base) : gen_expr(cg, e->base);
        return fmt_str("%s.%s", base, field_name(e->decl));
    }
    case E_INDEX: {
        const char *base = is_place(e->base) ? gen_place(cg, e->base) : gen_expr(cg, e->base);
        const char *index = gen_expr(cg, e->index);
        const char *t = new_tmp(cg);
        Type *it = e->index->type;
        bool is_unsigned = type_is_integer(it) && !type_is_signed(it);
        emit(cg, "int64_t %s = nr_index_%s((%s)%s, %lld, %s);", t, is_unsigned ? "u64" : "i64",
             is_unsigned ? "uint64_t" : "int64_t", index, (long long)e->base->type->length, loc_str(e->loc));
        return fmt_str("%s.a[%s]", base, t);
    }
    case E_ALIAS: return gen_place(cg, e->alias->expr);
    default: return gen_expr(cg, e);
    }
}

/* An lvalue for an owned value used in place: the place itself, or the
 * temporary holding a new object. */
static const char *gen_lvalue(CG *cg, Expr *e) {
    return is_place(e) ? gen_place(cg, e) : gen_expr(cg, e);
}

/* Address of an owned value, for passing by reference. */
static const char *gen_ref(CG *cg, Expr *e) {
    const char *p = gen_lvalue(cg, e);
    if (strcmp(p, "(*nr_self)") == 0) return "nr_self";
    return fmt_str("&%s", p);
}

/* An owned value given to a new owner: the caller must consume() it. */
static const char *gen_owned_value(CG *cg, Expr *e) {
    const char *v = gen_expr(cg, e);
    consume(cg, v);
    return v;
}

static const char *gen_iface_arg(CG *cg, Expr *e) {
    if (e->kind == E_TO_IFACE) {
        const char *ref = gen_ref(cg, e->base);
        return fmt_str("((%s){(void *)%s, &%s})", ctype(e->type), ref, vtable_name(e->iface_impl_struct, e->type->decl));
    }
    return gen_expr(cg, e);
}

static const char *gen_arg(CG *cg, Expr *arg, Var *param, bool is_extern) {
    Type *param_type = param ? param->type : NULL;
    if (param_type && param_type->kind == TY_INTERFACE) return gen_iface_arg(cg, arg);
    if (is_extern && arg->type == type_string) {
        /* C receives the bytes of the string (spec 14) */
        if (arg->kind == E_STRING) return c_string_literal(arg->str, arg->str_len);
        return fmt_str("nr_cstr(%s)", gen_ref(cg, arg));
    }
    if (param_type && type_is_owned(param_type)) {
        if (param->is_ref) return gen_ref(cg, arg);
        return gen_owned_value(cg, arg); /* value parameter: the callee owns it */
    }
    const char *v = gen_expr(cg, arg);
    if (is_extern && arg->type == type_char) return fmt_str("(char)%s", v);
    return v;
}

/* Emits a call. Owned results become temporaries even when unused, so they
 * are destroyed if nothing takes them over. */
static const char *finish_call(CG *cg, Type *ret, const char *call, bool want_value) {
    if (ret == type_void || (!want_value && !(type_is_owned(ret) && needs_drop(ret)))) {
        emit(cg, "%s;", call);
        return "";
    }
    const char *t = new_tmp(cg);
    emit(cg, "%s %s = %s;", ctype(ret), t, call);
    add_temp(cg, t, ret);
    return t;
}

static const char *gen_print(CG *cg, Expr *e) {
    if (e->args.len == 1) {
        Expr *arg = e->args.data[0];
        Type *t = arg->type;
        if (t == type_string) {
            emit(cg, "nr_print_str(%s);", gen_ref(cg, arg));
        } else {
            const char *v = gen_expr(cg, arg);
            if (type_is_integer(t)) {
                if (type_is_signed(t)) emit(cg, "nr_print_i64((int64_t)%s);", v);
                else emit(cg, "nr_print_u64((uint64_t)%s);", v);
            } else if (t == type_f32) {
                emit(cg, "nr_print_f32(%s);", v);
            } else if (t == type_f64) {
                emit(cg, "nr_print_f64(%s);", v);
            } else if (t == type_bool) {
                emit(cg, "nr_print_bool(%s);", v);
            } else if (t == type_char) {
                emit(cg, "nr_print_char(%s);", v);
            } else if (t->kind == TY_ENUM) {
                emit(cg, "nr_print_str(&%s_names[%s]);", t->decl->cname, v);
            }
        }
    }
    if (strcmp(e->name, "println") == 0) emit(cg, "nr_print_newline();");
    return "";
}

/* Built-in String methods and String.of (spec 6.5). */
static const char *gen_string_call(CG *cg, Expr *e, bool want_value) {
    const char *name = e->name;
    if (!e->base) { /* String.of */
        Expr *arg = e->args.data[0];
        Type *t = arg->type;
        const char *v = gen_expr(cg, arg);
        const char *call;
        if (type_is_integer(t)) call = fmt_str(type_is_signed(t) ? "nr_str_of_i64((int64_t)%s)" : "nr_str_of_u64((uint64_t)%s)", v);
        else if (t == type_f32) call = fmt_str("nr_str_of_f32(%s)", v);
        else if (t == type_f64) call = fmt_str("nr_str_of_f64(%s)", v);
        else if (t == type_bool) call = fmt_str("nr_str_of_bool(%s)", v);
        else if (t == type_char) call = fmt_str("nr_str_of_char(%s)", v);
        else call = fmt_str("%s_names[%s]", t->decl->cname, v); /* enum: static name */
        return finish_call(cg, type_string, call, want_value);
    }
    const char *recv = gen_ref(cg, e->base);
    StrBuf args = {0};
    for (int i = 0; i < e->args.len; i++) {
        Expr *arg = e->args.data[i];
        if (arg->type == type_string) sb_printf(&args, ", %s", gen_ref(cg, arg));
        else if (type_is_integer(arg->type)) sb_printf(&args, ", (int64_t)%s", gen_expr(cg, arg));
        else sb_printf(&args, ", %s", gen_expr(cg, arg));
    }
    const char *loc = loc_str(e->loc);
    const char *a = sb_cstr(&args);
    if (strcmp(name, "length") == 0) return finish_call(cg, type_i32, fmt_str("nr_str_length(%s, %s)", recv, loc), want_value);
    if (strcmp(name, "charAt") == 0) return finish_call(cg, type_char, fmt_str("nr_str_char_at(%s%s, %s)", recv, a, loc), want_value);
    if (strcmp(name, "setCharAt") == 0) return finish_call(cg, type_void, fmt_str("nr_str_set_char_at(%s%s, %s)", recv, a, loc), want_value);
    if (strcmp(name, "append") == 0) return finish_call(cg, type_void, fmt_str("nr_str_append(%s%s)", recv, a), want_value);
    if (strcmp(name, "appendChar") == 0) return finish_call(cg, type_void, fmt_str("nr_str_append_char(%s%s)", recv, a), want_value);
    if (strcmp(name, "substring") == 0) return finish_call(cg, type_string, fmt_str("nr_str_substring(%s%s, %s)", recv, a, loc), want_value);
    if (strcmp(name, "indexOf") == 0) return finish_call(cg, type_i32, fmt_str("nr_str_index_of(%s%s)", recv, a), want_value);
    if (strcmp(name, "toInt") == 0) return finish_call(cg, type_i32, fmt_str("nr_str_to_i32(%s, %s)", recv, loc), want_value);
    if (strcmp(name, "toInt64") == 0) return finish_call(cg, type_i64, fmt_str("nr_str_to_i64(%s, %s)", recv, loc), want_value);
    return finish_call(cg, type_f64, fmt_str("nr_str_to_f64(%s, %s)", recv, loc), want_value);
}

static const char *gen_call(CG *cg, Expr *e, bool want_value) {
    StrBuf args = {0};
    Decl *fn = e->decl;
    switch (e->kind) {
    case E_CALL_FUNC: {
        bool is_extern = fn->kind == D_EXTERN_FUNC;
        for (int i = 0; i < e->args.len; i++) {
            Var *param = i < fn->params.len ? fn->params.data[i] : NULL;
            sb_printf(&args, "%s%s", i ? ", " : "", gen_arg(cg, e->args.data[i], param, is_extern));
        }
        const char *call = fmt_str("%s(%s)", fn->cname, sb_cstr(&args));
        if (is_extern && fn->ret_type == type_char) call = fmt_str("(uint8_t)%s", call);
        return finish_call(cg, fn->ret_type, call, want_value);
    }
    case E_CALL_METHOD: {
        const char *recv = gen_ref(cg, e->base);
        sb_append(&args, recv);
        for (int i = 0; i < e->args.len; i++) {
            sb_printf(&args, ", %s", gen_arg(cg, e->args.data[i], fn->params.data[i], false));
        }
        return finish_call(cg, fn->ret_type, fmt_str("%s(%s)", fn->cname, sb_cstr(&args)), want_value);
    }
    case E_CALL_IFACE: {
        const char *recv = gen_expr(cg, e->base);
        sb_printf(&args, "%s.self", recv);
        for (int i = 0; i < e->args.len; i++) {
            sb_printf(&args, ", %s", gen_arg(cg, e->args.data[i], fn->params.data[i], false));
        }
        return finish_call(cg, fn->ret_type, fmt_str("%s.vt->m_%s(%s)", recv, fn->name, sb_cstr(&args)), want_value);
    }
    case E_CALL_CTOR: {
        Decl *s = e->decl;
        if (!s->ctor) {
            const char *t = new_tmp(cg);
            emit(cg, "%s %s = %s;", ctype(s->type), t, zero_init(s->type));
            add_temp(cg, t, s->type);
            return t;
        }
        for (int i = 0; i < e->args.len; i++) {
            sb_printf(&args, "%s%s", i ? ", " : "", gen_arg(cg, e->args.data[i], s->ctor->params.data[i], false));
        }
        return finish_call(cg, s->type, fmt_str("%s(%s)", s->ctor->cname, args.len ? sb_cstr(&args) : ""), true);
    }
    case E_CALL_PRINT: return gen_print(cg, e);
    case E_CALL_STRING: return gen_string_call(cg, e, want_value);
    default: return "";
    }
}

static const char *gen_switch_cond(SwitchData *sw, SwitchArm *arm, const char *subject) {
    StrBuf sb = {0};
    for (int j = 0; j < arm->values.len; j++) {
        CaseValue *cv = &arm->values.data[j];
        if (j) sb_append(&sb, " || ");
        switch (sw->kind) {
        case SW_STRING:
            sb_printf(&sb, "nr_str_eq(%s, &(nr_String)%s)", subject, string_init(cv->svalue, cv->slen));
            break;
        case SW_ENUM: sb_printf(&sb, "%s == %lld", subject, (long long)cv->ivalue); break;
        case SW_BOOL: sb_printf(&sb, "%s == %s", subject, cv->uvalue ? "true" : "false"); break;
        case SW_CHAR: sb_printf(&sb, "%s == %llu", subject, (unsigned long long)cv->uvalue); break;
        case SW_INT:
            sb_printf(&sb, "%s == %s", subject, int_literal(sw->subject->type, cv->uvalue, cv->is_negative));
            break;
        }
    }
    return sb_cstr(&sb);
}

/* Evaluates the subject of a switch and selects an arm, then destroys the
 * temporaries of the subject. Returns the arms in dispatch order and the C
 * variable holding the selected arm's index. */
static const char *gen_switch_select(CG *cg, SwitchData *sw, SwitchArm ***arms_out, int *count_out) {
    int mark = cg->temps.len;
    const char *subject;
    if (sw->kind == SW_STRING) {
        subject = gen_ref(cg, sw->subject);
    } else {
        const char *v = gen_expr(cg, sw->subject);
        subject = new_tmp(cg);
        emit(cg, "%s %s = %s;", ctype(sw->subject->type), subject, v);
    }
    VEC(SwitchArm *) arms = {0};
    SwitchArm *default_arm = NULL;
    for (int i = 0; i < sw->arms.len; i++) {
        if (sw->arms.data[i].is_default) default_arm = &sw->arms.data[i];
        else vec_push(arms, &sw->arms.data[i]);
    }
    if (default_arm) vec_push(arms, default_arm);
    const char *selected = new_tmp(cg);
    emit(cg, "int %s = %d;", selected, arms.len - 1);
    /* the last arm is the default, or for an exhaustive switch the only
     * remaining case, so it needs no test */
    for (int i = 0; i < arms.len - 1; i++) {
        emit(cg, "%sif %s %s = %d;", i ? "else " : "", paren(gen_switch_cond(sw, arms.data[i], subject)), selected, i);
    }
    drop_temps(cg, mark);
    *arms_out = arms.data;
    *count_out = arms.len;
    return selected;
}

static const char *checked_op_name(int op) {
    switch (op) {
    case TOK_PLUS:
    case TOK_PLUS_ASSIGN:
    case TOK_PLUSPLUS: return "add";
    case TOK_MINUS:
    case TOK_MINUS_ASSIGN:
    case TOK_MINUSMINUS: return "sub";
    case TOK_STAR:
    case TOK_STAR_ASSIGN: return "mul";
    case TOK_SLASH:
    case TOK_SLASH_ASSIGN: return "div";
    case TOK_PERCENT:
    case TOK_PERCENT_ASSIGN: return "rem";
    case TOK_SHL:
    case TOK_SHL_ASSIGN: return "shl";
    case TOK_SHR:
    case TOK_SHR_ASSIGN: return "shr";
    default: return NULL;
    }
}

static const char *bitwise_c_op(int op) {
    switch (op) {
    case TOK_AMP:
    case TOK_AMP_ASSIGN: return "&";
    case TOK_PIPE:
    case TOK_PIPE_ASSIGN: return "|";
    case TOK_CARET:
    case TOK_CARET_ASSIGN: return "^";
    default: return NULL;
    }
}

static const char *shift_amount(const char *v, Type *t) {
    if (t == type_u64) return fmt_str("nr_shift_amount_u64(%s)", v);
    return fmt_str("(int64_t)%s", v);
}

/* Applies an arithmetic operator to two already-evaluated operands of type t
 * (the right operand of a shift may have any integer type rt). */
static const char *apply_arith(CG *cg, int op, Type *t, const char *l, const char *r, Type *rt, Loc loc) {
    const char *bit = bitwise_c_op(op);
    if (bit) return fmt_str("((%s)(%s %s %s))", ctype(t), l, bit, r);
    const char *name = checked_op_name(op);
    if (type_is_float(t)) {
        bool f32 = t == type_f32;
        switch (op) {
        case TOK_PLUS:
        case TOK_PLUS_ASSIGN:
        case TOK_PLUSPLUS: return fmt_str("(%s + %s)", l, r);
        case TOK_MINUS:
        case TOK_MINUS_ASSIGN:
        case TOK_MINUSMINUS: return fmt_str("(%s - %s)", l, r);
        case TOK_STAR:
        case TOK_STAR_ASSIGN: return fmt_str("(%s * %s)", l, r);
        case TOK_SLASH:
        case TOK_SLASH_ASSIGN: return fmt_str("(%s / %s)", l, r);
        default: return fmt_str("nr_fmod%s(%s, %s)", f32 ? "f" : "", l, r);
        }
    }
    const char *tmp = new_tmp(cg);
    if (op == TOK_SHL || op == TOK_SHR || op == TOK_SHL_ASSIGN || op == TOK_SHR_ASSIGN) {
        emit(cg, "%s %s = nr_%s_%s(%s, %s, %s);", ctype(t), tmp, name, type_suffix(t), l, shift_amount(r, rt),
             loc_str(loc));
    } else {
        emit(cg, "%s %s = nr_%s_%s(%s, %s, %s);", ctype(t), tmp, name, type_suffix(t), l, r, loc_str(loc));
    }
    return tmp;
}

static const char *gen_binary(CG *cg, Expr *e) {
    int op = e->op;
    if (op == TOK_ANDAND || op == TOK_OROR) {
        const char *l = gen_expr(cg, e->base);
        const char *t = new_tmp(cg);
        emit(cg, "bool %s = %s;", t, l);
        emit(cg, "if (%s%s) {", op == TOK_ANDAND ? "" : "!", t);
        cg->indent++;
        int mark = cg->temps.len;
        const char *r = gen_expr(cg, e->index);
        emit(cg, "%s = %s;", t, r);
        drop_temps(cg, mark);
        cg->indent--;
        emit(cg, "}");
        return t;
    }
    Type *lt = e->base->type;
    if (op == TOK_PLUS && e->type == type_string) {
        /* concatenation: a new String; the operands are used in place */
        const char *l = gen_ref(cg, e->base);
        const char *r = gen_ref(cg, e->index);
        const char *t = new_tmp(cg);
        emit(cg, "nr_String %s = nr_str_concat(%s, %s);", t, l, r);
        add_temp(cg, t, type_string);
        return t;
    }
    if (op == TOK_IS) {
        /* identity: both operands are references (spec 6.10) */
        if (lt->kind == TY_INTERFACE) {
            const char *l = gen_expr(cg, e->base);
            const char *r = gen_expr(cg, e->index);
            return fmt_str("(%s.self == %s.self)", l, r);
        }
        const char *l = gen_ref(cg, e->base);
        const char *r = gen_ref(cg, e->index);
        return fmt_str("((const void *)%s == (const void *)%s)", l, r);
    }
    if ((op == TOK_EQ || op == TOK_NE) && type_is_owned(lt)) {
        const char *l = gen_lvalue(cg, e->base);
        const char *r = gen_lvalue(cg, e->index);
        return fmt_str("(%s%s)", op == TOK_NE ? "!" : "", eq_expr(lt, l, r));
    }
    const char *l = gen_expr(cg, e->base);
    const char *r = gen_expr(cg, e->index);
    switch (op) {
    case TOK_EQ: return fmt_str("(%s == %s)", l, r);
    case TOK_NE: return fmt_str("(%s != %s)", l, r);
    case TOK_LT: return fmt_str("(%s < %s)", l, r);
    case TOK_LE: return fmt_str("(%s <= %s)", l, r);
    case TOK_GT: return fmt_str("(%s > %s)", l, r);
    case TOK_GE: return fmt_str("(%s >= %s)", l, r);
    default: return apply_arith(cg, op, e->type, l, r, e->index->type, e->loc);
    }
}

static const char *gen_conversion(CG *cg, Expr *e, Type *from, Type *to, const char *v) {
    if (from == to) return v;
    if (type_is_float(from) && (type_is_integer(to))) {
        const char *t = new_tmp(cg);
        emit(cg, "%s %s = nr_f2%s((double)%s, %s);", ctype(to), t, type_suffix(to), v, loc_str(e->loc));
        return t;
    }
    return fmt_str("((%s)%s)", ctype(to), v);
}

static const char *gen_switch_expr(CG *cg, Expr *e) {
    SwitchArm **arms;
    int count;
    const char *selected = gen_switch_select(cg, e->sw, &arms, &count);
    const char *result = new_tmp(cg);
    emit(cg, "%s %s = %s;", ctype(e->type), result, zero_init(e->type));
    for (int i = 0; i < count; i++) {
        emit(cg, i == 0 ? "if (%s == %d) {" : "else if (%s == %d) {", selected, i);
        cg->indent++;
        int mark = cg->temps.len;
        const char *v = type_is_owned(e->type) ? gen_owned_value(cg, arms[i]->expr) : gen_expr(cg, arms[i]->expr);
        emit(cg, "%s = %s;", result, v);
        drop_temps(cg, mark);
        cg->indent--;
        emit(cg, "}");
    }
    add_temp(cg, result, e->type);
    return result;
}

static const char *gen_expr_impl(CG *cg, Expr *e, bool want_value) {
    if (e->implicit_copy) {
        /* an existing object passed to a value parameter is copied (spec 7.3) */
        const char *p = gen_place(cg, e);
        const char *t = new_tmp(cg);
        emit(cg, "%s %s = %s;", ctype(e->type), t, copy_expr(e->type, p));
        add_temp(cg, t, e->type);
        return t;
    }
    switch (e->kind) {
    case E_INT: return int_literal(e->type, e->int_value, e->negative);
    case E_FLOAT: return float_literal(e->float_value, e->type == type_f32);
    case E_CHAR: return fmt_str("((uint8_t)%llu)", (unsigned long long)e->int_value);
    case E_BOOL: return e->bool_value ? "true" : "false";
    case E_STRING: {
        /* each evaluation is a new String object (spec 6.5) */
        const char *t = new_tmp(cg);
        emit(cg, "nr_String %s = nr_str_lit(%s, %zu);", t, c_string_literal(e->str, e->str_len), e->str_len);
        add_temp(cg, t, type_string);
        return t;
    }
    case E_VAR: {
        const char *v = var_ref(e->var);
        if (e->transfers_ownership && needs_drop(e->type)) {
            /* The new owner takes the object; the variable is left empty so
             * that its own cleanup does nothing (spec 7.1). */
            const char *t = new_tmp(cg);
            emit(cg, "%s %s = %s;", ctype(e->type), t, v);
            emit(cg, "%s = %s;", v, zero_value(e->type));
            add_temp(cg, t, e->type);
            return t;
        }
        return v;
    }
    case E_THIS: return "(*nr_self)";
    case E_FIELD:
    case E_INDEX: {
        const char *p = gen_place(cg, e);
        if (type_is_owned(e->type)) return p;
        const char *t = new_tmp(cg);
        emit(cg, "%s %s = %s;", ctype(e->type), t, p);
        return t;
    }
    case E_ALIAS: return gen_expr_impl(cg, e->alias->expr, want_value);
    case E_ENUM_VALUE: return fmt_str("%d", e->enum_index);
    case E_CONST: {
        if (e->type != type_string) return e->decl->cname;
        const char *t = new_tmp(cg);
        emit(cg, "nr_String %s = %s;", t, e->decl->cname);
        add_temp(cg, t, type_string);
        return t;
    }
    case E_LENGTH: {
        /* The length is constant, but the base is still evaluated (and its
         * indexes bounds-checked). */
        const char *base = gen_lvalue(cg, e->base);
        if (strchr(base, '[') || strchr(base, '(')) emit(cg, "(void)%s;", base);
        return fmt_str("%lld", (long long)e->base->type->length);
    }
    case E_EQUALS: {
        const char *a = gen_lvalue(cg, e->base);
        const char *b = gen_lvalue(cg, e->args.data[0]);
        return eq_expr(e->base->type, a, b);
    }
    case E_COPY: {
        const char *p = gen_lvalue(cg, e->base);
        const char *t = new_tmp(cg);
        emit(cg, "%s %s = %s;", ctype(e->type), t, copy_expr(e->type, p));
        add_temp(cg, t, e->type);
        return t;
    }
    case E_CALL_FUNC:
    case E_CALL_METHOD:
    case E_CALL_IFACE:
    case E_CALL_CTOR:
    case E_CALL_PRINT:
    case E_CALL_STRING: return gen_call(cg, e, want_value);
    case E_UNARY: {
        const char *v = gen_expr(cg, e->base);
        switch (e->op) {
        case TOK_BANG: return fmt_str("(!%s)", v);
        case TOK_TILDE: return fmt_str("((%s)~%s)", ctype(e->type), v);
        default:
            if (type_is_float(e->type)) return fmt_str("(-%s)", v);
            {
                const char *t = new_tmp(cg);
                emit(cg, "%s %s = nr_neg_%s(%s, %s);", ctype(e->type), t, type_suffix(e->type), v, loc_str(e->loc));
                return t;
            }
        }
    }
    case E_BINARY: return gen_binary(cg, e);
    case E_CAST:
    case E_CONVERT: {
        const char *v = gen_expr(cg, e->base);
        return gen_conversion(cg, e, e->base->type, e->type, v);
    }
    case E_SWITCH: return gen_switch_expr(cg, e);
    case E_TO_IFACE: return gen_iface_arg(cg, e);
    default: return "0";
    }
}

static const char *gen_expr(CG *cg, Expr *e) {
    return gen_expr_impl(cg, e, true);
}

/* A condition's value, safe to use after the temporaries since mark are
 * destroyed. */
static const char *gen_condition(CG *cg, Expr *e, int mark) {
    const char *c = gen_expr(cg, e);
    if (!temps_pending(cg, mark)) return c;
    const char *t = new_tmp(cg);
    emit(cg, "bool %s = %s;", t, c);
    drop_temps(cg, mark);
    return t;
}

/* ---------------------------------------------------------------------- */
/* Statements                                                              */

static void gen_array_init(CG *cg, const char *place, Expr *init) {
    for (int i = 0; i < init->args.len; i++) {
        Expr *el = init->args.data[i];
        const char *elem_place = fmt_str("%s.a[%d]", place, i);
        if (el->kind == E_ARRAY_INIT) {
            gen_array_init(cg, elem_place, el);
        } else {
            const char *v = type_is_owned(el->type) ? gen_owned_value(cg, el) : gen_expr(cg, el);
            emit(cg, "%s = %s;", elem_place, v);
        }
    }
}

/* Generates a statement as the body of a C block with its own scope. */
static void gen_scoped(CG *cg, Stmt *s) {
    cg->indent++;
    push_scope(cg);
    if (s->kind == S_BLOCK) {
        for (int i = 0; i < s->stmts.len; i++) gen_stmt(cg, s->stmts.data[i]);
    } else {
        gen_stmt(cg, s);
    }
    pop_scope(cg);
    cg->indent--;
}

static void gen_loop(CG *cg, Stmt *s) {
    LoopLabels labels = {++cg->label, false, false, 0, cg->loop};
    if (s->kind == S_FOR) {
        emit(cg, "{");
        cg->indent++;
        push_scope(cg);
        if (s->for_init) gen_stmt(cg, s->for_init);
    }
    labels.scope_mark = cg->drop_vars.len;
    cg->loop = &labels;
    emit(cg, "for (;;) {");
    cg->indent++;
    if (s->kind != S_DO && s->expr) {
        const char *c = gen_condition(cg, s->expr, cg->temps.len);
        emit(cg, "if (!%s) break;", c);
    }
    emit(cg, "{");
    gen_scoped(cg, s->body);
    emit(cg, "}");
    StrBuf *saved = cg->out;
    StrBuf tail = {0};
    cg->out = &tail;
    if (s->kind == S_FOR && s->for_update) gen_stmt(cg, s->for_update);
    if (s->kind == S_DO) {
        const char *c = gen_condition(cg, s->expr, cg->temps.len);
        emit(cg, "if (!%s) break;", c);
    }
    cg->out = saved;
    if (labels.cont_used) emit(cg, "nr_cont%d:;", labels.id);
    sb_append(cg->out, sb_cstr(&tail));
    cg->indent--;
    emit(cg, "}");
    if (labels.brk_used) emit(cg, "nr_brk%d:;", labels.id);
    cg->loop = labels.outer;
    if (s->kind == S_FOR) {
        pop_scope(cg);
        cg->indent--;
        emit(cg, "}");
    }
}

static void gen_assign(CG *cg, Stmt *s) {
    Expr *target = s->expr;
    Type *t = target->type;
    if (s->op == TOK_ASSIGN) {
        const char *place = gen_place(cg, target);
        const char *v = type_is_owned(t) ? gen_owned_value(cg, s->value) : gen_expr(cg, s->value);
        if (type_is_owned(t) && needs_drop(t)) emit(cg, "%s", drop_stmt(t, place)); /* replaced object (spec 7.5) */
        emit(cg, "%s = %s;", place, v);
        return;
    }
    const char *place = gen_place(cg, target);
    if (t == type_string) {
        /* a += b appends in place */
        const char *v = gen_ref(cg, s->value);
        emit(cg, "nr_str_append(&%s, %s);", place, v);
        return;
    }
    const char *old = new_tmp(cg);
    emit(cg, "%s %s = %s;", ctype(t), old, place);
    const char *v = gen_expr(cg, s->value);
    const char *result = apply_arith(cg, s->op, t, old, v, s->value->type, s->loc);
    emit(cg, "%s = %s;", place, result);
}

static void switch_stmt_body(CG *cg, SwitchData *sw) {
    SwitchArm **arms;
    int count;
    const char *selected = gen_switch_select(cg, sw, &arms, &count);
    for (int i = 0; i < count; i++) {
        emit(cg, i == 0 ? "if (%s == %d) {" : "else if (%s == %d) {", selected, i);
        gen_scoped(cg, arms[i]->body);
        emit(cg, "}");
    }
}

static void gen_stmt(CG *cg, Stmt *s) {
    int mark = cg->temps.len;
    switch (s->kind) {
    case S_BLOCK:
        emit(cg, "{");
        gen_scoped(cg, s);
        emit(cg, "}");
        return;
    case S_VAR: {
        Var *v = s->var;
        const char *name = fmt_str("nr_l_%s", v->name);
        if (!s->init) {
            emit(cg, "%s %s = %s;", ctype(v->type), name, zero_init(v->type));
        } else if (s->init->kind == E_ARRAY_INIT) {
            emit(cg, "%s %s = {0};", ctype(v->type), name);
            gen_array_init(cg, name, s->init);
        } else {
            const char *val = type_is_owned(v->type) ? gen_owned_value(cg, s->init) : gen_expr(cg, s->init);
            emit(cg, "%s %s = %s;", ctype(v->type), name, val);
        }
        drop_temps(cg, mark);
        add_drop_var(cg, name, v->type);
        return;
    }
    case S_ALIAS: return;
    case S_EXPR:
        gen_expr_impl(cg, s->expr, false);
        drop_temps(cg, mark);
        return;
    case S_ASSIGN:
        gen_assign(cg, s);
        drop_temps(cg, mark);
        return;
    case S_INCDEC: {
        Type *t = s->expr->type;
        const char *place = gen_place(cg, s->expr);
        const char *one = int_literal(t, 1, false);
        const char *old = new_tmp(cg);
        emit(cg, "%s %s = %s;", ctype(t), old, place);
        const char *r = apply_arith(cg, s->op, t, old, one, t, s->loc);
        emit(cg, "%s = %s;", place, r);
        drop_temps(cg, mark);
        return;
    }
    case S_IF: {
        const char *c = gen_condition(cg, s->expr, mark);
        emit(cg, "if %s {", paren(c));
        gen_scoped(cg, s->then_branch);
        if (s->else_branch) {
            emit(cg, "} else {");
            gen_scoped(cg, s->else_branch);
        }
        emit(cg, "}");
        return;
    }
    case S_WHILE:
    case S_DO:
    case S_FOR: gen_loop(cg, s); return;
    case S_SWITCH:
        emit(cg, "{");
        cg->indent++;
        switch_stmt_body(cg, s->sw);
        cg->indent--;
        emit(cg, "}");
        return;
    case S_BREAK:
        drop_vars_from(cg, cg->loop->scope_mark);
        cg->loop->brk_used = true;
        emit(cg, "goto nr_brk%d;", cg->loop->id);
        return;
    case S_CONTINUE:
        drop_vars_from(cg, cg->loop->scope_mark);
        cg->loop->cont_used = true;
        emit(cg, "goto nr_cont%d;", cg->loop->id);
        return;
    case S_RETURN:
        if (cg->fn->kind == D_CTOR) {
            drop_vars_from(cg, 0);
            emit(cg, "return nr_self_val;");
        } else if (s->expr) {
            Type *t = cg->fn->ret_type;
            const char *v = type_is_owned(t) ? gen_owned_value(cg, s->expr) : gen_expr(cg, s->expr);
            const char *r = new_tmp(cg);
            emit(cg, "%s %s = %s;", ctype(t), r, v);
            drop_temps(cg, mark);
            drop_vars_from(cg, 0);
            emit(cg, "return %s;", r);
        } else {
            drop_vars_from(cg, 0);
            emit(cg, "return;");
        }
        return;
    }
}

/* ---------------------------------------------------------------------- */
/* Declarations                                                            */

static const char *param_decl(Var *p) {
    Type *t = p->type;
    if (type_is_owned(t) && p->is_ref) return fmt_str("%s *nr_l_%s", ctype(t), p->name);
    return fmt_str("%s nr_l_%s", ctype(t), p->name);
}

static const char *function_signature(Decl *fn) {
    StrBuf params = {0};
    int count = 0;
    if (fn->kind == D_METHOD) {
        sb_printf(&params, "%s *nr_self", ctype(fn->owner->type));
        count++;
    }
    for (int i = 0; i < fn->params.len; i++) {
        sb_printf(&params, "%s%s", count ? ", " : "", param_decl(fn->params.data[i]));
        count++;
    }
    if (!count) sb_append(&params, "void");
    Type *ret = fn->kind == D_CTOR ? fn->owner->type : fn->ret_type;
    return fmt_str("%s %s(%s)", ctype(ret), fn->cname, sb_cstr(&params));
}

static const char *extern_c_type(Type *t) {
    if (t == type_string) return "const char *";
    if (t == type_char) return "char";
    return ctype(t);
}

static void emit_type_def(CG *cg, Type *t) {
    if (t->emitted) return;
    if (t->kind == TY_ARRAY) {
        t->emitted = true;
        if (type_is_owned(t->elem)) emit_type_def(cg, t->elem);
        emit(cg, "struct %s { %s a[%lld]; };", ctype(t), ctype(t->elem), (long long)t->length);
        return;
    }
    if (t->kind != TY_STRUCT) return;
    t->emitted = true;
    Decl *s = t->decl;
    for (int i = 0; i < s->members.len; i++) {
        Decl *m = s->members.data[i];
        if (m->kind == D_FIELD && type_is_owned(m->field_type)) emit_type_def(cg, m->field_type);
    }
    emit(cg, "struct %s {", ctype(t));
    cg->indent++;
    int fields = 0;
    for (int i = 0; i < s->members.len; i++) {
        Decl *m = s->members.data[i];
        if (m->kind != D_FIELD) continue;
        emit(cg, "%s %s;", ctype(m->field_type), field_name(m));
        fields++;
    }
    if (!fields) emit(cg, "char nr_empty;");
    cg->indent--;
    emit(cg, "};");
}

/* Cleanup, copy and equality functions for each struct and array type. */
static void emit_value_functions(CG *cg, Program *prog) {
    VEC(Type *) types = {0};
    for (int i = 0; i < prog->all_structs.len; i++) vec_push(types, prog->all_structs.data[i]->type);
    for (int i = 0; i < type_array_count(); i++) vec_push(types, type_array_at(i));

    for (int i = 0; i < types.len; i++) {
        Type *t = types.data[i];
        const char *c = ctype(t);
        if (needs_drop(t)) {
            emit(cg, "static void nr_drop_%s(%s *p);", c, c);
            emit(cg, "static %s nr_copy_%s(const %s *p);", c, c, c);
        }
        emit(cg, "static bool nr_eq_%s(const %s *a, const %s *b);", c, c, c);
    }
    emit(cg, "");

    for (int i = 0; i < types.len; i++) {
        Type *t = types.data[i];
        const char *c = ctype(t);
        bool drops = needs_drop(t);
        if (t->kind == TY_ARRAY) {
            Type *el = t->elem;
            if (drops) {
                emit(cg, "static void nr_drop_%s(%s *p) {", c, c);
                emit(cg, "    for (int64_t i = %lld; i >= 0; i--) %s", (long long)t->length - 1, drop_stmt(el, "p->a[i]"));
                emit(cg, "}");
                emit(cg, "static %s nr_copy_%s(const %s *p) {", c, c, c);
                emit(cg, "    %s r;", c);
                emit(cg, "    for (int64_t i = 0; i < %lld; i++) r.a[i] = %s;", (long long)t->length, copy_expr(el, "p->a[i]"));
                emit(cg, "    return r;");
                emit(cg, "}");
            }
            emit(cg, "static bool nr_eq_%s(const %s *a, const %s *b) {", c, c, c);
            emit(cg, "    for (int64_t i = 0; i < %lld; i++) {", (long long)t->length);
            emit(cg, "        if (!%s) return false;", paren(eq_expr(el, "a->a[i]", "b->a[i]")));
            emit(cg, "    }");
            emit(cg, "    return true;");
            emit(cg, "}");
            continue;
        }
        Decl *s = t->decl;
        if (drops) {
            emit(cg, "static void nr_drop_%s(%s *p) {", c, c);
            for (int j = s->members.len - 1; j >= 0; j--) {
                Decl *m = s->members.data[j];
                if (m->kind == D_FIELD && needs_drop(m->field_type)) {
                    emit(cg, "    %s", drop_stmt(m->field_type, fmt_str("p->%s", field_name(m))));
                }
            }
            emit(cg, "}");
            emit(cg, "static %s nr_copy_%s(const %s *p) {", c, c, c);
            emit(cg, "    %s r = *p;", c);
            for (int j = 0; j < s->members.len; j++) {
                Decl *m = s->members.data[j];
                if (m->kind == D_FIELD && needs_drop(m->field_type)) {
                    const char *f = field_name(m);
                    emit(cg, "    r.%s = %s;", f, copy_expr(m->field_type, fmt_str("p->%s", f)));
                }
            }
            emit(cg, "    return r;");
            emit(cg, "}");
        }
        emit(cg, "static bool nr_eq_%s(const %s *a, const %s *b) {", c, c, c);
        for (int j = 0; j < s->members.len; j++) {
            Decl *m = s->members.data[j];
            if (m->kind != D_FIELD) continue;
            const char *f = field_name(m);
            emit(cg, "    if (!%s) return false;",
                 paren(eq_expr(m->field_type, fmt_str("a->%s", f), fmt_str("b->%s", f))));
        }
        emit(cg, "    return true;");
        emit(cg, "}");
    }
    emit(cg, "");
}

static void gen_function(CG *cg, Decl *fn) {
    cg->fn = fn;
    cg->tmp = 0;
    cg->loop = NULL;
    cg->temps.len = 0;
    cg->drop_vars.len = 0;
    cg->scope_marks.len = 0;
    emit(cg, "%s {", function_signature(fn));
    cg->indent++;
    if (fn->kind == D_CTOR) {
        emit(cg, "%s nr_self_val = {0};", ctype(fn->owner->type));
        emit(cg, "%s *nr_self = &nr_self_val;", ctype(fn->owner->type));
    }
    push_scope(cg);
    for (int i = 0; i < fn->params.len; i++) {
        Var *p = fn->params.data[i];
        if (!p->is_ref) add_drop_var(cg, fmt_str("nr_l_%s", p->name), p->type); /* value parameters are owned */
    }
    for (int i = 0; i < fn->body->stmts.len; i++) gen_stmt(cg, fn->body->stmts.data[i]);
    pop_scope(cg);
    if (fn->kind == D_CTOR) {
        emit(cg, "return nr_self_val;");
    } else if (fn->ret_type != type_void) {
        emit(cg, "nr_unreachable();");
    }
    cg->indent--;
    emit(cg, "}");
    emit(cg, "");
}

char *codegen_program(Program *prog) {
    StrBuf out = {0};
    CG cg = {0};
    cg.prog = prog;
    cg.out = &out;
    assign_names(prog);

    emit(&cg, "/* Generated by the Nirea compiler. Do not edit. */");
    emit(&cg, "#include \"nirea_rt.h\"");
    emit(&cg, "");
    /* Unused locals and declarations mirror the Nirea source; don't warn. */
    emit(&cg, "#if defined(__clang__)");
    emit(&cg, "#pragma clang diagnostic ignored \"-Wunknown-warning-option\"");
    emit(&cg, "#endif");
    emit(&cg, "#if defined(__GNUC__)");
    emit(&cg, "#pragma GCC diagnostic ignored \"-Wunused-variable\"");
    emit(&cg, "#pragma GCC diagnostic ignored \"-Wunused-but-set-variable\"");
    emit(&cg, "#pragma GCC diagnostic ignored \"-Wunused-const-variable\"");
    emit(&cg, "#pragma GCC diagnostic ignored \"-Wunused-function\"");
    emit(&cg, "#pragma GCC diagnostic ignored \"-Wformat-security\"");
    emit(&cg, "#endif");
    emit(&cg, "");

    /* Forward declarations of all aggregate types. */
    for (int i = 0; i < prog->all_structs.len; i++) {
        Type *t = prog->all_structs.data[i]->type;
        emit(&cg, "typedef struct %s %s;", ctype(t), ctype(t));
    }
    for (int i = 0; i < type_array_count(); i++) {
        Type *t = type_array_at(i);
        emit(&cg, "typedef struct %s %s;", ctype(t), ctype(t));
    }
    for (int i = 0; i < prog->all_interfaces.len; i++) {
        Decl *d = prog->all_interfaces.data[i];
        emit(&cg, "typedef struct %s_vt %s_vt;", d->cname, d->cname);
        emit(&cg, "typedef struct %s { void *self; const %s_vt *vt; } %s;", d->cname, d->cname, d->cname);
    }
    emit(&cg, "");

    for (int i = 0; i < prog->all_structs.len; i++) emit_type_def(&cg, prog->all_structs.data[i]->type);
    for (int i = 0; i < type_array_count(); i++) emit_type_def(&cg, type_array_at(i));
    emit(&cg, "");

    /* Interface vtables. */
    for (int i = 0; i < prog->all_interfaces.len; i++) {
        Decl *d = prog->all_interfaces.data[i];
        emit(&cg, "struct %s_vt {", d->cname);
        cg.indent++;
        for (int j = 0; j < d->members.len; j++) {
            Decl *m = d->members.data[j];
            StrBuf params = {0};
            sb_append(&params, "void *nr_self");
            for (int k = 0; k < m->params.len; k++) sb_printf(&params, ", %s", param_decl(m->params.data[k]));
            emit(&cg, "%s (*m_%s)(%s);", ctype(m->ret_type), m->name, sb_cstr(&params));
        }
        if (!d->members.len) emit(&cg, "char nr_empty;");
        cg.indent--;
        emit(&cg, "};");
    }
    emit(&cg, "");

    emit_value_functions(&cg, prog);

    /* Enum names. */
    for (int i = 0; i < prog->all_enums.len; i++) {
        Decl *d = prog->all_enums.data[i];
        StrBuf names = {0};
        for (int j = 0; j < d->values.len; j++) {
            const char *n = d->values.data[j].name;
            sb_printf(&names, "%s%s", j ? ", " : "", string_init(n, strlen(n)));
        }
        emit(&cg, "const nr_String %s_names[] = {%s};", d->cname, sb_cstr(&names));
    }

    /* Constants. */
    for (int i = 0; i < prog->all_consts.len; i++) {
        Decl *c = prog->all_consts.data[i];
        Expr *v = c->const_value;
        while (v->kind == E_CONVERT) v = v->base;
        const char *value;
        Type *t = c->const_type;
        switch (v->kind) {
        case E_INT: value = int_literal(t, v->int_value, v->negative); break;
        case E_FLOAT: value = float_literal(v->float_value, t == type_f32); break;
        case E_CHAR: value = fmt_str("%llu", (unsigned long long)v->int_value); break;
        case E_BOOL: value = v->bool_value ? "true" : "false"; break;
        case E_STRING: value = string_init(v->str, v->str_len); break;
        case E_ENUM_VALUE: value = fmt_str("%d", v->enum_index); break;
        default: value = "0"; break;
        }
        emit(&cg, "static const %s %s = %s;", ctype(t), c->cname, value);
    }
    emit(&cg, "");

    /* Prototypes. */
    for (int i = 0; i < prog->all_functions.len; i++) emit(&cg, "%s;", function_signature(prog->all_functions.data[i]));
    Map extern_seen = {0};
    for (int i = 0; i < prog->all_externs.len; i++) {
        Decl *d = prog->all_externs.data[i];
        if (map_get(&extern_seen, d->name)) continue;
        map_put(&extern_seen, d->name, d);
        StrBuf params = {0};
        for (int k = 0; k < d->params.len; k++) {
            sb_printf(&params, "%s%s", k ? ", " : "", extern_c_type(d->params.data[k]->type));
        }
        if (d->variadic) sb_append(&params, d->params.len ? ", ..." : "...");
        if (!d->params.len && !d->variadic) sb_append(&params, "void");
        emit(&cg, "extern %s %s(%s);", extern_c_type(d->ret_type), d->name, sb_cstr(&params));
    }
    emit(&cg, "");

    /* Interface thunks and vtable instances. */
    for (int i = 0; i < prog->all_structs.len; i++) {
        Decl *s = prog->all_structs.data[i];
        for (int j = 0; j < s->implements.len; j++) {
            Decl *iface = s->implements.data[j].decl;
            if (!iface) continue;
            StrBuf init = {0};
            for (int k = 0; k < iface->members.len; k++) {
                Decl *im = iface->members.data[k];
                Decl *m = map_get(&s->member_map, im->name);
                const char *thunk = fmt_str("nr_thunk_%d_%d_%s", s->id, iface->id, im->name);
                StrBuf params = {0}, args = {0};
                sb_append(&params, "void *nr_self");
                sb_printf(&args, "(%s *)nr_self", ctype(s->type));
                for (int p = 0; p < im->params.len; p++) {
                    sb_printf(&params, ", %s", param_decl(im->params.data[p]));
                    sb_printf(&args, ", nr_l_%s", im->params.data[p]->name);
                }
                emit(&cg, "static %s %s(%s) {", ctype(im->ret_type), thunk, sb_cstr(&params));
                emit(&cg, "    %s%s(%s);", im->ret_type == type_void ? "" : "return ", m->cname, sb_cstr(&args));
                emit(&cg, "}");
                sb_printf(&init, "%s%s", k ? ", " : "", thunk);
            }
            if (!iface->members.len) sb_append(&init, "0");
            emit(&cg, "static const %s_vt %s = {%s};", iface->cname, vtable_name(s, iface), sb_cstr(&init));
        }
    }
    emit(&cg, "");

    for (int i = 0; i < prog->all_functions.len; i++) gen_function(&cg, prog->all_functions.data[i]);

    emit(&cg, "int main(int argc, char **argv) {");
    emit(&cg, "    (void)argc;");
    emit(&cg, "    (void)argv;");
    if (prog->main->ret_type == type_void) {
        emit(&cg, "    %s();", prog->main->cname);
        emit(&cg, "    return 0;");
    } else {
        emit(&cg, "    return (int)%s();", prog->main->cname);
    }
    emit(&cg, "}");
    return out.data;
}
