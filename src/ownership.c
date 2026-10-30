/* Flow-sensitive ownership-transfer checking (spec 7.1). Semantic analysis
 * has already marked every local variable whose object goes to a new owner
 * (Expr.transfers_ownership); this pass follows which owned locals have given
 * their object away on each path and rejects later uses of them. */
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "sema.h"

enum { OWNS = 0, GAVE_AWAY = 1, MAYBE_GAVE_AWAY = 2 };

typedef struct State {
    uint8_t *v;
    bool reachable;
} State;

typedef struct LoopCtx {
    State brk;
    State cont;
    struct LoopCtx *outer;
} LoopCtx;

typedef struct OCtx {
    Decl *fn;
    int n;
    LoopCtx *loop;
    Loc *transfer_loc;
    Expr *alias_use; /* outermost alias being expanded, for error locations */
} OCtx;

static State state_new(OCtx *c, bool reachable) {
    State s;
    s.v = xcalloc((size_t)(c->n ? c->n : 1), 1);
    s.reachable = reachable;
    return s;
}

static State state_copy(OCtx *c, State *src) {
    State s = state_new(c, src->reachable);
    memcpy(s.v, src->v, (size_t)c->n);
    return s;
}

static void state_join(OCtx *c, State *dst, State *src) {
    if (!src->reachable) return;
    if (!dst->reachable) {
        memcpy(dst->v, src->v, (size_t)c->n);
        dst->reachable = true;
        return;
    }
    for (int i = 0; i < c->n; i++) {
        if (dst->v[i] != src->v[i]) dst->v[i] = MAYBE_GAVE_AWAY;
    }
}

static bool state_equal(OCtx *c, State *a, State *b) {
    if (a->reachable != b->reachable) return false;
    if (!a->reachable) return true;
    return memcmp(a->v, b->v, (size_t)c->n) == 0;
}

/* Owned locals, including value parameters (spec 7.3). */
static bool tracked(Var *v) {
    return (v->kind == VAR_LOCAL || !v->is_ref) && v->type && type_is_owned(v->type);
}

/* The local variable whose storage an expression refers to, if any. */
static Var *root_var(Expr *e) {
    switch (e->kind) {
    case E_VAR: return e->var;
    case E_FIELD:
    case E_INDEX:
    case E_TO_IFACE: return root_var(e->base);
    case E_ALIAS: return root_var(e->alias->expr);
    default: return NULL;
    }
}

static Var *call_param(Expr *call, int i) {
    Decl *fn = call->decl;
    if (call->kind == E_CALL_CTOR) fn = fn->ctor;
    if (!fn || i >= fn->params.len) return NULL;
    return fn->params.data[i];
}

/* Reports a local that a call or assignment refers to (as receiver,
 * reference argument or assignment target) but that gave its object to a new
 * owner while the call's arguments or the assigned value were evaluated
 * (spec 7.3). before is the state just after the reference was taken. */
static void check_still_owned(OCtx *c, Var *v, State *before, State *after, Loc fallback, const char *what) {
    if (!v || !tracked(v) || !before->reachable || !after->reachable) return;
    if (before->v[v->index] != OWNS || after->v[v->index] == OWNS) return;
    Loc loc = c->transfer_loc[v->index].file ? c->transfer_loc[v->index] : fallback;
    error_at(loc, "'%s' can't be given to a new owner while %s refers to it", v->name, what);
}

static Var *call_reference_root(Expr *call, int i) {
    if (call->kind == E_CALL_STRING) {
        /* String arguments of built-in methods are references */
        Expr *arg = call->args.data[i];
        return arg->type == type_string ? root_var(arg) : NULL;
    }
    Var *p = call_param(call, i);
    if (!p || !(p->is_ref || p->type->kind == TY_INTERFACE)) return NULL;
    return root_var(call->args.data[i]);
}

/* True if every value e can produce is a new object (a call result), never
 * an existing object that would have to be copied. */
static bool is_new_object(Expr *e) {
    switch (e->kind) {
    case E_CALL_FUNC:
    case E_CALL_METHOD:
    case E_CALL_IFACE:
    case E_CALL_CTOR:
    case E_CALL_STRING:
    case E_COPY:
    case E_STRING:
    case E_CONST: return true;
    case E_SWITCH:
        for (int i = 0; i < e->sw->arms.len; i++) {
            if (!is_new_object(e->sw->arms.data[i].expr)) return false;
        }
        return true;
    default: return false;
    }
}

/* A switch expression of an owned type outside a transfer position would act
 * on a hidden copy of an existing object (spec 10.1). */
static void check_switch_result(Expr *e) {
    if (e->transfers_ownership || !e->type || !type_is_owned(e->type)) return;
    for (int i = 0; i < e->sw->arms.len; i++) {
        Expr *arm = e->sw->arms.data[i].expr;
        if (arm->kind == E_SWITCH) continue; /* checked when visited */
        if (!is_new_object(arm)) {
            error_at(arm->loc,
                     "this switch branch refers to an existing %s, which the switch would copy implicitly; use the "
                     "switch where its result gets an owner (e.g. %s value = switch ...), or call .copy()",
                     type_name(e->type), type_name(e->type));
        }
    }
}

static void visit_expr(OCtx *c, Expr *e, State *s);

static void use_var(OCtx *c, Expr *e, State *s) {
    Var *v = e->var;
    if (!tracked(v) || !s->reachable) return;
    uint8_t st = s->v[v->index];
    if (st != OWNS) {
        if (error_suppress == 0) {
            Loc loc = c->alias_use ? c->alias_use->loc : e->loc;
            const char *via = c->alias_use ? fmt_str(" (through alias '%s')", c->alias_use->alias->name) : "";
            if (st == GAVE_AWAY) {
                error_at(loc, "'%s' can't be used here: its object was given to a new owner%s", v->name, via);
            } else {
                error_at(loc, "'%s' can't be used here: its object may have been given to a new owner on some path%s",
                         v->name, via);
            }
            if (c->transfer_loc[v->index].file) note_at(c->transfer_loc[v->index], "ownership of '%s' was transferred here", v->name);
            s->v[v->index] = OWNS; /* report each use only once */
        }
        return;
    }
    if (e->transfers_ownership) {
        s->v[v->index] = GAVE_AWAY;
        if (error_suppress == 0 || !c->transfer_loc[v->index].file) c->transfer_loc[v->index] = e->loc;
    }
}

static void visit_switch_expr(OCtx *c, SwitchData *sw, State *s) {
    visit_expr(c, sw->subject, s);
    State result = state_new(c, false);
    for (int i = 0; i < sw->arms.len; i++) {
        State arm = state_copy(c, s);
        visit_expr(c, sw->arms.data[i].expr, &arm);
        state_join(c, &result, &arm);
        free(arm.v);
    }
    free(s->v);
    *s = result;
}

static void visit_expr(OCtx *c, Expr *e, State *s) {
    if (!e) return;
    switch (e->kind) {
    case E_VAR: use_var(c, e, s); return;
    case E_ALIAS: {
        Expr *outer = c->alias_use;
        if (!outer) c->alias_use = e;
        visit_expr(c, e->alias->expr, s);
        c->alias_use = outer;
        return;
    }
    case E_FIELD:
    case E_LENGTH:
    case E_COPY:
    case E_CONVERT:
    case E_TO_IFACE:
    case E_CAST:
    case E_UNARY: visit_expr(c, e->base, s); return;
    case E_BINARY:
        visit_expr(c, e->base, s);
        if (e->op == TOK_ANDAND || e->op == TOK_OROR) {
            /* the right operand runs only on some paths */
            State right = state_copy(c, s);
            visit_expr(c, e->index, &right);
            state_join(c, s, &right);
            free(right.v);
        } else {
            visit_expr(c, e->index, s);
        }
        return;
    case E_INDEX:
        visit_expr(c, e->base, s);
        visit_expr(c, e->index, s);
        return;
    case E_CALL_METHOD:
    case E_CALL_IFACE:
    case E_CALL_FUNC:
    case E_CALL_CTOR:
    case E_CALL_STRING: {
        bool has_receiver = e->kind == E_CALL_METHOD || e->kind == E_CALL_IFACE || (e->kind == E_CALL_STRING && e->base);
        if (has_receiver) visit_expr(c, e->base, s);
        State after_receiver = state_copy(c, s);
        /* Each referenced argument must still own its object once all
         * arguments have been evaluated. */
        State *before_arg = xcalloc((size_t)(e->args.len ? e->args.len : 1), sizeof(State));
        for (int i = 0; i < e->args.len; i++) {
            visit_expr(c, e->args.data[i], s);
            before_arg[i] = state_copy(c, s);
        }
        if (has_receiver) check_still_owned(c, root_var(e->base), &after_receiver, s, e->loc, "this call");
        for (int i = 0; i < e->args.len; i++) {
            check_still_owned(c, call_reference_root(e, i), &before_arg[i], s, e->loc, "this call");
            free(before_arg[i].v);
        }
        free(before_arg);
        free(after_receiver.v);
        return;
    }
    case E_CALL_PRINT:
    case E_ARRAY_INIT:
        for (int i = 0; i < e->args.len; i++) visit_expr(c, e->args.data[i], s);
        return;
    case E_SWITCH:
        check_switch_result(e);
        visit_switch_expr(c, e->sw, s);
        return;
    default: return;
    }
}

static void visit_stmt(OCtx *c, Stmt *st, State *s);

static void visit_loop(OCtx *c, Stmt *st, State *s) {
    State entry = state_copy(c, s);
    State head = state_copy(c, s);
    State exit_state = state_new(c, false);
    bool infinite = (st->kind == S_FOR && !st->expr) || (st->expr && st->expr->kind == E_BOOL && st->expr->bool_value);

    for (int pass = 0;; pass++) {
        bool final = pass >= 10;
        if (!final) error_suppress++;

        LoopCtx lc = {state_new(c, false), state_new(c, false), c->loop};
        c->loop = &lc;
        State cur = state_copy(c, &head);
        State exit_now = state_new(c, false);

        if (st->kind == S_DO) {
            visit_stmt(c, st->body, &cur);
            state_join(c, &cur, &lc.cont);
            visit_expr(c, st->expr, &cur);
            if (!infinite) state_join(c, &exit_now, &cur);
        } else {
            if (st->expr) visit_expr(c, st->expr, &cur);
            if (!infinite) state_join(c, &exit_now, &cur);
            visit_stmt(c, st->body, &cur);
            state_join(c, &cur, &lc.cont);
            if (st->kind == S_FOR && st->for_update) visit_stmt(c, st->for_update, &cur);
        }
        state_join(c, &exit_now, &lc.brk);
        c->loop = lc.outer;

        if (!final) error_suppress--;

        if (final) {
            free(exit_state.v);
            exit_state = exit_now;
            free(cur.v);
            break;
        }
        State next_head = state_copy(c, &entry);
        state_join(c, &next_head, &cur);
        free(cur.v);
        free(exit_now.v);
        free(lc.brk.v);
        free(lc.cont.v);
        if (state_equal(c, &next_head, &head)) {
            free(next_head.v);
            pass = 9; /* run the final, reporting pass from the fixed point */
            continue;
        }
        free(head.v);
        head = next_head;
    }
    free(entry.v);
    free(head.v);
    free(s->v);
    *s = exit_state;
}

static bool assigns_whole_var(Expr *target, Var **out) {
    while (target->kind == E_ALIAS) target = target->alias->expr;
    if (target->kind == E_VAR) {
        *out = target->var;
        return true;
    }
    return false;
}

static void visit_stmt(OCtx *c, Stmt *st, State *s) {
    if (!st || !s->reachable) return;
    switch (st->kind) {
    case S_BLOCK:
        for (int i = 0; i < st->stmts.len; i++) visit_stmt(c, st->stmts.data[i], s);
        return;
    case S_VAR:
        if (st->init) visit_expr(c, st->init, s);
        if (s->reachable && tracked(st->var)) s->v[st->var->index] = OWNS;
        return;
    case S_ALIAS: return;
    case S_EXPR: visit_expr(c, st->expr, s); return;
    case S_ASSIGN: {
        Var *whole = NULL;
        if (st->op == TOK_ASSIGN && assigns_whole_var(st->expr, &whole)) {
            visit_expr(c, st->value, s);
            if (s->reachable && tracked(whole)) s->v[whole->index] = OWNS;
            return;
        }
        visit_expr(c, st->expr, s);
        State after_target = state_copy(c, s);
        visit_expr(c, st->value, s);
        check_still_owned(c, root_var(st->expr), &after_target, s, st->loc, "this assignment");
        free(after_target.v);
        return;
    }
    case S_INCDEC: visit_expr(c, st->expr, s); return;
    case S_IF: {
        visit_expr(c, st->expr, s);
        State other = state_copy(c, s);
        visit_stmt(c, st->then_branch, s);
        if (st->else_branch) visit_stmt(c, st->else_branch, &other);
        state_join(c, s, &other);
        free(other.v);
        return;
    }
    case S_WHILE:
    case S_DO: visit_loop(c, st, s); return;
    case S_FOR:
        if (st->for_init) visit_stmt(c, st->for_init, s);
        visit_loop(c, st, s);
        return;
    case S_SWITCH: {
        visit_expr(c, st->sw->subject, s);
        State result = state_new(c, false);
        for (int i = 0; i < st->sw->arms.len; i++) {
            State arm = state_copy(c, s);
            visit_stmt(c, st->sw->arms.data[i].body, &arm);
            state_join(c, &result, &arm);
            free(arm.v);
        }
        free(s->v);
        *s = result;
        return;
    }
    case S_BREAK:
        if (c->loop) state_join(c, &c->loop->brk, s);
        s->reachable = false;
        return;
    case S_CONTINUE:
        if (c->loop) state_join(c, &c->loop->cont, s);
        s->reachable = false;
        return;
    case S_RETURN:
        if (st->expr) visit_expr(c, st->expr, s);
        s->reachable = false;
        return;
    }
}

static void check_function(Decl *fn) {
    OCtx c = {0};
    c.fn = fn;
    c.n = fn->locals.len;
    c.transfer_loc = xcalloc((size_t)(c.n ? c.n : 1), sizeof(Loc));
    State s = state_new(&c, true);
    visit_stmt(&c, fn->body, &s);
    free(s.v);
    free(c.transfer_loc);
}

void ownership_check_program(Program *prog) {
    for (int i = 0; i < prog->all_functions.len; i++) check_function(prog->all_functions.data[i]);
}
