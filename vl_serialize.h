// [vl_serialize.h](https://github.com/victor-Lopez25/viclib) © 2024 by [Víctor López Cortés](https://github.com/victor-Lopez25) is licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0)
#ifndef VL_SERIALIZE_H
#define VL_SERIALIZE_H

#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef VL_BUILD_PATH
#define VL_BUILD_PATH "vl_build.h"
#endif // VL_BUILD_PATH

#if defined(VL_SERIALIZE_IMPLEMENTATION) && !defined(VL_BUILD_IMPLEMENTATION)
# define VL_BUILD_IMPLEMENTATION
#endif

#include VL_BUILD_PATH

#ifndef SERIALIZE_PROC
#define SERIALIZE_PROC VLIBPROC
#endif // SERIALIZE_PROC

/* This library will only print utf8 strings */

typedef enum {
    SerializeType_JSON,
    SerializeType_XML,
} vl_serialize_type;

typedef enum {
    SerializeScope_Object,
    SerializeScope_Array,
} vl_serialize_scope_type;

typedef struct {
    vl_serialize_scope_type type;
    bool prev_needs_comma;
    bool did_key;
    view current_elem;
    view array_elem_name;
    uint32_t array_elem_idx;
} vl_serialize_scope;

typedef struct {
    vl_serialize_type type;
    uint8_t indent;
} GetSerializeContext_opts;

typedef struct vl_serialize_context vl_serialize_context;
struct vl_serialize_context {
    vl_serialize_type type;
    string_builder output;
    uint8_t indent;

    struct {
        vl_serialize_scope *items;
        size_t count;
        size_t capacity;
    } scopes;
    uint32_t count_object_scopes;
    uint32_t count_array_scopes;
    bool should_pop_scope;
    bool should_quote_strings;

    void (*ObjectBegin)(vl_serialize_context *ctx);
    void (*AttributeNameView)(vl_serialize_context *ctx, view name);
    void (*ObjectEnd)(vl_serialize_context *ctx);
    void (*ArrayBeginView)(vl_serialize_context *ctx, view name);
    void (*ArrayEnd)(vl_serialize_context *ctx);

    // Internals
    void (*ElementBegin)(vl_serialize_context *ctx);
    void (*ElementEnd)(vl_serialize_context *ctx);
};

struct VL_ArrayBegin_opts {
    vl_serialize_context *ctx;
    const char *elem_name; /* array elements name, used in XML generation */
};

#define VL_ObjectBegin(serialize_ctx) ((serialize_ctx)->ObjectBegin(serialize_ctx))
#define VL_AttributeNameView(serialize_ctx, v) ((serialize_ctx)->AttributeNameView(serialize_ctx), v)
#define VL_ObjectEnd(serialize_ctx) ((serialize_ctx)->ObjectEnd(serialize_ctx))
#define VL_ArrayBegin(serialize_ctx, ...) VL_ArrayBegin_Impl((struct VL_ArrayBegin_opts){.ctx = (serialize_ctx), __VA_ARGS__})
#define VL_ArrayEnd(serialize_ctx) ((serialize_ctx)->ArrayEnd(serialize_ctx))

#define GetSerializeContext(Type, ...) \
    GetSerializeContext_Impl((GetSerializeContext_opts){.type = Type, __VA_ARGS__})
SERIALIZE_PROC vl_serialize_context GetSerializeContext_Impl(GetSerializeContext_opts opt);

SERIALIZE_PROC void VL_ArrayBegin_Impl(struct VL_ArrayBegin_opts opt);
SERIALIZE_PROC void VL_AttributeName(vl_serialize_context *ctx, const char *name);

SERIALIZE_PROC void VL_SerializeNull(vl_serialize_context *ctx);

SERIALIZE_PROC void VL_SerializeBool(vl_serialize_context *ctx, bool b);
SERIALIZE_PROC void VL_SerializeInt(vl_serialize_context *ctx, int64_t val);

// TODO: More floating point printing options?
SERIALIZE_PROC void VL_SerializeFloat(vl_serialize_context *ctx, double val);

SERIALIZE_PROC void VL_SerializeString(vl_serialize_context *ctx, const char *s);
SERIALIZE_PROC void VL_SerializeView(vl_serialize_context *ctx, view v);

#ifdef VL_SERIALIZE_IMPLEMENTATION

static vl_serialize_scope *VL__SerializeScopePush(vl_serialize_context *ctx, vl_serialize_scope_type type)
{
    vl_serialize_scope scope = {
        .type = type,
        .prev_needs_comma = false,
        .did_key = false,
    };
    da_Append(&ctx->scopes, scope);
    return &ctx->scopes.items[ctx->scopes.count - 1];
}

static void VL__SerializeScopePop(vl_serialize_context *ctx)
{
    Assert(ctx->scopes.count > 0);
    ctx->scopes.count--;
}

static uint8_t VL__Utf8CharLen(uint8_t ch)
{
    if((ch & 0x80) == 0) return 1;

    switch(ch & 0xf0) {
        case 0xf0: return 4;
        case 0xe0: return 3;
        default:   return 2;
    }
}

static void VL__SerializeViewNoElement(vl_serialize_context *ctx, view v)
{
    const char *hex = "0123456789abcdef";
    const char *special_chars = "btnvfr";

    da_Append(&ctx->output, '"');
    const char *s = v.items;
    for(size_t i = 0; i < v.count; i++) {
        uint8_t ch = ((uint8_t*)s)[i];
        if(ch == '"' || ch == '\\') {
            da_Append(&ctx->output, '\\');
            da_Append(&ctx->output, s[i]);
        } else if(ch >= '\b' && ch <= '\r') {
            da_Append(&ctx->output, '\\');
            da_Append(&ctx->output, special_chars[ch - '\b']);
        } else if(ch >= 0x20 && ch <= 0x7f) {
            da_Append(&ctx->output, s[i]);
        } else {
            uint8_t utf8len = VL__Utf8CharLen(ch);
            if(utf8len == 1) {
                sb_AppendBuf(&ctx->output, "\\u00", 4);
                da_Append(&ctx->output, hex[(ch >> 4) % 0x0f]);
                da_Append(&ctx->output, hex[ch % 0x0f]);
            } else {
                sb_AppendBuf(&ctx->output, s + i, utf8len);
                i += utf8len - 1;
            }
        }
    }
    da_Append(&ctx->output, '"');
}

//////////////////////////////////////////////

static void JSON_ElementBegin(vl_serialize_context *ctx)
{
    if(ctx->scopes.count > 0) {
        vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
        if(scope->prev_needs_comma && !scope->did_key) {
            da_Append(&ctx->output, ',');
        }
        if(ctx->indent) {
            if(scope->did_key) da_Append(&ctx->output, ' ');
            else {
                sb_Appendf(&ctx->output, "\n%*s", (int)(ctx->scopes.count*ctx->indent), "");
            }
        }
    }
}

static void JSON_ElementEnd(vl_serialize_context *ctx)
{
    if(ctx->scopes.count > 0) {
        vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
        scope->prev_needs_comma = true;
        scope->did_key = false;
    }
}

static void JSON_ObjectBegin(vl_serialize_context *ctx)
{
    ctx->ElementBegin(ctx);
    da_Append(&ctx->output, '{');
    VL__SerializeScopePush(ctx, SerializeScope_Object);
}

static void JSON_AttributeNameView(vl_serialize_context *ctx, view name)
{
    ctx->ElementBegin(ctx);
    AssertMsg(ctx->scopes.count > 0, "Error: Must be in an object before adding an attribute");
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(scope->type == SerializeScope_Object, "Error: Must be in an object not an array");
    AssertMsg(!scope->did_key, "Error: Can only have key-value pairs, no double keys");
    
    VL__SerializeViewNoElement(ctx, name);
    da_Append(&ctx->output, ':');
    scope->did_key = true;
}

static void JSON_ObjectEnd(vl_serialize_context *ctx)
{
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(scope->type == SerializeScope_Object, "Error: Last json element was not an array and called VL_ArrayEnd");
    if(ctx->indent > 0 && scope->prev_needs_comma) {
        sb_Appendf(&ctx->output, "\n%*s", (int)(ctx->indent*(ctx->scopes.count - 1)), "");
    }
    da_Append(&ctx->output, '}');
    VL__SerializeScopePop(ctx);
    ctx->ElementEnd(ctx);
}

static void JSON_ArrayBeginView(vl_serialize_context *ctx, view v)
{
    (void)v; // NOTE: JSON doesn't give a name to array elements
    ctx->ElementBegin(ctx);
    da_Append(&ctx->output, '[');
    VL__SerializeScopePush(ctx, SerializeScope_Array);
}

static void JSON_ArrayEnd(vl_serialize_context *ctx)
{
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(scope->type == SerializeScope_Array, "Error: Last json element was not an array and called VL_ArrayEnd");
    if(ctx->indent > 0 && scope->prev_needs_comma) {
        sb_Appendf(&ctx->output, "\n%*s", (int)(ctx->indent*(ctx->scopes.count - 1)), "");
    }
    da_Append(&ctx->output, ']');
    VL__SerializeScopePop(ctx);
    ctx->ElementEnd(ctx);
}

//////////////////////////////////////////////

static void XML_ElementBegin(vl_serialize_context *ctx)
{
    if(ctx->scopes.count > 0) {
        vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
        if(scope->prev_needs_comma) {
            if(ctx->indent) {
                if(!scope->did_key) sb_Appendf(&ctx->output, "\n%*s", (int)(ctx->indent*(ctx->scopes.count - 1)), "");
            }
        }

        if(scope->type == SerializeScope_Array) {
            if(view_Eq(scope->array_elem_name, VIEW(""))) {
                sb_Appendf(&ctx->output, "<element %u>", scope->array_elem_idx);
            } else {
                sb_Appendf(&ctx->output, "<"VIEW_FMT">", VIEW_ARG(scope->array_elem_name));
            }
            scope->array_elem_idx++;
        }
    }
}

static void XML_ElementEnd(vl_serialize_context *ctx)
{
    AssertMsg(ctx->scopes.count > 0, "Must be in a scope to call ElementEnd");
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    scope->prev_needs_comma = true;
    scope->did_key = false;
    if(ctx->should_pop_scope) {
        VL__SerializeScopePop(ctx);
        ctx->should_pop_scope = false;
        if(ctx->scopes.count == 0) return;
        
        if(ctx->indent > 0) {
            if(((scope->type == SerializeScope_Array) && (scope->array_elem_idx != 0)) ||
               ((scope->type == SerializeScope_Object) && !view_Eq(scope->current_elem, VIEW(""))))
            {
                sb_Appendf(&ctx->output, "\n%*s", (int)(ctx->indent*(ctx->scopes.count - 1)), "");
            } 
        }

        scope = &ctx->scopes.items[ctx->scopes.count - 1];
        scope->did_key = false;
    }

    if(scope->type == SerializeScope_Array) {
        if(scope->array_elem_idx != 0) {
            if(view_Eq(scope->current_elem, VIEW(""))) {
                sb_Appendf(&ctx->output, "</element %u>", scope->array_elem_idx - 1);
            } else {
                sb_Appendf(&ctx->output, "</"VIEW_FMT">", VIEW_ARG(scope->array_elem_name));
            }
        }
    } else {
        if(!view_Eq(scope->current_elem, VIEW(""))) {
            sb_Appendf(&ctx->output, "</"VIEW_FMT">", VIEW_ARG(scope->current_elem));
        }
    }

    //if(ctx->scopes.count > 0) {
    //    scope->prev_needs_comma = false;
    //}
}

static void XML_ObjectBegin(vl_serialize_context *ctx)
{
    ctx->ElementBegin(ctx);
    vl_serialize_scope *scope = VL__SerializeScopePush(ctx, SerializeScope_Object);
    scope->prev_needs_comma = true;
}

static void XML_AttributeNameView(vl_serialize_context *ctx, view name)
{
    ctx->ElementBegin(ctx);
    AssertMsg(ctx->scopes.count > 0, "Error: Must be in a scope before adding an attribute");
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(!scope->did_key, "Error: Can only have key-value pairs, no double keys");
    
    sb_Appendf(&ctx->output, "<"VIEW_FMT">", VIEW_ARG(name));
    scope->current_elem = name;
    scope->prev_needs_comma = false;
    scope->did_key = true;
}

static void XML_ObjectEnd(vl_serialize_context *ctx)
{
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(scope->type == SerializeScope_Object, "Error: Last json element was not an array and called VL_ArrayEnd");
    ctx->should_pop_scope = true;
    ctx->ElementEnd(ctx);
    scope = &ctx->scopes.items[ctx->scopes.count - 1];
    scope->prev_needs_comma = true;
}

static void XML_ArrayBeginView(vl_serialize_context *ctx, view elem_name)
{
    ctx->ElementBegin(ctx);
    vl_serialize_scope *scope = VL__SerializeScopePush(ctx, SerializeScope_Array);
    scope->array_elem_name = elem_name;
    scope->prev_needs_comma = true;
}

static void XML_ArrayEnd(vl_serialize_context *ctx)
{
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(scope->type == SerializeScope_Array, "Error: Outer scope was not an array and called VL_ArrayEnd");
    ctx->should_pop_scope = true;
    ctx->ElementEnd(ctx);
    vl_serialize_scope *outer_scope = &ctx->scopes.items[ctx->scopes.count - 1];
    outer_scope->prev_needs_comma = true;
//    outer_scope->prev_needs_comma = false;
}

SERIALIZE_PROC vl_serialize_context GetSerializeContext_Impl(GetSerializeContext_opts opt)
{
    vl_serialize_context result = {0};
    result.indent = opt.indent;
    result.type = opt.type;
    switch(opt.type) {
        case SerializeType_JSON: {
            result.ObjectBegin = JSON_ObjectBegin;
            result.AttributeNameView = JSON_AttributeNameView;
            result.ObjectEnd = JSON_ObjectEnd;

            result.ArrayBeginView = JSON_ArrayBeginView;
            result.ArrayEnd = JSON_ArrayEnd;

            result.ElementBegin = JSON_ElementBegin;
            result.ElementEnd = JSON_ElementEnd;
            result.should_quote_strings = true;
        } break;

        case SerializeType_XML: {
            result.ObjectBegin = XML_ObjectBegin;
            result.AttributeNameView = XML_AttributeNameView;
            result.ObjectEnd = XML_ObjectEnd;

            result.ArrayBeginView = XML_ArrayBeginView;
            result.ArrayEnd = XML_ArrayEnd;

            result.ElementBegin = XML_ElementBegin;
            result.ElementEnd = XML_ElementEnd;
            result.should_quote_strings = false;
        } break;

        default: {
            AssertMsg(false, "Unknown serialize type");
        } break;
    }

    return result;
}

SERIALIZE_PROC void VL_AttributeName(vl_serialize_context *ctx, const char *name)
{
    ctx->AttributeNameView(ctx, view_FromCstr(name));
}

SERIALIZE_PROC void VL_ArrayBegin_Impl(struct VL_ArrayBegin_opts opt)
{
    opt.ctx->ArrayBeginView(opt.ctx,
        opt.elem_name ? view_FromCstr(opt.elem_name) : VIEW(""));
}

SERIALIZE_PROC void VL_SerializeNull(vl_serialize_context *ctx)
{
    ctx->ElementBegin(ctx);
    sb_Appendf(&ctx->output, "null");
    ctx->ElementEnd(ctx);
}

SERIALIZE_PROC void VL_SerializeBool(vl_serialize_context *ctx, bool b)
{
    ctx->ElementBegin(ctx);
    sb_Appendf(&ctx->output, b ? "true" : "false");
    ctx->ElementEnd(ctx);
}

SERIALIZE_PROC void VL_SerializeInt(vl_serialize_context *ctx, int64_t val)
{
    ctx->ElementBegin(ctx);
    sb_Appendf(&ctx->output, S64_Fmt, val);
    ctx->ElementEnd(ctx);
}

SERIALIZE_PROC void VL_SerializeFloat(vl_serialize_context *ctx, double val)
{
    ctx->ElementBegin(ctx);
    if(isnan(val - val)) { /* check if val is NaN or Inf */
        sb_Appendf(&ctx->output, "null");
    } else {
        sb_Appendf(&ctx->output, "%lf", val);
    }
    ctx->ElementEnd(ctx);
}

SERIALIZE_PROC void VL_SerializeString(vl_serialize_context *ctx, const char *s)
{
    VL_SerializeView(ctx, view_FromCstr(s));
}

SERIALIZE_PROC void VL_SerializeView(vl_serialize_context *ctx, view v)
{
    ctx->ElementBegin(ctx);
    if(ctx->should_quote_strings) {
        VL__SerializeViewNoElement(ctx, v);
    } else {
        sb_Appendf(&ctx->output, VIEW_FMT, VIEW_ARG(v));
    }
    ctx->ElementEnd(ctx);
}

#endif // VL_SERIALIZE_IMPLEMENTATION
#endif // VL_SERIALIZE_H