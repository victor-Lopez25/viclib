#define VL_REBUILD_URSELF(bin_path, src_path) VL_DEFAULT_REBUILD_URSELF(bin_path, src_path), VL_CC_DEBUG_INFO
#define VL_SERIALIZE_IMPLEMENTATION
#include "../vl_serialize.h"

#include <float.h>

int main(int argc, char **argv)
{
    VL_GO_REBUILD_URSELF(argc, argv, "vl_serialize.h");

    //vl_serialize_context ctx = GetSerializeContext(SerializeType_JSON, .indent = 2);
    //vl_serialize_context ctx = GetSerializeContext(SerializeType_XML, .indent = 2);
    vl_serialize_context ctx = GetSerializeContext(SerializeType_TOML, .indent = 2);

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
            VL_AttributeName(&ctx, "empty array");
            VL_ArrayBegin(&ctx);
            VL_ArrayEnd(&ctx);

            VL_AttributeName(&ctx, "empty object");
            VL_ObjectBegin(&ctx);
            VL_ObjectEnd(&ctx);
        VL_ObjectEnd(&ctx);

        VL_AttributeName(&ctx, "deep object");
        VL_ObjectBegin(&ctx);
            VL_AttributeName(&ctx, "deeper object");
            VL_ObjectBegin(&ctx);
                VL_AttributeName(&ctx, "deeperer object");
                VL_ObjectBegin(&ctx);
                    VL_AttributeName(&ctx, "deep arr");
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

    printf(VIEW_FMT"\n", VIEW_ARG(ctx.output));

    return 0;
}