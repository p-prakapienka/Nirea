/* Flow-sensitive ownership and reference checking (spec 7.1, 9.3). Semantic
 * analysis has already marked every local variable whose object goes to a new
 * owner (Expr.transfers_ownership). On each path, this pass follows which
 * owned locals have given their object away and which reference variables
 * have ended, and rejects later uses of them. It also rejects calls that reach
 * the same object in two ways.
 *
 * Objects are compared as paths from a root (a local variable, a parameter or
 * `this`) through fields and array elements. Two objects overlap when one
 * path is a prefix of the other, treating an index computed at run time as
 * any index. */
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "sema.h"

/* Per-variable state. For owned locals: owns its object, gave it away, or
 * gave it away on some paths. For reference variables: usable, ended, or
 * ended on some paths. */
enum { USABLE = 0, GONE = 1, MAYBE_GONE = 2 };

typedef struct State {
    uint8_t *v;
    bool reachable;
} State;

typedef struct LoopCtx {
    State brk;
    State cont;
    struct LoopCtx *outer;
} LoopCtx;

/* A step from an object to a part of it: a field, or an array element with
 * a constant index (index >= 0) or an index computed at run time. */
typedef struct Step {
    Decl *field;
    int64_t index;
} Step;

typedef struct Path {
    Var *root; /* NULL for `this` */
    Step *steps;
    int len;
    const char *text; /* for messages */
} Path;

typedef VEC(Path) PathVec;

typedef struct OCtx {
    Decl *fn;
    int n;
    LoopCtx *loop;
    Loc *transfer_loc;
    Loc *end_loc;            /* where each reference variable ended */
    const char **end_reason; /* why it ended */
    PathVec *targets;        /* objects each reference variable reaches */
    VarVec refs;             /* the function's reference variables */
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
        if (dst->v[i] != src->v[i]) dst->v[i] = MAYBE_GONE;
    }
}

static bool state_equal(OCtx *c, State *a, State *b) {
    if (a->reachable != b->reachable) return false;
    if (!a->reachable) return true;
    return memcmp(a->v, b->v, (size_t)c->n) == 0;
}

/* Owned locals, including value parameters (spec 7.3). */
static bool tracked(Var *v) {
    return !v->is_ref && v->type && type_is_owned(v->type);
}

static bool is_ref_var(Var *v) {
    return v->is_ref && v->kind == VAR_LOCAL;
}

/* The local variable whose storage an expression refers to, if any. */
static Var *root_var(Expr *e) {
    switch (e->kind) {
    case E_VAR: return e->var;
    case E_FIELD:
    case E_INDEX:
    case E_TO_IFACE: return root_var(e->base);
    default: return NULL;
    }
}

static Var *call_param(Expr *call, int i) {
    Decl *fn = call->decl;
    if (call->kind == E_CALL_CTOR) fn = fn->ctor;
    if (!fn || i >= fn->params.len) return NULL;
    return fn->params.data[i];
}

/* True if argument i of a call is used in place rather than copied. */
static bool passed_by_reference(Expr *call, int i) {
    if (call->kind == E_EQUALS) return true;
    if (call->kind == E_CALL_STRING) return call->args.data[i]->type == type_string; /* built-in String methods */
    if (call->kind == E_CALL_PRINT) return false;
    Var *p = call_param(call, i);
    return p && (p->is_ref || p->type->kind == TY_INTERFACE);
}

/* True if a call uses the object it is called on in place. */
static bool has_receiver(Expr *call) {
    return call->kind == E_CALL_METHOD || call->kind == E_CALL_IFACE || call->kind == E_EQUALS ||
           (call->kind == E_CALL_STRING && call->base);
}

/* ---- Paths ---- */

static Path path_root(Var *v) {
    Path p = {v, NULL, 0, v ? v->name : "this"};
    return p;
}

static Path path_extend(Path *base, Step step, const char *suffix) {
    Path p = {base->root, xcalloc((size_t)base->len + 1, sizeof(Step)), base->len + 1, NULL};
    if (base->len) memcpy(p.steps, base->steps, (size_t)base->len * sizeof(Step));
    p.steps[base->len] = step;
    p.text = fmt_str("%s%s", base->text, suffix);
    return p;
}

static bool paths_overlap(Path *a, Path *b) {
    if (a->root != b->root) return false;
    int n = a->len < b->len ? a->len : b->len;
    for (int i = 0; i < n; i++) {
        Step *x = &a->steps[i], *y = &b->steps[i];
        if (x->field != y->field) return false;
        if (!x->field && x->index >= 0 && y->index >= 0 && x->index != y->index) return false;
    }
    return true;
}

static bool any_overlap(PathVec *a, PathVec *b, int *ia, int *ib) {
    for (int i = 0; i < a->len; i++) {
        for (int j = 0; j < b->len; j++) {
            if (paths_overlap(&a->data[i], &b->data[j])) {
                if (ia) *ia = i;
                if (ib) *ib = j;
                return true;
            }
        }
    }
    return false;
}

/* The constant value of an array index, or -1 if it is computed at run time. */
static int64_t constant_index(Expr *e) {
    while (e->kind == E_CONVERT) e = e->base;
    if (e->kind == E_CONST && e->decl->const_value) {
        e = e->decl->const_value;
        while (e->kind == E_CONVERT) e = e->base;
    }
    if (e->kind == E_INT && !e->negative && e->int_value <= INT64_MAX) return (int64_t)e->int_value;
    return -1;
}

static const char *index_text(Expr *e, int64_t index) {
    if (index >= 0) return fmt_str("[%lld]", (long long)index);
    if (e->kind == E_VAR) return fmt_str("[%s]", e->var->name);
    return "[...]";
}

/* Adds the objects e refers to, if e is a place rather than a temporary.
 * Reference variables are replaced by the objects they reach, and a call
 * that returns a reference reaches the object it is called on and its
 * reference arguments, each as a whole (spec 9.2). */
static bool place_paths(OCtx *c, Expr *e, PathVec *out) {
    switch (e->kind) {
    case E_VAR:
        if (is_ref_var(e->var)) {
            PathVec *t = &c->targets[e->var->index];
            for (int i = 0; i < t->len; i++) vec_push(*out, t->data[i]);
        } else {
            vec_push(*out, path_root(e->var));
        }
        return true;
    case E_THIS: vec_push(*out, path_root(NULL)); return true;
    case E_TO_IFACE: return place_paths(c, e->base, out);
    case E_FIELD:
    case E_INDEX: {
        PathVec base = {0};
        if (!place_paths(c, e->base, &base)) return false;
        Step step = {NULL, -1};
        const char *suffix;
        if (e->kind == E_FIELD) {
            step.field = e->decl;
            suffix = fmt_str(".%s", e->decl->name);
        } else {
            step.index = constant_index(e->index);
            suffix = index_text(e->index, step.index);
        }
        for (int i = 0; i < base.len; i++) vec_push(*out, path_extend(&base.data[i], step, suffix));
        free(base.data);
        return true;
    }
    default:
        if (!is_ref_call(e)) return false;
        if (e->kind != E_CALL_FUNC) place_paths(c, e->base, out);
        for (int i = 0; i < e->args.len; i++) {
            if (passed_by_reference(e, i)) place_paths(c, e->args.data[i], out);
        }
        return true;
    }
}

/* ---- Ending reference variables (spec 9.3) ---- */

/* Ends every reference variable other than except that reaches an object
 * overlapping one of paths. */
static void end_overlapping(OCtx *c, State *s, PathVec *paths, Var *except, Loc loc, const char *reason) {
    if (!s->reachable) return;
    for (int i = 0; i < c->refs.len; i++) {
        Var *r = c->refs.data[i];
        if (r == except || s->v[r->index] == GONE) continue;
        if (!any_overlap(&c->targets[r->index], paths, NULL, NULL)) continue;
        s->v[r->index] = GONE;
        if (error_suppress == 0 || !c->end_loc[r->index].file) {
            c->end_loc[r->index] = loc;
            c->end_reason[r->index] = reason;
        }
    }
}

static void use_ref_var(OCtx *c, Var *v, Loc loc, State *s) {
    if (!s->reachable || s->v[v->index] == USABLE) return;
    if (error_suppress == 0) {
        if (s->v[v->index] == GONE) {
            error_at(loc, "'%s' can't be used here: the reference ended when its object was reached another way", v->name);
        } else {
            error_at(loc, "'%s' can't be used here: the reference may have ended on some path, when its object was "
                          "reached another way",
                     v->name);
        }
        if (c->end_loc[v->index].file) note_at(c->end_loc[v->index], "'%s' ended here, because %s", v->name, c->end_reason[v->index]);
        s->v[v->index] = USABLE; /* report each use only once */
    }
}

/* ---- Ownership transfers (spec 7.1) ---- */

/* Reports a local that a call or assignment refers to (as receiver,
 * reference argument or assignment target) but that gave its object to a new
 * owner while the call's arguments or the assigned value were evaluated
 * (spec 7.3). before is the state just after the reference was taken. */
static void check_still_owned(OCtx *c, Var *v, State *before, State *after, Loc fallback, const char *what) {
    if (!v || !tracked(v) || !before->reachable || !after->reachable) return;
    if (before->v[v->index] != USABLE || after->v[v->index] == USABLE) return;
    Loc loc = c->transfer_loc[v->index].file ? c->transfer_loc[v->index] : fallback;
    error_at(loc, "'%s' can't be given to a new owner while %s refers to it", v->name, what);
}

/* True if every value e can produce is a new object (a call result), never
 * an existing object that would have to be copied. */
static bool is_new_object(Expr *e) {
    switch (e->kind) {
    case E_CALL_FUNC:
    case E_CALL_METHOD:
    case E_CALL_IFACE: return !is_ref_call(e);
    case E_CALL_CTOR:
    case E_CALL_STRING:
    case E_COPY:
    case E_STRING:
    case E_CONST:
    case E_BINARY: return true; /* string concatenation */
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

static void use_var(OCtx *c, Expr *e, State *s) {
    Var *v = e->var;
    if (is_ref_var(v)) {
        use_ref_var(c, v, e->loc, s);
        return;
    }
    if (!tracked(v) || !s->reachable) return;
    uint8_t st = s->v[v->index];
    if (st != USABLE) {
        if (error_suppress == 0) {
            if (st == GONE) {
                error_at(e->loc, "'%s' can't be used here: its object was given to a new owner", v->name);
            } else {
                error_at(e->loc, "'%s' can't be used here: its object may have been given to a new owner on some path",
                         v->name);
            }
            if (c->transfer_loc[v->index].file) note_at(c->transfer_loc[v->index], "ownership of '%s' was transferred here", v->name);
            s->v[v->index] = USABLE; /* report each use only once */
        }
        return;
    }
    if (e->transfers_ownership) {
        s->v[v->index] = GONE;
        if (error_suppress == 0 || !c->transfer_loc[v->index].file) c->transfer_loc[v->index] = e->loc;
    }
}

/* ---- Expressions ---- */

static void visit_expr(OCtx *c, Expr *e, State *s);

/* Evaluates the index expressions of a place, innermost first. */
static void visit_indexes(OCtx *c, Expr *e, Expr *root, State *s) {
    if (e == root) return;
    visit_indexes(c, e->base, root, s);
    if (e->kind == E_INDEX) visit_expr(c, e->index, s);
}

/* Reference variables to end once a call has received its arguments. */
typedef struct Pending {
    PathVec paths;
    Var *via;
    Loc loc;
} Pending;

typedef VEC(Pending) PendingVec;

/* Visits a place that is used: a variable, `this`, or a field or element of
 * a place or of any other value. The root is checked first, then index
 * expressions are evaluated, and finally the reference variables reaching
 * the object in another way end (spec 9.3), or are added to pending if the
 * place is passed by reference to a call. */
static void use_place(OCtx *c, Expr *e, State *s, PendingVec *pending) {
    Expr *root = e;
    while (root->kind == E_FIELD || root->kind == E_INDEX || root->kind == E_TO_IFACE) root = root->base;
    Var *via = root->kind == E_VAR && is_ref_var(root->var) ? root->var : NULL;
    /* a reference that has already ended reaches nothing, so ends nothing */
    bool valid = !via || !s->reachable || s->v[via->index] == USABLE;
    if (root->kind == E_VAR) {
        use_var(c, root, s);
    } else if (root->kind != E_THIS) {
        visit_expr(c, root, s);
    }
    visit_indexes(c, e, root, s);
    if (c->refs.len == 0 || !s->reachable || !valid) return;
    PathVec paths = {0};
    if (!place_paths(c, e, &paths) || !paths.len) {
        free(paths.data);
        return;
    }
    if (pending) {
        Pending p = {paths, via, e->loc};
        vec_push(*pending, p);
        return;
    }
    end_overlapping(c, s, &paths, via, e->loc, fmt_str("%s is used", paths.data[0].text));
    free(paths.data);
}

/* Visits the object a call is made on, or an argument passed by reference.
 * The call receives them all at once, so the reference variables they end
 * are ended only after all arguments are evaluated. */
static void visit_operand(OCtx *c, Expr *e, State *s, PendingVec *pending) {
    switch (e->kind) {
    case E_VAR:
    case E_THIS:
    case E_FIELD:
    case E_INDEX:
    case E_TO_IFACE: use_place(c, e, s, pending); return;
    default: visit_expr(c, e, s); return;
    }
}

typedef struct Reach {
    PathVec paths;
    Loc loc;
    const char *who;
} Reach;

/* A call can't reach the same object in two ways: the object it is called on
 * and its reference arguments must not overlap (spec 9.3). */
static void check_call_reach(OCtx *c, Expr *e) {
    if (error_suppress > 0) return;
    VEC(Reach) reaches = {0};
    if (has_receiver(e)) {
        Reach r = {{0}, e->base->loc, "the object it is called on"};
        place_paths(c, e->base, &r.paths);
        vec_push(reaches, r);
    }
    for (int i = 0; i < e->args.len; i++) {
        if (!passed_by_reference(e, i)) continue;
        Reach r = {{0}, e->args.data[i]->loc, fmt_str("argument %d", i + 1)};
        place_paths(c, e->args.data[i], &r.paths);
        vec_push(reaches, r);
    }
    for (int j = 1; j < reaches.len; j++) {
        for (int i = 0; i < j; i++) {
            int a, b;
            if (!any_overlap(&reaches.data[i].paths, &reaches.data[j].paths, &a, &b)) continue;
            Path *later = &reaches.data[j].paths.data[b];
            Path *earlier = &reaches.data[i].paths.data[a];
            const char *what = later->len < earlier->len ? fmt_str("part of %s", later->text) : later->text;
            error_at(reaches.data[j].loc,
                     "this call already reaches %s through %s, and a call can reach an object in only one way; pass a "
                     "copy with .copy()",
                     what, reaches.data[i].who);
            break;
        }
    }
    for (int i = 0; i < reaches.len; i++) free(reaches.data[i].paths.data);
    free(reaches.data);
}

static void visit_call(OCtx *c, Expr *e, State *s) {
    PendingVec pending = {0};
    if (has_receiver(e)) visit_operand(c, e->base, s, &pending);
    State after_receiver = state_copy(c, s);
    /* Each referenced argument must still own its object once all arguments
     * have been evaluated. */
    State *before_arg = xcalloc((size_t)(e->args.len ? e->args.len : 1), sizeof(State));
    for (int i = 0; i < e->args.len; i++) {
        if (passed_by_reference(e, i)) visit_operand(c, e->args.data[i], s, &pending);
        else visit_expr(c, e->args.data[i], s);
        before_arg[i] = state_copy(c, s);
    }
    if (has_receiver(e)) check_still_owned(c, root_var(e->base), &after_receiver, s, e->loc, "this call");
    for (int i = 0; i < e->args.len; i++) {
        if (passed_by_reference(e, i)) check_still_owned(c, root_var(e->args.data[i]), &before_arg[i], s, e->loc, "this call");
        free(before_arg[i].v);
    }
    free(before_arg);
    free(after_receiver.v);
    check_call_reach(c, e);
    for (int i = 0; i < pending.len; i++) {
        Pending *p = &pending.data[i];
        end_overlapping(c, s, &p->paths, p->via, p->loc, fmt_str("%s is used", p->paths.data[0].text));
        free(p->paths.data);
    }
    free(pending.data);
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
    case E_VAR:
    case E_THIS:
    case E_FIELD:
    case E_INDEX:
    case E_TO_IFACE: use_place(c, e, s, NULL); return;
    case E_LENGTH:
    case E_COPY:
    case E_CONVERT:
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
    case E_CALL_METHOD:
    case E_CALL_IFACE:
    case E_CALL_FUNC:
    case E_CALL_CTOR:
    case E_CALL_STRING:
    case E_EQUALS: visit_call(c, e, s); return;
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

/* ---- Statements ---- */

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

/* A reference variable starts usable, and ends every other reference
 * variable that reaches an overlapping object (spec 9.3). */
static void declare_reference(OCtx *c, Stmt *st, State *s) {
    Var *v = st->var;
    if (st->init) visit_expr(c, st->init, s);
    PathVec *t = &c->targets[v->index];
    t->len = 0;
    if (st->init) place_paths(c, st->init, t);
    if (!s->reachable) return;
    s->v[v->index] = USABLE;
    end_overlapping(c, s, t, v, st->loc, fmt_str("reference '%s' reaches the same object", v->name));
}

static void visit_stmt(OCtx *c, Stmt *st, State *s) {
    if (!st || !s->reachable) return;
    switch (st->kind) {
    case S_BLOCK:
        for (int i = 0; i < st->stmts.len; i++) visit_stmt(c, st->stmts.data[i], s);
        return;
    case S_VAR:
        if (is_ref_var(st->var)) {
            declare_reference(c, st, s);
            return;
        }
        if (st->init) visit_expr(c, st->init, s);
        if (s->reachable && tracked(st->var)) s->v[st->var->index] = USABLE;
        return;
    case S_EXPR: visit_expr(c, st->expr, s); return;
    case S_ASSIGN: {
        if (st->op == TOK_ASSIGN && st->expr->kind == E_VAR) {
            /* replacing a variable's object: references to the old one end */
            Var *whole = st->expr->var;
            visit_expr(c, st->value, s);
            if (s->reachable && tracked(whole)) s->v[whole->index] = USABLE;
            if (c->refs.len) {
                PathVec paths = {0};
                vec_push(paths, path_root(whole));
                end_overlapping(c, s, &paths, NULL, st->loc, fmt_str("%s is assigned", whole->name));
                free(paths.data);
            }
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
    size_t n = (size_t)(c.n ? c.n : 1);
    c.transfer_loc = xcalloc(n, sizeof(Loc));
    c.end_loc = xcalloc(n, sizeof(Loc));
    c.end_reason = xcalloc(n, sizeof(const char *));
    c.targets = xcalloc(n, sizeof(PathVec));
    for (int i = 0; i < fn->locals.len; i++) {
        if (is_ref_var(fn->locals.data[i])) vec_push(c.refs, fn->locals.data[i]);
    }
    State s = state_new(&c, true);
    visit_stmt(&c, fn->body, &s);
    free(s.v);
    free(c.transfer_loc);
    free(c.end_loc);
    free(c.end_reason);
    for (int i = 0; i < c.n; i++) free(c.targets[i].data);
    free(c.targets);
    free(c.refs.data);
}

void ownership_check_program(Program *prog) {
    for (int i = 0; i < prog->all_functions.len; i++) check_function(prog->all_functions.data[i]);
}
