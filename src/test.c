#include <stdlib.h>
#include <stdio.h>

#include "raddbg_markup.h"

#define VICLIB_IMPLEMENTATION
#include "../viclib.h"

void TestViewStrs()
{
    view T1 = VIEW_STATIC("1020");
    view T2 = view_FromParts("1020", 4);
    view T3 = view_FromCstr("1020");
    Assert(view_Eq(T1, T2) && view_Eq(T2, T3));

    printf("Chop \""VIEW_FMT"\" by delim '0': ", VIEW_ARG(T1));
    view T4 = view_ChopByDelim(&T1, '0');
    printf("Chopped: \""VIEW_FMT"\"\tRemaining: \""VIEW_FMT"\"\n", VIEW_ARG(T4), VIEW_ARG(T1));

    printf("Chop \""VIEW_FMT"\" by view \"02\": ", VIEW_ARG(T2));
    view T5 = view_ChopByView(&T2, VIEW("02"));
    printf("Chopped: \""VIEW_FMT"\"\tRemaining: \""VIEW_FMT"\"\n", VIEW_ARG(T5), VIEW_ARG(T2));

    printf("Chop \""VIEW_FMT"\" left by 1:\t", VIEW_ARG(T3));
    view T6 = view_ChopLeft(&T3, 1);
    printf("Chopped: \""VIEW_FMT"\"\tRemaining: \""VIEW_FMT"\"\n", VIEW_ARG(T6), VIEW_ARG(T3));

    printf("Chop \""VIEW_FMT"\" right by 1:\t", VIEW_ARG(T3));
    T6 = view_ChopRight(&T3, 1);
    printf("Chopped: \""VIEW_FMT"\"\tRemaining: \""VIEW_FMT"\"\n", VIEW_ARG(T6), VIEW_ARG(T3));

    view T7 = VIEW_STATIC("\t \nHello\v\f\r");
    T6 = view_Trim(T7);
    Assert(view_StartsWith(T6, VIEW("Hell")));
    Assert(view_Contains(T6, VIEW("ell")) != 0);
    Assert(view_EndsWith(T6, VIEW("ello")));
    printf("To trim: \""VIEW_FMT"\"\tTrimmed: \""VIEW_FMT"\"\n", VIEW_ARG(T7), VIEW_ARG(T6));

    view T8 = view_Slice(T6, 2, 4);
    printf("To slice: \""VIEW_FMT"\"\tSliced (2, 4): \""VIEW_FMT"\"\n", VIEW_ARG(T6), VIEW_ARG(T8));

    view T9 = 
        VIEW("typedef struct {\n"
             "    const char *Data;\n"
             "    size_t Len;\n"
             "} view;\n"
             "#define VIEW(cstr_lit) view_FromParts((cstr_lit), sizeof(cstr_lit) - 1)\n"
             "#define VIEW_STATIC(cstr_lit) {(const char*)(cstr_lit), sizeof(cstr_lit) - 1}\n"
             "#define VIEW_FMT \"%.*s\"\n"
             "#define VIEW_ARG(v) (int)(v).Len, (v).Data\n");
    printf("Splitting lines:\n"
           "source string: \""VIEW_FMT"\"\n", VIEW_ARG(T9));

    view_IterateLines(T9, lineIdx, line) printf("line %d: "VIEW_FMT"\n", (int)lineIdx, VIEW_ARG(line));
}

void TestParsing()
{
    view TestStr = VIEW_STATIC("123");
    s64 Val;
    bool ok = view_ParseS64(TestStr, &Val, 0);
    Assert(ok);
    printf("%lld\n", Val);

    TestStr = VIEW("+123Hello");
    view Remaining;
    Assert(view_ParseS64(TestStr, &Val, &Remaining));
    printf("string: \""VIEW_FMT"\" -> num: %lld, remaining: \""VIEW_FMT"\"\n",
           VIEW_ARG(TestStr), Val, VIEW_ARG(Remaining));

    TestStr = VIEW("-123");
    Assert(view_ParseS64(TestStr, &Val, 0));
    printf("%lld\n", Val);
    TestStr = VIEW("+123");
    Assert(view_ParseS64(TestStr, &Val, 0));
    printf("%lld\n", Val);

    TestStr = VIEW("0xFF");
    Assert(view_ParseS64(TestStr, &Val, 0));
    printf("%lld\n", Val);

    TestStr = VIEW("0b0110");
    Assert(view_ParseS64(TestStr, &Val, 0));
    printf("%lld\n", Val);

    TestStr = VIEW("-0d1234");
    Assert(view_ParseS64(TestStr, &Val, 0));
    printf("%lld\n", Val);

    f64 F64Val;
    TestStr = VIEW("2.71828");
    Assert(view_ParseF64(TestStr, &F64Val, 0) == PARSE_OK);
    printf("%lf\n", F64Val);

    TestStr = VIEW("-.135hello");
    Assert(view_ParseF64(TestStr, &F64Val, &TestStr) == PARSE_OK);
    printf("val: %lf, rem: "VIEW_FMT"\n", F64Val, VIEW_ARG(TestStr));

    TestStr = VIEW("1.32e+4hello");
    Assert(view_ParseF64(TestStr, &F64Val, &TestStr) == PARSE_OK);
    printf("val: %lf, rem: "VIEW_FMT"\n", F64Val, VIEW_ARG(TestStr));
}

void TestLoc()
{
    printf("current code location with different styles:\n");
    printf(LOC_MSVC_STR"\n");
    printf(LOC_UNIX_STR"\n");
    printf(LOC_STR"\n\n");

    code_location Loc = CURR_LOC;
    printf(LOC_FMT"\n", LOC_ARG(Loc));
    printf(LOC_FMT"\n", LOC_ARG(CURR_LOC));
}

bool IsSorted(int *Arr, int Len) {
    for(int i = 0; i < Len - 1; i++) {
        if(Arr[i] > Arr[i + 1]) {
            return false;
        }
    }
    return true;
}

#include <time.h>
void TestSort()
{
    int TestSize = 1024;
    int *Arr = (int*)malloc(TestSize*sizeof(int));
    srand((unsigned int)time(0));

    for(int i = 0; i < TestSize; i++)
    {
        Arr[i] = rand();
    }

    Sort(Arr, TestSize, sizeof(int), int_less_than);
    for(int i = 0; i < TestSize; i++) printf("%d\t", Arr[i]);
    printf("\n");
    AssertMsg(IsSorted(Arr, TestSize), "Sort failed");
}

void TestAll()
{
    TestViewStrs();
    TestParsing();
    TestLoc();
    TestSort();
}

int main()
{
    TestAll();

    return 0;
}