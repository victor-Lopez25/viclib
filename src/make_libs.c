#define VL_BUILD_IMPLEMENTATION
#include "../vl_build.h"

#if OS_WINDOWS
#define GCC_OUT_DIRECTORY "mingw-gcc"
#else
#define GCC_OUT_DIRECTORY "gcc"
#endif

#define DEBUG_DIRECTORY "debug"
#define RELEASE_DIRECTORY "release"

bool CompileLib(vl_cmd *cmd, vl_compile_ctx *ctx, const char *name, bool debug)
{
    ctx->debug = debug;
    ctx->output = name;
    ctx->sourceFiles.count = 0;
    DaAppend(&ctx->sourceFiles, temp_sprintf("../src/%s.c", name));

#if OS_WINDOWS
    ctx->cc = CCompiler_MSVC;
    ctx->type = Compile_StaticLibrary;
    ctx->outputDir = temp_sprintf("%s/msvc", debug ? DEBUG_DIRECTORY : RELEASE_DIRECTORY);
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

    ctx->outputDir = temp_sprintf("%s/" GCC_OUT_DIRECTORY, debug ? DEBUG_DIRECTORY : RELEASE_DIRECTORY);
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

    MkdirIfNotExist(DEBUG_DIRECTORY);
    MkdirIfNotExist(RELEASE_DIRECTORY);
#if defined(_WIN32)
    MkdirIfNotExist(DEBUG_DIRECTORY   "/msvc");
    MkdirIfNotExist(RELEASE_DIRECTORY "/msvc");
#endif
    MkdirIfNotExist(DEBUG_DIRECTORY "/" GCC_OUT_DIRECTORY);
    MkdirIfNotExist(RELEASE_DIRECTORY "/" GCC_OUT_DIRECTORY);

    CompileLib(&cmd, &ctx, "viclib", true);
    CompileLib(&cmd, &ctx, "vl_build", true);
    CompileLib(&cmd, &ctx, "vl_serialize", true);

    CompileLib(&cmd, &ctx, "viclib", false);
    CompileLib(&cmd, &ctx, "vl_build", false);
    CompileLib(&cmd, &ctx, "vl_serialize", false);

    VL_Popd();
    return 0;
}