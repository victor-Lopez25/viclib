#define VL_REBUILD_URSELF(bin, src) VL_DEFAULT_REBUILD_URSELF(bin, src), VL_CC_DEBUG_INFO
#define VL_BUILD_IMPLEMENTATION
#include "../vl_build.h"

void CompileStuff(void)
{
    vl_cmd cmd = {0};

    VL_cc(&cmd);
    cmd_Append(&cmd, "../src/example.c");
    VL_ccOutput(&cmd, "example" VL_EXE_EXTENSION);
    VL_ccDebug(&cmd);
    VL_ccWarnings(&cmd);
    VL_ccWarningsAsErrors(&cmd);
    if(!CmdRun(&cmd)) return;

    VL_cc(&cmd);
    cmd_Append(&cmd, "../src/mem_example.c");
    VL_ccOutput(&cmd, "mem_example" VL_EXE_EXTENSION);
    VL_ccWarnings(&cmd);
    VL_ccWarningsAsErrors(&cmd);
    if(!CmdRun(&cmd)) return;

    VL_cc(&cmd);
    cmd_Append(&cmd, "../src/file_io_example.c");
    VL_ccOutput(&cmd, "file_io_example" VL_EXE_EXTENSION);
    VL_ccWarnings(&cmd);
    VL_ccWarningsAsErrors(&cmd);
    if(!CmdRun(&cmd)) return;
}

void TestNeedsRebuild(void)
{
    if(VL_Needs_C_Rebuild("vl_build_example" VL_EXE_EXTENSION, "src/vl_build_example.c")) {
        printf("Needs rebuild\n");
    } else {
        printf("Doesn't need rebuild\n");
    }
}

int main(int argc, char **argv)
{
    VL_GO_REBUILD_URSELF(argc, argv, "vl_build.h");
    bool ok = VL_Init();
    AssertAlways(ok);

    MkdirIfNotExist("bin");
    VL_Pushd("bin");
    LogTimeBetween(CompileStuff(), "Build ");
    VL_Popd();

    //TestNeedsRebuild();

    if(VL_Needs_C_Rebuild("vl_build_example" VL_EXE_EXTENSION, "src/vl_build_example.c")) {
        printf("Needs rebuild\n");
    } else {
        printf("Doesn't need rebuild\n");
    }

    return 0;
}
