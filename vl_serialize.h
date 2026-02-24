// [vl_serialize.h](https://github.com/victor-Lopez25/viclib) © 2024 by [Víctor López Cortés](https://github.com/victor-Lopez25) is licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0)
// version: 1.2.2
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

// TODO: Remove trailing newlines (at the end) of TOML generation

#ifndef DEFAULT_PARSE_CHUNK_SIZE
#define DEFAULT_PARSE_CHUNK_SIZE 4096
#endif // DEFAULT_PARSE_CHUNK_SIZE

#ifndef MIN_PARSE_CHUNK_SIZE
#define MIN_PARSE_CHUNK_SIZE 1024
#endif // MIN_PARSE_CHUNK_SIZE

#if DEFAULT_PARSE_CHUNK_SIZE < MIN_PARSE_CHUNK_SIZE
#error MIN_PARSE_CHUNK_SIZE must be less than DEFAULT_PARSE_CHUNK_SIZE
#endif

typedef enum {
    SerializeType_JSON,
    SerializeType_C99_Initializer,
    SerializeType_XML,
    SerializeType_TOML,
} vl_serialize_type;

typedef enum {
    SerializeScope_Object,
    SerializeScope_Array,
} vl_serialize_scope_type;

typedef struct {
    vl_serialize_scope_type type;
    bool prev_needs_comma;
    bool did_key;
    bool scope_needs_newline; /* for TOML */
    view current_elem;
    view array_elem_name;
    uint32_t array_elem_idx;
} vl_serialize_scope;

typedef struct {
    vl_serialize_type type;
    uint8_t indent;
    const char *float_fmt; // default is "%lf"
} GetSerializeContext_opts;

typedef struct {
    vl_serialize_type type;
    const char *buffer;
    uint32_t buffer_size;

    const char *filename;
    bool buffered_reads;
} GetDeserializeContext_opts;

typedef struct vl_serialize_context vl_serialize_context;
struct vl_serialize_context {
    vl_serialize_type type;
    string_builder output;

    struct {
        vl_serialize_scope *items;
        size_t count;
        size_t capacity;
    } scopes;

    union {
        struct {
            view current_elem;
            const char *float_fmt;
            uint8_t indent;
            bool should_quote_strings;
            
            union {
                struct {
                    bool ignore_element_begin;
                    bool done_first_object;
                    bool should_push_scope;
                    vl_serialize_scope_type scope_type_to_push;
                    size_t count_non_newline_scopes;
                } TOML;
                struct {
                    bool should_pop_scope;
                } XML;
            } as;
        } serialize;

        struct {
            vl_file_chunk file_chunk;
            const char *filename;
            size_t current_chunk_size;
            size_t used_buffer_size;

            bool fatal_error;
            bool must_free_chunk_buffer;
        } deserialize;
    } as;

    bool (*ObjectBegin)(vl_serialize_context *ctx);
    bool (*AttributeNameView)(vl_serialize_context *ctx, view name);
    bool (*ObjectEnd)(vl_serialize_context *ctx);
    bool (*ArrayBeginView)(vl_serialize_context *ctx, view name);
    bool (*ArrayEnd)(vl_serialize_context *ctx);

    // TODO: SerializeOpNull ????
    // TODO: SerializeOpBool
    // TODO: SerializeOpInt
    // TODO: SerializeOpFloat
    // TODO: SerializeOpString
    // TODO: SerializeOpView

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

#define GetDeserializeContext(Type, ...) \
    GetDeserializeContext_Impl((GetDeserializeContext_opts){.type = Type, __VA_ARGS__})
SERIALIZE_PROC vl_serialize_context GetDeserializeContext_Impl(GetDeserializeContext_opts opt);

SERIALIZE_PROC bool VL_ArrayBegin_Impl(struct VL_ArrayBegin_opts opt);
SERIALIZE_PROC bool VL_AttributeName(vl_serialize_context *ctx, const char *name);

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
    DaAppend(&ctx->scopes, scope);
    return &ctx->scopes.items[ctx->scopes.count - 1];
}

static void VL__SerializeScopePop(vl_serialize_context *ctx)
{
    Assert(ctx->scopes.count > 0);
    ctx->scopes.count--;
    mem_zero(&ctx->scopes.items[ctx->scopes.count], sizeof(*ctx->scopes.items));
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

    const char *s = v.items;
    for(size_t i = 0; i < v.count; i++) {
        uint8_t ch = ((uint8_t*)s)[i];
        if(ch == '"' || ch == '\\') {
            DaAppend(&ctx->output, '\\');
            DaAppend(&ctx->output, s[i]);
        } else if(ch >= '\b' && ch <= '\r') {
            DaAppend(&ctx->output, '\\');
            DaAppend(&ctx->output, special_chars[ch - '\b']);
        } else if(ch >= 0x20 && ch <= 0x7f) {
            DaAppend(&ctx->output, s[i]);
        } else {
            uint8_t utf8len = VL__Utf8CharLen(ch);
            if(utf8len == 1) {
                SbAppendBuf(&ctx->output, "\\u00", 4);
                DaAppend(&ctx->output, hex[(ch >> 4) % 0x0f]);
                DaAppend(&ctx->output, hex[ch % 0x0f]);
            } else {
                SbAppendBuf(&ctx->output, s + i, utf8len);
                i += utf8len - 1;
            }
        }
    }
}

//////////////////////////////////////////////

static void JSON_ElementBegin(vl_serialize_context *ctx)
{
    if(ctx->scopes.count > 0) {
        vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
        if(scope->prev_needs_comma && !scope->did_key) {
            DaAppend(&ctx->output, ',');
        }
        if(ctx->as.serialize.indent) {
            if(scope->did_key) DaAppend(&ctx->output, ' ');
            else {
                SbAppendf(&ctx->output, "\n%*s", (int)(ctx->scopes.count*ctx->as.serialize.indent), "");
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

static bool JSON_ObjectBegin(vl_serialize_context *ctx)
{
    ctx->ElementBegin(ctx);
    DaAppend(&ctx->output, '{');
    VL__SerializeScopePush(ctx, SerializeScope_Object);

    return true;
}

static bool JSON_AttributeNameView(vl_serialize_context *ctx, view name)
{
    ctx->ElementBegin(ctx);
    AssertMsg(ctx->scopes.count > 0, "Error: Must be in an object before adding an attribute");
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(scope->type == SerializeScope_Object, "Error: Must be in an object to make `key = value` pairs");
    AssertMsg(!scope->did_key, "Error: Can only have key-value pairs, no double keys");
    
    DaAppend(&ctx->output, '"');
    VL__SerializeViewNoElement(ctx, name);
    SbAppendf(&ctx->output, "\":");
    scope->did_key = true;

    return true;
}

static bool JSON_ObjectEnd(vl_serialize_context *ctx)
{
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(scope->type == SerializeScope_Object, "Error: Last element was not an array and called VL_ObjectEnd");
    if(ctx->as.serialize.indent > 0 && scope->prev_needs_comma) {
        SbAppendf(&ctx->output, "\n%*s", (int)(ctx->as.serialize.indent*(ctx->scopes.count - 1)), "");
    }
    DaAppend(&ctx->output, '}');
    VL__SerializeScopePop(ctx);
    ctx->ElementEnd(ctx);

    return true;
}

static bool JSON_ArrayBeginView(vl_serialize_context *ctx, view v)
{
    (void)v; // NOTE: JSON doesn't give a name to array elements
    ctx->ElementBegin(ctx);
    DaAppend(&ctx->output, '[');
    VL__SerializeScopePush(ctx, SerializeScope_Array);

    return true;
}

static bool JSON_ArrayEnd(vl_serialize_context *ctx)
{
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(scope->type == SerializeScope_Array, "Error: Last json element was not an array and called VL_ArrayEnd");
    if(ctx->as.serialize.indent > 0 && scope->prev_needs_comma) {
        SbAppendf(&ctx->output, "\n%*s", (int)(ctx->as.serialize.indent*(ctx->scopes.count - 1)), "");
    }
    DaAppend(&ctx->output, ']');
    VL__SerializeScopePop(ctx);
    ctx->ElementEnd(ctx);

    return true;
}

//////////////////////////////////////////////

static void C99_Initializer_ElementBegin(vl_serialize_context *ctx)
{
    if(ctx->scopes.count > 0) {
        vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
        if(scope->prev_needs_comma && !scope->did_key) {
            DaAppend(&ctx->output, ',');
        }

        if(scope->did_key) SbAppendf(&ctx->output, ctx->as.serialize.indent ? " = " : "=");
        else if(ctx->as.serialize.indent) {
            SbAppendf(&ctx->output, "\n%*s", (int)(ctx->scopes.count*ctx->as.serialize.indent), "");
        }
    }
}

static bool C99_Initializer_AttributeNameView(vl_serialize_context *ctx, view name)
{
    ctx->ElementBegin(ctx);
    AssertMsg(ctx->scopes.count > 0, "Error: Must be in an object before adding an attribute");
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(scope->type == SerializeScope_Object, "Error: Must be in an object to make `key = value` pairs");
    AssertMsg(!scope->did_key, "Error: Can only have key-value pairs, no double keys");
    
    DaAppend(&ctx->output, '.');
    VL__SerializeViewNoElement(ctx, name);
    scope->did_key = true;

    return true;
}

static bool C99_Initializer_ArrayBeginView(vl_serialize_context *ctx, view v)
{
    (void)v; // NOTE: C99 initializers don't give a name to array elements
    ctx->ElementBegin(ctx);
    DaAppend(&ctx->output, '{');
    VL__SerializeScopePush(ctx, SerializeScope_Array);

    return true;
}

static bool C99_Initializer_ArrayEnd(vl_serialize_context *ctx)
{
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(scope->type == SerializeScope_Array, "Error: Last C literal element was not an array and called VL_ArrayEnd");
    if(ctx->as.serialize.indent > 0 && scope->prev_needs_comma) {
        SbAppendf(&ctx->output, "\n%*s", (int)(ctx->as.serialize.indent*(ctx->scopes.count - 1)), "");
    }
    DaAppend(&ctx->output, '}');
    VL__SerializeScopePop(ctx);
    ctx->ElementEnd(ctx);

    return true;
}

//////////////////////////////////////////////

static void XML_ElementBegin(vl_serialize_context *ctx)
{
    if(ctx->scopes.count > 0) {
        vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
        if(scope->prev_needs_comma) {
            if(ctx->as.serialize.indent) {
                if(!scope->did_key) SbAppendf(&ctx->output, "\n%*s", (int)(ctx->as.serialize.indent*(ctx->scopes.count - 1)), "");
            }
        }

        if(scope->type == SerializeScope_Array) {
            if(ViewEq(scope->array_elem_name, VIEW(""))) {
                SbAppendf(&ctx->output, "<element %u>", scope->array_elem_idx);
            } else {
                SbAppendf(&ctx->output, "<"VIEW_FMT">", VIEW_ARG(scope->array_elem_name));
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
    if(ctx->as.serialize.as.XML.should_pop_scope) {
        ctx->as.serialize.as.XML.should_pop_scope = false;

        vl_serialize_scope_type popped_scope_type = scope->type;
        view popped_scope_current_elem = scope->current_elem;
        size_t popped_scope_array_elem_idx = scope->array_elem_idx;
        VL__SerializeScopePop(ctx);
        if(ctx->scopes.count == 0) return;
        
        if(ctx->as.serialize.indent > 0) {
            if(((popped_scope_type == SerializeScope_Array) && 
                (popped_scope_array_elem_idx != 0)) ||
               ((popped_scope_type == SerializeScope_Object) && 
                !ViewEq(popped_scope_current_elem, VIEW(""))))
            {
                SbAppendf(&ctx->output, "\n%*s", (int)(ctx->as.serialize.indent*(ctx->scopes.count - 1)), "");
            } 
        }

        scope = &ctx->scopes.items[ctx->scopes.count - 1];
        scope->did_key = false;
    }

    if(scope->type == SerializeScope_Array) {
        if(scope->array_elem_idx != 0) {
            if(ViewEq(scope->current_elem, VIEW(""))) {
                SbAppendf(&ctx->output, "</element %u>", scope->array_elem_idx - 1);
            } else {
                SbAppendf(&ctx->output, "</"VIEW_FMT">", VIEW_ARG(scope->array_elem_name));
            }
        }
    } else {
        if(!ViewEq(scope->current_elem, VIEW(""))) {
            SbAppendf(&ctx->output, "</"VIEW_FMT">", VIEW_ARG(scope->current_elem));
        }
    }
}

static bool XML_ObjectBegin(vl_serialize_context *ctx)
{
    ctx->ElementBegin(ctx);
    vl_serialize_scope *scope = VL__SerializeScopePush(ctx, SerializeScope_Object);
    scope->prev_needs_comma = true;

    return true;
}

static bool XML_AttributeNameView(vl_serialize_context *ctx, view name)
{
    ctx->ElementBegin(ctx);
    AssertMsg(ctx->scopes.count > 0, "Error: Must be in a scope before adding an attribute");
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(!scope->did_key, "Error: Can only have key-value pairs, no double keys");
    
    SbAppendf(&ctx->output, "<"VIEW_FMT">", VIEW_ARG(name));
    scope->current_elem = name;
    scope->prev_needs_comma = false;
    scope->did_key = true;

    return true;
}

static bool XML_ObjectEnd(vl_serialize_context *ctx)
{
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(scope->type == SerializeScope_Object, "Error: Last XML element was not an array and called VL_ArrayEnd");
    ctx->as.serialize.as.XML.should_pop_scope = true;
    ctx->ElementEnd(ctx);
    scope = &ctx->scopes.items[ctx->scopes.count - 1];
    scope->prev_needs_comma = true;

    return true;
}

static bool XML_ArrayBeginView(vl_serialize_context *ctx, view elem_name)
{
    ctx->ElementBegin(ctx);
    vl_serialize_scope *scope = VL__SerializeScopePush(ctx, SerializeScope_Array);
    scope->array_elem_name = elem_name;
    scope->prev_needs_comma = true;

    return true;
}

static bool XML_ArrayEnd(vl_serialize_context *ctx)
{
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(scope->type == SerializeScope_Array, "Error: Outer scope was not an array and called VL_ArrayEnd");
    ctx->as.serialize.as.XML.should_pop_scope = true;
    ctx->ElementEnd(ctx);
    vl_serialize_scope *outer_scope = &ctx->scopes.items[ctx->scopes.count - 1];
    outer_scope->prev_needs_comma = true;

    return true;
}

//////////////////////////////////////////////

static void TOML_ElementBegin(vl_serialize_context *ctx)
{
    vl_serialize_scope_type outer_scope_type = 0;
    bool outer_scope_needs_newline = false;
    if(ctx->scopes.count > 0) {
        outer_scope_type = ctx->scopes.items[ctx->scopes.count - 1].type;
        outer_scope_needs_newline = ctx->scopes.items[ctx->scopes.count - 1].scope_needs_newline;
    }
    
    vl_serialize_scope *scope;
    if(ctx->as.serialize.as.TOML.should_push_scope) {
        scope = VL__SerializeScopePush(ctx, ctx->as.serialize.as.TOML.scope_type_to_push);
    } else {
        scope = &ctx->scopes.items[ctx->scopes.count - 1];
    }

    if(ctx->as.serialize.as.TOML.ignore_element_begin) {
        ctx->as.serialize.as.TOML.ignore_element_begin = false;
        return;
    }

    if(scope->prev_needs_comma) {
        DaAppend(&ctx->output, outer_scope_needs_newline ? '\n' : ',');
    }

    if(ctx->as.serialize.indent && !outer_scope_needs_newline) {
        SbAppendf(&ctx->output, "\n%*s", (int)(ctx->as.serialize.indent*ctx->as.serialize.as.TOML.count_non_newline_scopes), "");
    }

    if(outer_scope_type == SerializeScope_Object) {
        SbAppendf(&ctx->output, VIEW_FMT"%s", VIEW_ARG(ctx->as.serialize.current_elem),
            ctx->as.serialize.indent ? " = " : "=");
    }
}

static void TOML_ElementEnd(vl_serialize_context *ctx)
{
    if(ctx->scopes.count > 0) {
        vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
        scope->prev_needs_comma = true;

        if(scope->scope_needs_newline) {
            DaAppend(&ctx->output, '\n');
        }
    }
}

static bool TOML_ObjectBegin(vl_serialize_context *ctx)
{
    if(ctx->scopes.count <= 1) ctx->as.serialize.as.TOML.ignore_element_begin = true;
    ctx->as.serialize.as.TOML.scope_type_to_push = SerializeScope_Object;
    ctx->as.serialize.as.TOML.should_push_scope = true;
    ctx->ElementBegin(ctx);
    ctx->as.serialize.as.TOML.should_push_scope = false;

    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    if(ctx->scopes.count == 1) {
        scope->scope_needs_newline = true;
    } else if(ctx->scopes.count > 2 || ctx->scopes.items[1].type == SerializeScope_Array) {
        ctx->as.serialize.as.TOML.count_non_newline_scopes++;
        DaAppend(&ctx->output, '{');
    } else if(ctx->scopes.count == 2) {
        if(!ctx->as.serialize.as.TOML.done_first_object) {
            ctx->as.serialize.as.TOML.done_first_object = true;
            SbAppendf(&ctx->output, "\n["VIEW_FMT"]\n", VIEW_ARG(ctx->as.serialize.current_elem));
        } else {
            SbAppendf(&ctx->output, "["VIEW_FMT"]\n", VIEW_ARG(ctx->as.serialize.current_elem));
        }

        scope->scope_needs_newline = true;
    }

    return true;
}

static bool TOML_AttributeNameView(vl_serialize_context *ctx, view name)
{
    //ctx->ElementBegin(ctx);
    AssertMsg(ctx->scopes.count > 0, "Error: Must be in an object before adding an attribute");
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(scope->type == SerializeScope_Object, "Error: Must be in an object to make `key = value` pairs");
    ctx->as.serialize.current_elem = name;

    return true;
}

static bool TOML_ObjectEnd(vl_serialize_context *ctx)
{
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(scope->type == SerializeScope_Object, "Error: Last toml element was not an object and called VL_ObjectEnd");
    if(!scope->scope_needs_newline) {
        ctx->as.serialize.as.TOML.count_non_newline_scopes--;
        if(ctx->as.serialize.indent > 0 && scope->prev_needs_comma) {
            SbAppendf(&ctx->output, "\n%*s", (int)(ctx->as.serialize.indent*ctx->as.serialize.as.TOML.count_non_newline_scopes), "");
        }
        DaAppend(&ctx->output, '}');
    }
    
    VL__SerializeScopePop(ctx);
    ctx->ElementEnd(ctx);

    return true;
}

static bool TOML_ArrayBeginView(vl_serialize_context *ctx, view elem_name)
{
    (void)elem_name; // NOTE: TOML doesn't give a name to array elements
    AssertMsg(ctx->scopes.count > 0, "Error: An outmost scope of an array is not allowed in TOML");

    ctx->as.serialize.as.TOML.scope_type_to_push = SerializeScope_Array;
    ctx->as.serialize.as.TOML.should_push_scope = true;
    ctx->ElementBegin(ctx);
    ctx->as.serialize.as.TOML.should_push_scope = false;

    ctx->as.serialize.as.TOML.count_non_newline_scopes++;
    DaAppend(&ctx->output, '[');

    return true;
}

static bool TOML_ArrayEnd(vl_serialize_context *ctx)
{
    vl_serialize_scope *scope = &ctx->scopes.items[ctx->scopes.count - 1];
    AssertMsg(scope->type == SerializeScope_Array, "Error: Last toml element was not an array and called VL_ArrayEnd");
    ctx->as.serialize.as.TOML.count_non_newline_scopes--;
    if(ctx->as.serialize.indent > 0 && scope->prev_needs_comma) {
        SbAppendf(&ctx->output, "\n%*s", (int)(ctx->as.serialize.indent*ctx->as.serialize.as.TOML.count_non_newline_scopes), "");
    }
    DaAppend(&ctx->output, ']');
    VL__SerializeScopePop(ctx);
    ctx->ElementEnd(ctx);

    return true;
}

//////////////////////////////////////////////

SERIALIZE_PROC vl_serialize_context GetSerializeContext_Impl(GetSerializeContext_opts opt)
{
    vl_serialize_context result = {0};
    result.as.serialize.indent = opt.indent;
    result.type = opt.type;
    result.as.serialize.float_fmt = opt.float_fmt ? opt.float_fmt : "%lf";
    switch(opt.type) {
        case SerializeType_JSON: {
            result.ObjectBegin = JSON_ObjectBegin;
            result.AttributeNameView = JSON_AttributeNameView;
            result.ObjectEnd = JSON_ObjectEnd;

            result.ArrayBeginView = JSON_ArrayBeginView;
            result.ArrayEnd = JSON_ArrayEnd;

            result.ElementBegin = JSON_ElementBegin;
            result.ElementEnd = JSON_ElementEnd;
            result.as.serialize.should_quote_strings = true;
        } break;

        case SerializeType_C99_Initializer: {
            result.ObjectBegin = JSON_ObjectBegin;
            result.AttributeNameView = C99_Initializer_AttributeNameView;
            result.ObjectEnd = JSON_ObjectEnd;

            result.ArrayBeginView = C99_Initializer_ArrayBeginView;
            result.ArrayEnd = C99_Initializer_ArrayEnd;

            result.ElementBegin = C99_Initializer_ElementBegin;
            result.ElementEnd = JSON_ElementEnd;
            result.as.serialize.should_quote_strings = true;
        } break;

        case SerializeType_XML: {
            result.ObjectBegin = XML_ObjectBegin;
            result.AttributeNameView = XML_AttributeNameView;
            result.ObjectEnd = XML_ObjectEnd;

            result.ArrayBeginView = XML_ArrayBeginView;
            result.ArrayEnd = XML_ArrayEnd;

            result.ElementBegin = XML_ElementBegin;
            result.ElementEnd = XML_ElementEnd;
            result.as.serialize.should_quote_strings = false;
        } break;

        case SerializeType_TOML: {
            result.ObjectBegin = TOML_ObjectBegin;
            result.AttributeNameView = TOML_AttributeNameView;
            result.ObjectEnd = TOML_ObjectEnd;

            result.ArrayBeginView = TOML_ArrayBeginView;
            result.ArrayEnd = TOML_ArrayEnd;

            result.ElementBegin = TOML_ElementBegin;
            result.ElementEnd = TOML_ElementEnd;            
            result.as.serialize.should_quote_strings = true;
        } break;

        default: {
            AssertMsg(false, "Unknown serialize type");
        } break;
    }

    return result;
}

//////////////////////////////////////////////

static inline void VL__DeserializeFetchNextChunk(vl_serialize_context *ctx)
{
    if((ctx->as.deserialize.file_chunk.RemainingFileSize > 0) &&
       (ctx->as.deserialize.used_buffer_size > 
        (ctx->as.deserialize.current_chunk_size - MIN_PARSE_CHUNK_SIZE)))
    {
        size_t prev_remaining = 
            ctx->as.deserialize.current_chunk_size - ctx->as.deserialize.used_buffer_size;
        
        /* move start <-- end */
        mem_copy(ctx->as.deserialize.file_chunk.Buffer,
            ctx->as.deserialize.file_chunk.Buffer + ctx->as.deserialize.used_buffer_size,
            prev_remaining);

        uint8_t *buffer = ctx->as.deserialize.file_chunk.Buffer;
        size_t buffer_size = ctx->as.deserialize.file_chunk.BufferSize;

        ctx->as.deserialize.file_chunk.Buffer += prev_remaining;
        ctx->as.deserialize.file_chunk.BufferSize -= (uint32_t)prev_remaining;

        uint32_t curr_read_size;
        ReadFileChunk(&ctx->as.deserialize.file_chunk, ctx->as.deserialize.filename, &curr_read_size);

        ctx->as.deserialize.file_chunk.Buffer = buffer;
        ctx->as.deserialize.file_chunk.BufferSize = (uint32_t)buffer_size;
        ctx->as.deserialize.current_chunk_size = curr_read_size + prev_remaining;
    }
}

static bool VL__DeserializeGetRemaining(vl_serialize_context *ctx, view *remaining)
{
    if(ctx->as.deserialize.fatal_error) return false;

    VL__DeserializeFetchNextChunk(ctx);

    const char *start = (const char*)ctx->as.deserialize.file_chunk.Buffer + ctx->as.deserialize.used_buffer_size;
    size_t remaining_size = ctx->as.deserialize.current_chunk_size - ctx->as.deserialize.used_buffer_size;

    *remaining = ViewFromParts(start, remaining_size);
    *remaining = ViewTrimLeft(*remaining);

    return remaining_size > 0;
}

//////////////////////////////////////////////

static bool JSON_ExpectObjectBegin(vl_serialize_context *ctx)
{
    view remaining;
    if(!VL__DeserializeGetRemaining(ctx, &remaining)) return false;

    bool found_expected = (remaining.items[0] == '{');
    ctx->as.deserialize.fatal_error = !found_expected;
    ctx->as.deserialize.used_buffer_size = ctx->as.deserialize.current_chunk_size - (remaining.count - 1);
    return found_expected;
}

static bool JSON_ExpectAttributeName(vl_serialize_context *ctx, view name)
{
    view remaining;
    if(!VL__DeserializeGetRemaining(ctx, &remaining)) return false;

    bool found_expected = (remaining.items[0] == '"');
    ViewChopLeft(&remaining, 1);
    found_expected = found_expected && ViewChopStartsWith(&remaining, name);
    found_expected = found_expected && (remaining.count > 1) && (remaining.items[0] == '"');
    ViewChopLeft(&remaining, 1);
    remaining = ViewTrimLeft(remaining);
    found_expected = found_expected && (remaining.count > 1) && (remaining.items[0] == ':');
    
    if(found_expected) {
        ctx->as.deserialize.used_buffer_size = 
            ctx->as.deserialize.current_chunk_size - (remaining.count - 1);
    }
    return found_expected;
}

static bool JSON_ExpectObjectEnd(vl_serialize_context *ctx)
{
    view remaining;
    if(!VL__DeserializeGetRemaining(ctx, &remaining)) return false;

    bool found_expected = (remaining.items[0] == '}');
    ctx->as.deserialize.fatal_error = !found_expected;
    ctx->as.deserialize.used_buffer_size = ctx->as.deserialize.current_chunk_size - (remaining.count - 1);
    return found_expected;
}

static bool JSON_ExpectArrayBegin(vl_serialize_context *ctx, view name)
{
    (void)name;
    view remaining;
    if(!VL__DeserializeGetRemaining(ctx, &remaining)) return false;

    bool found_expected = (remaining.items[0] == '[');
    ctx->as.deserialize.fatal_error = !found_expected;    
    ctx->as.deserialize.used_buffer_size = ctx->as.deserialize.current_chunk_size - (remaining.count - 1);
    return found_expected;
}

static bool JSON_ExpectArrayEnd(vl_serialize_context *ctx)
{
    view remaining;
    if(!VL__DeserializeGetRemaining(ctx, &remaining)) return false;

    bool found_expected = (remaining.items[0] == ']');
    ctx->as.deserialize.fatal_error = !found_expected;    
    ctx->as.deserialize.used_buffer_size = ctx->as.deserialize.current_chunk_size - (remaining.count - 1);
    return found_expected;
}

//////////////////////////////////////////////

SERIALIZE_PROC vl_serialize_context GetDeserializeContext_Impl(GetDeserializeContext_opts opt)
{
    AssertMsgAlways(opt.buffer || opt.filename, "Error: Must be given a buffer with data or a file to read from");
    vl_serialize_context result = {0};
    result.type = opt.type;
    if(!opt.buffer) {
        if(opt.buffer_size < DEFAULT_PARSE_CHUNK_SIZE) {
            opt.buffer_size = DEFAULT_PARSE_CHUNK_SIZE;
        }
        opt.buffer = malloc(opt.buffer_size);
        mem_zero((void*)opt.buffer, opt.buffer_size);
        result.as.deserialize.must_free_chunk_buffer = true;
    } else if(opt.filename) {
        AssertMsgAlways(opt.buffer_size >= DEFAULT_PARSE_CHUNK_SIZE, "Error: buffer size must be at least DEFAULT_PARSE_CHUNK_SIZE size");
    }
    result.as.deserialize.file_chunk.Buffer = (uint8_t*)opt.buffer;
    result.as.deserialize.file_chunk.BufferSize = opt.buffer_size;

    if(opt.filename) {
        result.as.deserialize.filename = opt.filename;
        uint32_t curr_read_size;
        ReadFileChunk(&result.as.deserialize.file_chunk, opt.filename, &curr_read_size);
        result.as.deserialize.current_chunk_size = curr_read_size;
    } else {
        result.as.deserialize.current_chunk_size = opt.buffer_size;
    }

    switch(opt.type) {
        case SerializeType_JSON: {
            result.ObjectBegin = JSON_ExpectObjectBegin;
            result.AttributeNameView = JSON_ExpectAttributeName;
            result.ObjectEnd = JSON_ExpectObjectEnd;

            result.ArrayBeginView = JSON_ExpectArrayBegin;
            result.ArrayEnd = JSON_ExpectArrayEnd;
        } break;

        case SerializeType_XML:
        case SerializeType_TOML:
        case SerializeType_C99_Initializer: {
            AssertMsgAlways(false, "Deserializing not supported for chosen type");
        } break;

        default: {
            AssertMsg(false, "Unknown serialize type");
        } break;
    }

    return result;
}

//////////////////////////////////////////////

SERIALIZE_PROC bool VL_AttributeName(vl_serialize_context *ctx, const char *name)
{
    return ctx->AttributeNameView(ctx, ViewFromCstr(name));
}

SERIALIZE_PROC bool VL_ArrayBegin_Impl(struct VL_ArrayBegin_opts opt)
{
    return opt.ctx->ArrayBeginView(opt.ctx,
            opt.elem_name ? ViewFromCstr(opt.elem_name) : VIEW(""));
}

SERIALIZE_PROC void VL_SerializeNull(vl_serialize_context *ctx)
{
    ctx->ElementBegin(ctx);
    SbAppendf(&ctx->output, "null");
    ctx->ElementEnd(ctx);
}

SERIALIZE_PROC void VL_SerializeBool(vl_serialize_context *ctx, bool b)
{
    ctx->ElementBegin(ctx);
    SbAppendf(&ctx->output, b ? "true" : "false");
    ctx->ElementEnd(ctx);
}

SERIALIZE_PROC void VL_SerializeInt(vl_serialize_context *ctx, int64_t val)
{
    ctx->ElementBegin(ctx);
    SbAppendf(&ctx->output, S64_Fmt, val);
    ctx->ElementEnd(ctx);
}

SERIALIZE_PROC void VL_SerializeFloat(vl_serialize_context *ctx, double val)
{
    ctx->ElementBegin(ctx);
    if(isnan(val - val)) { /* check if val is NaN or Inf */
        SbAppendf(&ctx->output, "null");
    } else {
        SbAppendf(&ctx->output, ctx->as.serialize.float_fmt, val);
    }
    ctx->ElementEnd(ctx);
}

SERIALIZE_PROC void VL_SerializeString(vl_serialize_context *ctx, const char *s)
{
    VL_SerializeView(ctx, ViewFromCstr(s));
}

SERIALIZE_PROC void VL_SerializeView(vl_serialize_context *ctx, view v)
{
    ctx->ElementBegin(ctx);
    if(ctx->as.serialize.should_quote_strings) {
        DaAppend(&ctx->output, '"');
        VL__SerializeViewNoElement(ctx, v);
        DaAppend(&ctx->output, '"');
    } else {
        SbAppendf(&ctx->output, VIEW_FMT, VIEW_ARG(v));
    }
    ctx->ElementEnd(ctx);
}

#endif // VL_SERIALIZE_IMPLEMENTATION
#endif // VL_SERIALIZE_H