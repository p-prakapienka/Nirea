/* Flow-sensitive checking of initialisation, moves and references (spec 6.8,
 * 7.2, 9.3). Semantic analysis has marked every local variable whose object
 * goes to a new owner (Expr.transfers_ownership). On each path, this pass
 * follows which locals have no value, because they were declared without one
 * or their object moved, and which reference variables have ended. A use of
 * a local declared without a value, or of a reference variable that has
 * ended, is an error. A later use of a local whose object moved, directly or
 * through a reference variable, instead turns the move into a copy
 * (Expr.implicit_copy), and the function is checked again, until no move is
 * followed by a use. The pass also rejects calls that reach the same object in
 * two ways.
 *
 * Objects are compared as paths from a root (a local variable, a parameter or
 * `this`) through fields and array elements. Two objects overlap when one
 * path is a prefix of the other, treating an index computed at run time as
 * any index. */
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "sema.h"

/* Per-variable state. For other locals: has a value, has none (declared
 * without one, or its object moved), or has none on some paths. For
 * reference variables: usable, ended, or ended on some paths. */
enum { USABLE = 0, GONE = 1, MAYBE_GONE = 2 };

/* The moves that may have taken a local's object: one move (site), or the
 * moves of two joined paths (a and b). */
typedef struct Sites {
    Expr *site;
    struct Sites *a, *b;
    int mark;
} Sites;

/* v holds a state for every local, followed (at v + v_bytes) by the Sites of
 * each local whose object may have moved, NULL otherwise. */
typedef struct State {
    uint8_t *v;
    bool reachable;
} State;

static int conversions; /* moves turned into copies in this check */
static int sites_mark;

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
    Loc *end_loc;            /* where each reference variable ended */
    const char **end_reason; /* why it ended */
    PathVec *targets;        /* objects each reference variable reaches */
    VarVec refs;             /* the function's reference variables */
    uint8_t *self_globals;   /* in methods: the global objects `this` may be (spec 9.3) */
} OCtx;

/* ---- Global objects (spec 5.6, 9.3) ---- */

/* The global objects a function can reach: by calling a method on them by
 * name, or through the functions it calls. For each one, where it is first
 * reached, and through which called function (NULL if by name). */
typedef struct GlobalReach {
    uint8_t *bits;
    Loc *loc;
    Decl **via;
    DeclVec callees;
    VEC(Loc) callee_locs;
} GlobalReach;

static Program *program;

static GlobalReach *reach_of(Decl *fn) {
    return fn ? (GlobalReach *)fn->reach : NULL;
}

static void add_callee(GlobalReach *r, Decl *callee, Loc loc) {
    if (!callee || !callee->reach) return;
    vec_push(r->callees, callee);
    vec_push(r->callee_locs, loc);
}

/* A call through an interface can run any implementation of the method. */
static void add_implementations(GlobalReach *r, Decl *im, Loc loc) {
    for (int i = 0; i < program->all_structs.len; i++) {
        Decl *s = program->all_structs.data[i];
        for (int j = 0; j < s->implements.len; j++) {
            if (s->implements.data[j].decl != im->owner) continue;
            Decl *m = map_get(&s->member_map, im->name);
            if (m && m->kind == D_METHOD) add_callee(r, m, loc);
        }
    }
}

static void collect_reach_stmt(GlobalReach *r, Stmt *s);

static void collect_reach_expr(GlobalReach *r, Expr *e) {
    if (!e) return;
    switch (e->kind) {
    case E_GLOBAL: {
        int g = e->decl->global_index;
        if (!r->bits[g]) {
            r->bits[g] = 1;
            r->loc[g] = e->loc;
        }
        break;
    }
    case E_CALL_FUNC:
    case E_CALL_METHOD: add_callee(r, e->decl, e->loc); break;
    case E_CALL_CTOR: add_callee(r, e->decl->ctor, e->loc); break;
    case E_CALL_IFACE: add_implementations(r, e->decl, e->loc); break;
    default: break;
    }
    collect_reach_expr(r, e->base);
    collect_reach_expr(r, e->index);
    for (int i = 0; i < e->args.len; i++) collect_reach_expr(r, e->args.data[i]);
    if (e->sw) {
        collect_reach_expr(r, e->sw->subject);
        for (int i = 0; i < e->sw->arms.len; i++) collect_reach_expr(r, e->sw->arms.data[i].expr);
    }
}

static void collect_reach_stmt(GlobalReach *r, Stmt *s) {
    if (!s) return;
    for (int i = 0; i < s->stmts.len; i++) collect_reach_stmt(r, s->stmts.data[i]);
    collect_reach_expr(r, s->init);
    collect_reach_expr(r, s->expr);
    collect_reach_expr(r, s->value);
    collect_reach_stmt(r, s->then_branch);
    collect_reach_stmt(r, s->else_branch);
    collect_reach_stmt(r, s->body);
    collect_reach_stmt(r, s->for_init);
    collect_reach_stmt(r, s->for_update);
    if (s->sw) {
        collect_reach_expr(r, s->sw->subject);
        for (int i = 0; i < s->sw->arms.len; i++) collect_reach_stmt(r, s->sw->arms.data[i].body);
    }
}

static const char *function_desc(Decl *fn) {
    if (fn->global_of) return fmt_str("the initialiser of '%s'", fn->global_of->name);
    if (fn->kind == D_CTOR) return fmt_str("the constructor of '%s'", fn->owner->name);
    if (fn->owner) return fmt_str("'%s.%s'", fn->owner->name, fn->name);
    return fmt_str("'%s'", fn->name);
}

/* Explains, with notes, how fn reaches global object g. */
static void note_reach_chain(Decl *fn, int g) {
    Decl *global = program->all_globals.data[g];
    for (int depth = 0; fn && depth < 100; depth++) {
        GlobalReach *r = reach_of(fn);
        if (!r->via[g]) {
            note_at(r->loc[g], "%s uses '%s' here", function_desc(fn), global->name);
            return;
        }
        note_at(r->loc[g], "%s calls %s here", function_desc(fn), function_desc(r->via[g]));
        fn = r->via[g];
    }
}

/* Works out, for every function, the global objects it can reach. */
static void compute_reach(Program *prog) {
    int n = prog->all_globals.len;
    for (int i = 0; i < prog->all_functions.len; i++) {
        GlobalReach *r = NEW(GlobalReach);
        r->bits = xcalloc((size_t)(n ? n : 1), 1);
        r->loc = xcalloc((size_t)(n ? n : 1), sizeof(Loc));
        r->via = xcalloc((size_t)(n ? n : 1), sizeof(Decl *));
        prog->all_functions.data[i]->reach = r;
    }
    for (int i = 0; i < prog->all_functions.len; i++) {
        Decl *fn = prog->all_functions.data[i];
        collect_reach_stmt(reach_of(fn), fn->body);
    }
    for (bool changed = true; changed;) {
        changed = false;
        for (int i = 0; i < prog->all_functions.len; i++) {
            GlobalReach *r = reach_of(prog->all_functions.data[i]);
            for (int k = 0; k < r->callees.len; k++) {
                GlobalReach *c = reach_of(r->callees.data[k]);
                for (int g = 0; g < n; g++) {
                    if (!c->bits[g] || r->bits[g]) continue;
                    r->bits[g] = 1;
                    r->loc[g] = r->callee_locs.data[k];
                    r->via[g] = r->callees.data[k];
                    changed = true;
                }
            }
        }
    }
}

/* True if a call can reach global object g; *via is the function that does,
 * or NULL if the call is made on g by name. */
static bool call_reaches(Expr *call, int g, Decl **via) {
    *via = NULL;
    switch (call->kind) {
    case E_CALL_METHOD:
        if (call->base->kind == E_GLOBAL && call->base->decl->global_index == g) return true;
        /* fall through */
    case E_CALL_FUNC: *via = call->decl; break;
    case E_CALL_CTOR: *via = call->decl->ctor; break;
    case E_CALL_IFACE:
        for (int i = 0; i < program->all_structs.len; i++) {
            Decl *s = program->all_structs.data[i];
            for (int j = 0; j < s->implements.len; j++) {
                if (s->implements.data[j].decl != call->decl->owner) continue;
                Decl *m = map_get(&s->member_map, call->decl->name);
                if (m && reach_of(m) && reach_of(m)->bits[g]) {
                    *via = m;
                    return true;
                }
            }
        }
        return false;
    default: return false;
    }
    return *via && reach_of(*via) && reach_of(*via)->bits[g];
}

/* Global objects are created in dependency order; an initialiser that can
 * reach its own global object, directly or through others, is an error
 * (spec 5.6). */
static void order_globals(Program *prog, int g, uint8_t *state) {
    if (state[g] == 2) return;
    Decl *global = prog->all_globals.data[g];
    if (state[g] == 1) {
        if (reach_of(global->init_fn)->bits[g]) {
            error_at(global->loc, "global object '%s' can't be created: its initialiser can reach '%s' itself", global->name,
                     global->name);
            note_reach_chain(global->init_fn, g);
        } else {
            error_at(global->loc,
                     "global object '%s' can't be created: the global objects its initialiser reaches need '%s' first",
                     global->name, global->name);
        }
        state[g] = 2;
        return;
    }
    state[g] = 1;
    GlobalReach *r = reach_of(global->init_fn);
    for (int h = 0; h < prog->all_globals.len; h++) {
        if (r->bits[h]) order_globals(prog, h, state);
    }
    if (state[g] == 1) vec_push(prog->global_order, global);
    state[g] = 2;
}

static size_t v_bytes(OCtx *c) {
    return ((size_t)(c->n ? c->n : 1) + 15) & ~(size_t)15;
}

static size_t state_bytes(OCtx *c) {
    return v_bytes(c) + (size_t)(c->n ? c->n : 1) * sizeof(Sites *);
}

static Sites **moves_of(OCtx *c, State *s) {
    return (Sites **)(void *)(s->v + v_bytes(c));
}

static State state_new(OCtx *c, bool reachable) {
    State s;
    s.v = xcalloc(state_bytes(c), 1);
    s.reachable = reachable;
    return s;
}

static State state_copy(OCtx *c, State *src) {
    State s = state_new(c, src->reachable);
    memcpy(s.v, src->v, state_bytes(c));
    return s;
}

static Sites *sites_join(Sites *x, Sites *y) {
    if (x == y || !y) return x;
    if (!x) return y;
    Sites *u = NEW(Sites);
    u->a = x;
    u->b = y;
    return u;
}

static void state_join(OCtx *c, State *dst, State *src) {
    if (!src->reachable) return;
    if (!dst->reachable) {
        memcpy(dst->v, src->v, state_bytes(c));
        dst->reachable = true;
        return;
    }
    Sites **dm = moves_of(c, dst), **sm = moves_of(c, src);
    for (int i = 0; i < c->n; i++) {
        if (dst->v[i] != src->v[i]) dst->v[i] = MAYBE_GONE;
        dm[i] = sites_join(dm[i], sm[i]);
    }
}

/* Turns the moves in t into copies, since the object is used afterwards
 * (spec 7.2). */
static void keep_sites(Sites *t) {
    if (!t || t->mark == sites_mark) return;
    t->mark = sites_mark;
    if (t->site) {
        if (t->site->transfers_ownership) {
            t->site->transfers_ownership = false;
            t->site->implicit_copy = true;
            conversions++;
        }
        return;
    }
    keep_sites(t->a);
    keep_sites(t->b);
}

/* Local v, or a reference variable reaching it, is used: if its object may
 * have moved, those moves become copies. Returns true if it had moved. */
static bool keep_moved(OCtx *c, State *s, Var *v) {
    if (!s->reachable || s->v[v->index] == USABLE) return false;
    Sites **m = moves_of(c, s);
    if (!m[v->index]) return false;
    sites_mark++;
    keep_sites(m[v->index]);
    m[v->index] = NULL;
    s->v[v->index] = USABLE;
    return true;
}

/* The local's state when it is assigned or declared. */
static void set_var(OCtx *c, State *s, Var *v, uint8_t st) {
    s->v[v->index] = st;
    moves_of(c, s)[v->index] = NULL;
}

static bool state_equal(OCtx *c, State *a, State *b) {
    if (a->reachable != b->reachable) return false;
    if (!a->reachable) return true;
    return memcmp(a->v, b->v, (size_t)c->n) == 0;
}

/* Locals that hold a value, including value parameters (spec 6.8, 7.3). */
static bool tracked(Var *v) {
    return !v->is_ref;
}

static bool is_ref_var(Var *v) {
    return v->is_ref && v->kind == VAR_LOCAL;
}

static Var *call_param(Expr *call, int i) {
    Decl *fn = call->decl;
    if (call->kind == E_CALL_CTOR) fn = fn->ctor;
    if (!fn || i >= fn->params.len) return NULL;
    return fn->params.data[i];
}

/* True if argument i of a call is passed to a reference parameter. The
 * arguments of built-in methods are values (spec 6.5, 6.9). */
static bool passed_by_reference(Expr *call, int i) {
    if (call->kind == E_EQUALS || call->kind == E_CALL_STRING || call->kind == E_CALL_PRINT) return false;
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
/* Ends the reference variables other than except that reach an object
 * overlapping one of paths: all of them for a change, and only changing ones
 * for a read, since any number of references can read at once (spec 9.3). */
static void end_overlapping(OCtx *c, State *s, PathVec *paths, Var *except, bool change, Loc loc, const char *reason) {
    if (!s->reachable) return;
    for (int i = 0; i < c->refs.len; i++) {
        Var *r = c->refs.data[i];
        if (r == except || s->v[r->index] == GONE || (!change && !r->changes)) continue;
        if (!any_overlap(&c->targets[r->index], paths, NULL, NULL)) continue;
        s->v[r->index] = GONE;
        if (error_suppress == 0 || !c->end_loc[r->index].file) {
            c->end_loc[r->index] = loc;
            c->end_reason[r->index] = reason;
        }
    }
}

static void use_ref_var(OCtx *c, Var *v, Loc loc, State *s) {
    if (!s->reachable) return;
    if (s->v[v->index] == USABLE) {
        /* the objects it reaches must not have moved (spec 7.2) */
        PathVec *t = &c->targets[v->index];
        for (int i = 0; i < t->len; i++) {
            Var *root = t->data[i].root;
            if (root && tracked(root)) keep_moved(c, s, root);
        }
        return;
    }
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

/* ---- Moves (spec 7.2) ---- */

/* A call or assignment refers to place (as receiver, reference argument or
 * assignment target), and the locals it reaches must keep their objects
 * while the call's arguments or the assigned value are evaluated: any that
 * moved meanwhile is copied instead. before is the state just after the
 * reference was taken. */
static void keep_still_owned(OCtx *c, Expr *place, State *before, State *after) {
    if (!before->reachable || !after->reachable) return;
    PathVec paths = {0};
    if (!place_paths(c, place, &paths)) {
        free(paths.data);
        return;
    }
    for (int i = 0; i < paths.len; i++) {
        Var *v = paths.data[i].root;
        if (v && tracked(v) && before->v[v->index] == USABLE) keep_moved(c, after, v);
    }
    free(paths.data);
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
    case E_NEW_ARRAY:
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

/* A switch expression of an owned type outside a position where its result
 * gets an owner would work on a copy of an existing object, which is then
 * lost (spec 10.1). */
static void check_switch_result(Expr *e) {
    if (e->transfers_ownership || !e->type || !type_is_owned(e->type)) return;
    for (int i = 0; i < e->sw->arms.len; i++) {
        Expr *arm = e->sw->arms.data[i].expr;
        if (arm->kind == E_SWITCH) continue; /* checked when visited */
        if (!is_new_object(arm)) {
            error_at(arm->loc,
                     "this switch branch refers to an existing %s, which the switch would copy, and the copy would be "
                     "lost; use the switch where its result gets an owner, as in '%s value = switch ...'",
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
    if (s->v[v->index] != USABLE && !keep_moved(c, s, v)) {
        /* declared without a value (spec 6.8) */
        if (s->v[v->index] == GONE) {
            error_at(e->loc, "'%s' can't be used here: it has no value yet; assign one to it first", v->name);
        } else {
            error_at(e->loc, "'%s' can't be used here: it may have no value on some path; assign one to it on every "
                             "path first",
                     v->name);
        }
        note_at(v->loc, "'%s' is declared here without a value", v->name);
        s->v[v->index] = USABLE; /* report each use only once */
    }
    if (e->transfers_ownership) {
        /* the object moves, unless the variable is used afterwards (spec 7.2) */
        Sites *site = NEW(Sites);
        site->site = e;
        s->v[v->index] = GONE;
        moves_of(c, s)[v->index] = site;
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
    bool change;
    Loc loc;
} Pending;

typedef VEC(Pending) PendingVec;

/* Visits a place that is used: a variable, `this`, or a field or element of
 * a place or of any other value. The root is checked first, then index
 * expressions are evaluated, and finally the reference variables reaching
 * the object in another way end (spec 9.3), or are added to pending if the
 * place is passed by reference to a call. */
static void use_place(OCtx *c, Expr *e, State *s, PendingVec *pending, bool change) {
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
        Pending p = {paths, via, change, e->loc};
        vec_push(*pending, p);
        return;
    }
    end_overlapping(c, s, &paths, via, change, e->loc,
                    fmt_str("%s is %s", paths.data[0].text, change ? "changed" : "used"));
    free(paths.data);
}

/* Visits the object a call is made on, or an argument passed by reference.
 * The call receives them all at once, so the reference variables they end
 * are ended only after all arguments are evaluated. */
static void visit_operand(OCtx *c, Expr *e, State *s, PendingVec *pending, bool change) {
    switch (e->kind) {
    case E_VAR:
    case E_THIS:
    case E_FIELD:
    case E_INDEX:
    case E_TO_IFACE: use_place(c, e, s, pending, change); return;
    default: visit_expr(c, e, s); return;
    }
}

static bool call_changes_receiver(Expr *call);
static bool call_changes_arg(Expr *call, int i);

typedef struct Reach {
    PathVec paths;
    Loc loc;
    const char *who;
    bool change;
} Reach;

/* A call can receive overlapping objects, as the object it is called on and as
 * reference arguments, only if it changes none of them (spec 9.3). */
static void check_call_reach(OCtx *c, Expr *e) {
    if (error_suppress > 0) return;
    VEC(Reach) reaches = {0};
    if (has_receiver(e)) {
        Reach r = {{0}, e->base->loc, "the object it is called on", call_changes_receiver(e)};
        place_paths(c, e->base, &r.paths);
        vec_push(reaches, r);
    }
    for (int i = 0; i < e->args.len; i++) {
        if (!passed_by_reference(e, i)) continue;
        Reach r = {{0}, e->args.data[i]->loc, fmt_str("argument %d", i + 1), call_changes_arg(e, i)};
        place_paths(c, e->args.data[i], &r.paths);
        vec_push(reaches, r);
    }
    for (int j = 1; j < reaches.len; j++) {
        for (int i = 0; i < j; i++) {
            int a, b;
            if (!reaches.data[i].change && !reaches.data[j].change) continue; /* reading only */
            if (!any_overlap(&reaches.data[i].paths, &reaches.data[j].paths, &a, &b)) continue;
            bool later_changes = reaches.data[j].change;
            Reach *changer = later_changes ? &reaches.data[j] : &reaches.data[i];
            Reach *other = later_changes ? &reaches.data[i] : &reaches.data[j];
            Path *changed = &changer->paths.data[later_changes ? b : a];
            Path *received = &other->paths.data[later_changes ? a : b];
            error_at(reaches.data[j].loc,
                     "this call changes %s (%s), so it can't also receive %s (%s); assign one of them to a variable to "
                     "pass a copy",
                     changed->text, changer->who, received->text, other->who);
            break;
        }
    }
    for (int i = 0; i < reaches.len; i++) free(reaches.data[i].paths.data);
    free(reaches.data);
}

/* How a call reaches global object g, as text for a note. */
static const char *reach_chain_text(Decl *via, int g) {
    Decl *global = program->all_globals.data[g];
    if (!via) return fmt_str("it is made on '%s'", global->name);
    StrBuf sb = {0};
    sb_append(&sb, function_desc(via));
    Decl *fn = via;
    for (int depth = 0; fn && depth < 100; depth++) {
        Decl *next = reach_of(fn)->via[g];
        if (!next) {
            sb_printf(&sb, " uses '%s'", global->name);
            break;
        }
        sb_printf(&sb, " calls %s, which", function_desc(next));
        fn = next;
    }
    return sb_cstr(&sb);
}

/* Inside the methods of a struct type that has global objects, `this` may be
 * one of them. A call that can reach it can't receive a reference into its
 * state, and ends the reference variables into it (spec 9.3). */
static void check_global_reach(OCtx *c, Expr *e, State *s) {
    if (!c->self_globals || !s->reachable) return;
    for (int g = 0; g < program->all_globals.len; g++) {
        Decl *via;
        if (!c->self_globals[g] || !call_reaches(e, g, &via)) continue;
        Decl *global = program->all_globals.data[g];
        for (int i = -1; i < e->args.len && error_suppress == 0; i++) {
            Expr *arg;
            if (i < 0) {
                if (!has_receiver(e)) continue;
                arg = e->base;
                if (arg->kind == E_THIS && e->kind == E_CALL_METHOD) continue; /* one of its own methods */
            } else {
                if (!passed_by_reference(e, i)) continue;
                arg = e->args.data[i];
            }
            PathVec paths = {0};
            bool into = place_paths(c, arg, &paths);
            bool found = false;
            for (int k = 0; into && k < paths.len; k++) found = found || paths.data[k].root == NULL;
            free(paths.data);
            if (!found) continue;
            error_at(arg->loc,
                     "this call can reach global object '%s' while %s refers into it, which could leave the reference "
                     "pointing at replaced memory; assign it to a variable and pass that copy instead",
                     global->name, i < 0 ? "the object it is called on" : "this argument");
            if (via) note_reach_chain(via, g);
            else note_at(e->loc, "the call is made on '%s' itself", global->name);
        }
        PathVec self = {0};
        vec_push(self, path_root(NULL));
        end_overlapping(c, s, &self, NULL, true, e->loc,
                        fmt_str("this call can reach global object '%s': %s", global->name, reach_chain_text(via, g)));
        free(self.data);
    }
}

static void visit_call(OCtx *c, Expr *e, State *s) {
    PendingVec pending = {0};
    if (has_receiver(e)) visit_operand(c, e->base, s, &pending, call_changes_receiver(e));
    State after_receiver = state_copy(c, s);
    /* Each referenced argument must still own its object once all arguments
     * have been evaluated. */
    State *before_arg = xcalloc((size_t)(e->args.len ? e->args.len : 1), sizeof(State));
    for (int i = 0; i < e->args.len; i++) {
        if (passed_by_reference(e, i)) visit_operand(c, e->args.data[i], s, &pending, call_changes_arg(e, i));
        else visit_expr(c, e->args.data[i], s);
        before_arg[i] = state_copy(c, s);
    }
    if (has_receiver(e)) keep_still_owned(c, e->base, &after_receiver, s);
    for (int i = 0; i < e->args.len; i++) {
        if (passed_by_reference(e, i)) keep_still_owned(c, e->args.data[i], &before_arg[i], s);
        free(before_arg[i].v);
    }
    free(before_arg);
    free(after_receiver.v);
    check_call_reach(c, e);
    check_global_reach(c, e, s);
    for (int i = 0; i < pending.len; i++) {
        Pending *p = &pending.data[i];
        end_overlapping(c, s, &p->paths, p->via, p->change, p->loc,
                        fmt_str("%s is %s", p->paths.data[0].text, p->change ? "changed" : "used"));
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
    case E_TO_IFACE: use_place(c, e, s, NULL, false); return; /* copying or moving only reads */
    case E_LENGTH:
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
    case E_NEW_ARRAY:
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
    end_overlapping(c, s, t, v, v->changes, st->loc, fmt_str("reference '%s' reaches the same object", v->name));
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
        if (s->reachable && tracked(st->var)) {
            /* without an initialiser, it has no value until assigned (spec 6.8) */
            set_var(c, s, st->var, st->init ? USABLE : GONE);
        }
        return;
    case S_EXPR: visit_expr(c, st->expr, s); return;
    case S_ASSIGN: {
        if (st->op == TOK_ASSIGN && st->expr->kind == E_VAR) {
            /* replacing a variable's object: references to the old one end */
            Var *whole = st->expr->var;
            visit_expr(c, st->value, s);
            if (s->reachable && tracked(whole)) set_var(c, s, whole, USABLE);
            if (c->refs.len) {
                PathVec paths = {0};
                vec_push(paths, path_root(whole));
                end_overlapping(c, s, &paths, NULL, true, st->loc, fmt_str("%s is assigned", whole->name));
                free(paths.data);
            }
            return;
        }
        visit_operand(c, st->expr, s, NULL, true); /* the target is changed */
        State after_target = state_copy(c, s);
        visit_expr(c, st->value, s);
        keep_still_owned(c, st->expr, &after_target, s);
        free(after_target.v);
        return;
    }
    case S_INCDEC: visit_operand(c, st->expr, s, NULL, true); return;
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

/* ---- Which uses change an object (spec 9.3) ---- */

static bool string_method_changes(Expr *call) {
    return call->kind == E_CALL_STRING && call->base &&
           (strcmp(call->name, "append") == 0 || strcmp(call->name, "appendChar") == 0 ||
            strcmp(call->name, "setCharAt") == 0);
}

/* True if a call changes the object it is called on. */
static bool call_changes_receiver(Expr *call) {
    switch (call->kind) {
    case E_CALL_METHOD:
    case E_CALL_IFACE: return call->decl->changes_this;
    case E_CALL_STRING: return string_method_changes(call);
    default: return false;
    }
}

/* True if a call changes argument i, passed by reference. */
static bool call_changes_arg(Expr *call, int i) {
    if (!passed_by_reference(call, i)) return false;
    Var *p = call_param(call, i);
    return p && p->changes;
}

/* Records that fn changes the object e reaches: `this`, a reference
 * parameter, or a reference variable and what it was declared from. */
static void mark_changed(Decl *fn, Expr *e, bool *progress) {
    while (e->kind == E_FIELD || e->kind == E_INDEX || e->kind == E_TO_IFACE) e = e->base;
    if (e->kind == E_THIS) {
        if (fn->kind == D_METHOD && !fn->changes_this) {
            fn->changes_this = true;
            *progress = true;
        }
        return;
    }
    if (e->kind == E_VAR) {
        Var *v = e->var;
        if (!v->is_ref) return; /* an object the function owns */
        if (!v->changes) {
            v->changes = true;
            *progress = true;
        }
        if (v->kind == VAR_LOCAL && v->ref_init) mark_changed(fn, v->ref_init, progress);
        return;
    }
    if (is_ref_call(e)) {
        if (e->kind != E_CALL_FUNC) mark_changed(fn, e->base, progress);
        for (int i = 0; i < e->args.len; i++) {
            if (passed_by_reference(e, i)) mark_changed(fn, e->args.data[i], progress);
        }
    }
}

static void scan_changes_expr(Decl *fn, Expr *e, bool *progress) {
    if (!e) return;
    switch (e->kind) {
    case E_CALL_METHOD:
    case E_CALL_IFACE:
    case E_CALL_STRING:
        if (call_changes_receiver(e)) mark_changed(fn, e->base, progress);
        /* fall through */
    case E_CALL_FUNC:
    case E_CALL_CTOR:
        for (int i = 0; i < e->args.len; i++) {
            if (call_changes_arg(e, i)) mark_changed(fn, e->args.data[i], progress);
        }
        break;
    default: break;
    }
    scan_changes_expr(fn, e->base, progress);
    scan_changes_expr(fn, e->index, progress);
    for (int i = 0; i < e->args.len; i++) scan_changes_expr(fn, e->args.data[i], progress);
    if (e->sw) {
        scan_changes_expr(fn, e->sw->subject, progress);
        for (int i = 0; i < e->sw->arms.len; i++) scan_changes_expr(fn, e->sw->arms.data[i].expr, progress);
    }
}

static void scan_changes_stmt(Decl *fn, Stmt *s, bool *progress) {
    if (!s) return;
    if (s->kind == S_VAR && s->var->is_ref) s->var->ref_init = s->init;
    if (s->kind == S_ASSIGN || s->kind == S_INCDEC) mark_changed(fn, s->expr, progress);
    for (int i = 0; i < s->stmts.len; i++) scan_changes_stmt(fn, s->stmts.data[i], progress);
    scan_changes_expr(fn, s->init, progress);
    scan_changes_expr(fn, s->expr, progress);
    scan_changes_expr(fn, s->value, progress);
    scan_changes_stmt(fn, s->then_branch, progress);
    scan_changes_stmt(fn, s->else_branch, progress);
    scan_changes_stmt(fn, s->body, progress);
    scan_changes_stmt(fn, s->for_init, progress);
    scan_changes_stmt(fn, s->for_update, progress);
    if (s->sw) {
        scan_changes_expr(fn, s->sw->subject, progress);
        for (int i = 0; i < s->sw->arms.len; i++) scan_changes_stmt(fn, s->sw->arms.data[i].body, progress);
    }
}

/* An interface method changes `this`, or a parameter, if any implementation
 * does. */
static void infer_interface_changes(Program *prog, bool *progress) {
    for (int i = 0; i < prog->all_structs.len; i++) {
        Decl *s = prog->all_structs.data[i];
        for (int j = 0; j < s->implements.len; j++) {
            Decl *iface = s->implements.data[j].decl;
            if (!iface) continue;
            for (int k = 0; k < iface->members.len; k++) {
                Decl *im = iface->members.data[k];
                Decl *m = map_get(&s->member_map, im->name);
                if (!m || m->kind != D_METHOD) continue;
                if (m->changes_this && !im->changes_this) {
                    im->changes_this = true;
                    *progress = true;
                }
                for (int p = 0; p < im->params.len && p < m->params.len; p++) {
                    if (m->params.data[p]->changes && !im->params.data[p]->changes) {
                        im->params.data[p]->changes = true;
                        *progress = true;
                    }
                }
            }
        }
    }
}

/* Works out, for the whole program, which methods change `this` and which
 * reference parameters and variables are used to change their object. */
static void infer_changes(Program *prog) {
    for (bool progress = true; progress;) {
        progress = false;
        for (int i = 0; i < prog->all_functions.len; i++) {
            Decl *fn = prog->all_functions.data[i];
            scan_changes_stmt(fn, fn->body, &progress);
        }
        infer_interface_changes(prog, &progress);
    }
}

static void run_check(OCtx *c) {
    size_t n = (size_t)(c->n ? c->n : 1);
    c->loop = NULL;
    c->end_loc = xcalloc(n, sizeof(Loc));
    c->end_reason = xcalloc(n, sizeof(const char *));
    c->targets = xcalloc(n, sizeof(PathVec));
    State s = state_new(c, true);
    visit_stmt(c, c->fn->body, &s);
    free(s.v);
    free(c->end_loc);
    free(c->end_reason);
    for (int i = 0; i < c->n; i++) free(c->targets[i].data);
    free(c->targets);
}

static void check_function(Decl *fn) {
    OCtx c = {0};
    c.fn = fn;
    c.n = fn->locals.len;
    for (int i = 0; i < fn->locals.len; i++) {
        if (is_ref_var(fn->locals.data[i])) vec_push(c.refs, fn->locals.data[i]);
    }
    if (fn->kind == D_METHOD) {
        for (int g = 0; g < program->all_globals.len; g++) {
            if (program->all_globals.data[g]->field_type != fn->owner->type) continue;
            if (!c.self_globals) c.self_globals = xcalloc((size_t)program->all_globals.len, 1);
            c.self_globals[g] = 1;
        }
    }
    /* Moves followed by a use become copies, which can expose other moves
     * followed by a use; once none is left, check again to report errors. */
    do {
        conversions = 0;
        error_suppress++;
        run_check(&c);
        error_suppress--;
    } while (conversions);
    run_check(&c);
    free(c.refs.data);
    free(c.self_globals);
}

void ownership_check_program(Program *prog) {
    program = prog;
    compute_reach(prog);
    uint8_t *state = xcalloc((size_t)(prog->all_globals.len ? prog->all_globals.len : 1), 1);
    for (int g = 0; g < prog->all_globals.len; g++) order_globals(prog, g, state);
    free(state);
    infer_changes(prog);
    for (int i = 0; i < prog->all_functions.len; i++) check_function(prog->all_functions.data[i]);
}
