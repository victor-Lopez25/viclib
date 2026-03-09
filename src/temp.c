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
        "  \"floating-point-values\": [\n"
        "    0.0,\n"
        "    -0.0,\n"
        "    134.576,\n"
        "    null,\n"
        "    null\n"
        "  ]\n"
        "  \"another-object\": {\n"
        "    \"empty-array\": [],\n"
        "    \"empty-object\": {}\n"
        "  },\n"
        "  \"deep-object\": {\n"
        "    \"deeper-object\": {\n"
        "      \"deeperer-object\": {\n"
        "        \"deep-arr\": [\n"
        "          \"abcdef\",\n"
        "          \"ghijkl\"\n"
        "        ],\n"
        "        \"value\": 10\n"
        "      }\n"
        "    }\n"
        "  }\n"
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

    VL_AttributeName(&ctx, "floating-point-values");
    VL_ArrayBegin(&ctx);
        double val;
        // NOTE: float parsing is pretty bad in viclib.h (specifically for exponent parsing)
        //       will rework soon
        Assert(VL_SerializeOpFloat(&ctx, &val));
        Assert(VL_SerializeOpFloat(&ctx, &val));
        Assert(VL_SerializeOpFloat(&ctx, &val));
        Assert(VL_SerializeOpFloat(&ctx, &val));
        Assert(VL_SerializeOpFloat(&ctx, &val));
    VL_ArrayEnd(&ctx);

    if(VL_AttributeName(&ctx, "another-object")) {
        Assert(VL_ObjectBegin(&ctx));
            if(VL_AttributeName(&ctx, "empty-array")) {
                Assert(VL_ArrayBegin(&ctx));
                Assert(VL_ArrayEnd(&ctx));
            }

            if(VL_AttributeName(&ctx, "empty-object")) {
                Assert(VL_ObjectBegin(&ctx));
                Assert(VL_ObjectEnd(&ctx));
            }
        Assert(VL_ObjectEnd(&ctx));
    }

    if(VL_AttributeName(&ctx, "deep-object")) {
        Assert(VL_ObjectBegin(&ctx));
        if(VL_AttributeName(&ctx, "deeper-object")) {
            Assert(VL_ObjectBegin(&ctx));
            if(VL_AttributeName(&ctx, "deeperer-object")) {
                Assert(VL_ObjectBegin(&ctx));
                    if(VL_AttributeName(&ctx, "deep-arr")) {
                        Assert(VL_ArrayBegin(&ctx));
                        view v;
                        Assert(VL_SerializeOpView(&ctx, &v));
                        Assert(VL_SerializeOpView(&ctx, &v));
                        Assert(VL_ArrayEnd(&ctx));
                    }

                    if(VL_AttributeName(&ctx, "value")) {
                        int64_t val;
                        Assert(VL_SerializeOpInt(&ctx, &val));
                    }
                Assert(VL_ObjectEnd(&ctx));
            }
            Assert(VL_ObjectEnd(&ctx));
        }
        Assert(VL_ObjectEnd(&ctx));
    }
    Assert(VL_ObjectEnd(&ctx));

    return 0;
}