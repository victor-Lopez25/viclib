#define VL_BUILD_IMPLEMENTATION
#include "../vl_build.h"

#if OS_WINDOWS
#define GCC_OUT_DIRECTORY "mingw-gcc"
#else
#define GCC_OUT_DIRECTORY "gcc"
#endif

bool CompileLib(vl_cmd *cmd, vl_compile_ctx *ctx, const char *name)
{
    ctx->output = name;
    ctx->sourceFiles.count = 0;
    DaAppend(&ctx->sourceFiles, temp_sprintf("../src/%s.c", name));

#if OS_WINDOWS
    ctx->cc = CCompiler_MSVC;
    ctx->type = Compile_StaticLibrary;
    ctx->outputDir = "msvc";
    if(!VL_CCompile(cmd, ctx)) {
        fprintf(stderr, "Could not build %s.lib", name);
        return false;
    }

    ctx->type = Compile_DynamicLibrary;
    if(!VL_CCompile(cmd, ctx)) {
        fprintf(stderr, "Could not build %s.dll", name);
        return false;
    }
#endif

    ctx->outputDir = GCC_OUT_DIRECTORY;
    ctx->cc = CCompiler_GCC;
    ctx->type = Compile_StaticLibrary;
    if(!VL_CCompile(cmd, ctx)) {
        fprintf(stderr, "Could not build "GCC_OUT_DIRECTORY"/lib%s.a", name);
        return false;
    }

    ctx->type = Compile_DynamicLibrary;
    if(!VL_CCompile(cmd, ctx)) {
        fprintf(stderr, "Could not build "GCC_OUT_DIRECTORY"%s" VL_DLL_EXT, name);
        return false;
    }

    return true;
}

int main(int argc, char **argv)
{
    VL_GO_REBUILD_URSELF(argc, argv, "viclib.h", "vl_build.h");

    MkdirIfNotExist("bin");
    VL_Pushd("bin");

    vl_cmd cmd = {0};
    vl_compile_ctx ctx = {
        .cc = CCompiler_MSVC,
        .type = Compile_StaticLibrary,
        .debug = true,
        .warnings = true,
        .warningsAsErrors = true,
    };

#if defined(_WIN32)
    MkdirIfNotExist("msvc");
#endif
    MkdirIfNotExist(GCC_OUT_DIRECTORY);

    CompileLib(&cmd, &ctx, "viclib");
    CompileLib(&cmd, &ctx, "vl_build");
    CompileLib(&cmd, &ctx, "vl_serialize");

    VL_Popd();
    return 0;
}