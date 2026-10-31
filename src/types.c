#include "ast.h"

static Type t_error = {TY_ERROR, .cname = "int"};
static Type t_void = {TY_VOID, .cname = "void"};
static Type t_bool = {TY_BOOL, .cname = "bool"};
static Type t_char = {TY_CHAR, .cname = "uint8_t"};
static Type t_i8 = {TY_I8, .cname = "int8_t"};
static Type t_i16 = {TY_I16, .cname = "int16_t"};
static Type t_i32 = {TY_I32, .cname = "int32_t"};
static Type t_i64 = {TY_I64, .cname = "int64_t"};
static Type t_u8 = {TY_U8, .cname = "uint8_t"};
static Type t_u16 = {TY_U16, .cname = "uint16_t"};
static Type t_u32 = {TY_U32, .cname = "uint32_t"};
static Type t_u64 = {TY_U64, .cname = "uint64_t"};
static Type t_f32 = {TY_F32, .cname = "float"};
static Type t_f64 = {TY_F64, .cname = "double"};
static Type t_string = {TY_STRING, .cname = "nr_String"};

Type *type_error = &t_error, *type_void = &t_void, *type_bool = &t_bool, *type_char = &t_char,
     *type_i8 = &t_i8, *type_i16 = &t_i16, *type_i32 = &t_i32, *type_i64 = &t_i64, *type_u8 = &t_u8,
     *type_u16 = &t_u16, *type_u32 = &t_u32, *type_u64 = &t_u64, *type_f32 = &t_f32, *type_f64 = &t_f64,
     *type_string = &t_string;

static VEC(Type *) array_types;

Type *type_array(Type *elem) {
    for (int i = 0; i < array_types.len; i++) {
        Type *t = array_types.data[i];
        if (t->elem == elem) return t;
    }
    Type *t = NEW(Type);
    t->kind = TY_ARRAY;
    t->elem = elem;
    t->cname = fmt_str("nr_arr%d", array_types.len);
    vec_push(array_types, t);
    return t;
}

int type_array_count(void) {
    return array_types.len;
}

Type *type_array_at(int index) {
    return array_types.data[index];
}

bool type_is_integer(Type *t) {
    return t->kind >= TY_I8 && t->kind <= TY_U64;
}

bool type_is_signed(Type *t) {
    return t->kind >= TY_I8 && t->kind <= TY_I64;
}

bool type_is_float(Type *t) {
    return t->kind == TY_F32 || t->kind == TY_F64;
}

bool type_is_numeric(Type *t) {
    return type_is_integer(t) || type_is_float(t);
}

bool type_is_owned(Type *t) {
    return t->kind == TY_STRUCT || t->kind == TY_ARRAY || t->kind == TY_STRING;
}

int type_int_bits(Type *t) {
    switch (t->kind) {
    case TY_I8:
    case TY_U8:
    case TY_CHAR: return 8;
    case TY_I16:
    case TY_U16: return 16;
    case TY_I32:
    case TY_U32: return 32;
    case TY_I64:
    case TY_U64: return 64;
    default: return 0;
    }
}

const char *type_name(Type *t) {
    switch (t->kind) {
    case TY_ERROR: return "<error>";
    case TY_VOID: return "void";
    case TY_BOOL: return "boolean";
    case TY_CHAR: return "char";
    case TY_I8: return "int8";
    case TY_I16: return "int16";
    case TY_I32: return "int";
    case TY_I64: return "int64";
    case TY_U8: return "uint8";
    case TY_U16: return "uint16";
    case TY_U32: return "uint";
    case TY_U64: return "uint64";
    case TY_F32: return "float";
    case TY_F64: return "float64";
    case TY_STRING: return "String";
    case TY_ENUM:
    case TY_STRUCT:
    case TY_INTERFACE:
        if (t->decl->ns) return fmt_str("%s.%s", t->decl->ns->name, t->decl->name);
        return t->decl->name;
    case TY_ARRAY: return fmt_str("%s[]", type_name(t->elem));
    }
    return "?";
}
