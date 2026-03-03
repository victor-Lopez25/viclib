#define VL_REBUILD_URSELF(bin_path, src_path) VL_DEFAULT_REBUILD_URSELF(bin_path, src_path), VL_CC_DEBUG_INFO
#define VL_SERIALIZE_IMPLEMENTATION
#include "../vl_serialize.h"

#include <float.h>

int main(int argc, char **argv)
{
    VL_GO_REBUILD_URSELF(argc, argv, "vl_serialize.h");

    char data[] = 
        "{\n"
        "  \"somenullobj\": null,\n"
        "  \"boolean-values\": [\n"
        "    true,\n"
        "    false\n"
        "  ],\n"
        "  \"integers\": [\n"
        "    0,\n"
        "    1,\n"
        "    2,\n"
        "    3\n"
        "  ],\n"
        "  \"string-values\": [\n"
        "    \"hyper!\",\n"
        "    \"In the view\"\n"
        "  ]\n"
        "}";

    vl_serialize_context ctx = GetDeserializeContext(SerializeType_JSON, .buffer = data, .buffer_size = sizeof(data));

    Assert(VL_ObjectBegin(&ctx));
        if(VL_AttributeName(&ctx, "somenullobj")) {
            Assert(VL_SerializeNull(&ctx));
        }

        if(VL_AttributeName(&ctx, "boolean-values")) {
            Assert(VL_ArrayBegin(&ctx));
                bool boolean;
                Assert(VL_SerializeOpBool(&ctx, &boolean) && boolean);
                Assert(VL_SerializeOpBool(&ctx, &boolean) && !boolean);
            Assert(VL_ArrayEnd(&ctx));
        }

        if(VL_AttributeName(&ctx, "integers")) {
            Assert(VL_ArrayBegin(&ctx));
                int64_t val;
                for(int i = 0; i < 4; i++) {
                    Assert(VL_SerializeOpInt(&ctx, &val) && (val == i));
                }
            Assert(VL_ArrayEnd(&ctx));
        }

        if(VL_AttributeName(&ctx, "string-values")) {
            Assert(VL_ArrayBegin(&ctx));
                char *s;
                Assert(VL_SerializeOpString(&ctx, &s) && !strcmp(s, "hyper!"));
                free(s); // remember to free this!

                view v;
                Assert(VL_SerializeOpView(&ctx, &v) && ViewEq(v, VIEW("In the view")));
            Assert(VL_ArrayEnd(&ctx));
        }
    Assert(VL_ObjectEnd(&ctx));

#if 0
    //vl_serialize_context ctx = GetSerializeContext(SerializeType_JSON, .indent = 2, .float_fmt = "%+lg");
    //vl_serialize_context ctx = GetSerializeContext(SerializeType_XML, .indent = 2);
    //vl_serialize_context ctx = GetSerializeContext(SerializeType_TOML, .indent = 2);
    vl_serialize_context ctx = GetSerializeContext(SerializeType_C99_Initializer, .indent = 2);

    VL_ObjectBegin(&ctx);
        VL_AttributeName(&ctx, "somenullobj");
        VL_SerializeNull(&ctx);

        VL_AttributeName(&ctx, "boolean-values");
        VL_ArrayBegin(&ctx);
            VL_SerializeBool(&ctx, true);
            VL_SerializeBool(&ctx, false);
        VL_ArrayEnd(&ctx);

        VL_AttributeName(&ctx, "integers");
        VL_ArrayBegin(&ctx);
            for(int i = 0; i < 4; i++) {
                VL_SerializeInt(&ctx, i);
            }
        VL_ArrayEnd(&ctx);

        VL_AttributeName(&ctx, "floating-point-values");
        VL_ArrayBegin(&ctx);
            VL_SerializeFloat(&ctx, 0.0);
            VL_SerializeFloat(&ctx, -0.0);
            VL_SerializeFloat(&ctx, 134.576);
            VL_SerializeFloat(&ctx, NAN);
            VL_SerializeFloat(&ctx, INFINITY);
        VL_ArrayEnd(&ctx);

        VL_AttributeName(&ctx, "string-values");
        VL_ArrayBegin(&ctx);
            VL_SerializeString(&ctx, "hyper!");
            VL_SerializeView(&ctx, VIEW("In the view"));
        VL_ArrayEnd(&ctx);

        VL_AttributeName(&ctx, "another-object");
        VL_ObjectBegin(&ctx);
            VL_AttributeName(&ctx, "empty-array");
            VL_ArrayBegin(&ctx);
            VL_ArrayEnd(&ctx);

            VL_AttributeName(&ctx, "empty-object");
            VL_ObjectBegin(&ctx);
            VL_ObjectEnd(&ctx);
        VL_ObjectEnd(&ctx);

        VL_AttributeName(&ctx, "deep-object");
        VL_ObjectBegin(&ctx);
            VL_AttributeName(&ctx, "deeper-object");
            VL_ObjectBegin(&ctx);
                VL_AttributeName(&ctx, "deeperer-object");
                VL_ObjectBegin(&ctx);
                    VL_AttributeName(&ctx, "deep-arr");
                    VL_ArrayBegin(&ctx);
                        VL_SerializeString(&ctx, "abcdef");
                        VL_SerializeString(&ctx, "ghijkl");
                    VL_ArrayEnd(&ctx);

                    VL_AttributeName(&ctx, "value");
                    VL_SerializeInt(&ctx, 10);
                VL_ObjectEnd(&ctx);
            VL_ObjectEnd(&ctx);
        VL_ObjectEnd(&ctx);

    VL_ObjectEnd(&ctx);

#endif

    return 0;
}