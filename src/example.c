#include <stdlib.h>
#include <stdio.h>

#if defined(_MSC_VER)
#include "raddbg_markup.h"
#endif

#define VICLIB_IMPLEMENTATION
#include "../viclib.h"

void TestViewStrs()
{
    view T1 = VIEW_STATIC("1020");
    view T2 = ViewFromParts("1020", 4);
    view T3 = ViewFromCstr("1020");
    Assert(ViewEq(T1, T2) && ViewEq(T2, T3));

    printf("Chop \""VIEW_FMT"\" by delim '0': ", VIEW_ARG(T1));
    view T4 = ViewChopByDelim(&T1, '0');
    printf("Chopped: \""VIEW_FMT"\"\tRemaining: \""VIEW_FMT"\"\n", VIEW_ARG(T4), VIEW_ARG(T1));
    Assert(ViewEq(T4, VIEW("1")) && ViewEq(T1, VIEW("20")));

    printf("Chop \""VIEW_FMT"\" by view \"02\": ", VIEW_ARG(T2));
    view T5 = ViewChopByView(&T2, VIEW("02"));
    printf("Chopped: \""VIEW_FMT"\"\tRemaining: \""VIEW_FMT"\"\n", VIEW_ARG(T5), VIEW_ARG(T2));
    Assert(ViewEq(T5, VIEW("1")) && ViewEq(T2, VIEW("0")));

    printf("Chop \""VIEW_FMT"\" left by 1:\t", VIEW_ARG(T3));
    view T6 = ViewChopLeft(&T3, 1);
    printf("Chopped: \""VIEW_FMT"\"\tRemaining: \""VIEW_FMT"\"\n", VIEW_ARG(T6), VIEW_ARG(T3));
    Assert(ViewEq(T6, VIEW("1")) && ViewEq(T3, VIEW("020")));

    printf("Chop \""VIEW_FMT"\" right by 1:\t", VIEW_ARG(T3));
    T6 = ViewChopRight(&T3, 1);
    printf("Chopped: \""VIEW_FMT"\"\tRemaining: \""VIEW_FMT"\"\n", VIEW_ARG(T6), VIEW_ARG(T3));
    Assert(ViewEq(T6, VIEW("0")) && ViewEq(T3, VIEW("02")));

    view T7 = VIEW_STATIC("\t \nHello\v\f\r");
    T6 = ViewTrim(T7);
    Assert(ViewStartsWith(T6, VIEW("Hell")));
    Assert(ViewFind(T6, VIEW("ell")) != 0);
    Assert(ViewEndsWith(T6, VIEW("ello")));
    printf("To trim: \""VIEW_FMT"\"\tTrimmed: \""VIEW_FMT"\"\n", VIEW_ARG(T7), VIEW_ARG(T6));

    view T8 = ViewSlice(T6, 2, 4);
    printf("To slice: \""VIEW_FMT"\"\tSliced (2, 4): \""VIEW_FMT"\"\n", VIEW_ARG(T6), VIEW_ARG(T8));

    view T9 = 
        VIEW("typedef struct {\r\n"
             "    const char *items;\r\n"
             "    size_t count;\n"
             "} view;\n"
             "#define VIEW(cstr_lit) view_FromParts((cstr_lit), sizeof(cstr_lit) - 1)\r\n"
             "#define VIEW_STATIC(cstr_lit) {(const char*)(cstr_lit), sizeof(cstr_lit) - 1}\n"
             "#define VIEW_FMT \"%.*s\"\n"
             "#define VIEW_ARG(v) (int)(v).count, (v).items\n");
    view T10 = T9;
    printf("Splitting lines:\n"
           "source string: \""VIEW_FMT"\"\n", VIEW_ARG(T9));

    ViewIterateLines(&T9, lineIdx, line) printf("line %d: \'"VIEW_FMT"\'\n", (int)lineIdx, VIEW_ARG(line));

    view T11 = T10;
    printf("Splitting spaces:\n");
    ViewIterateSpaces(&T10, wordIdx, word)
        printf("word %d: \""VIEW_FMT"\"\t", (int)wordIdx, VIEW_ARG(word));

    printf("\nSplitting by separators:\n");
    view delims = VIEW_STATIC(" {}*;#().,");
    ViewIterateDelimiters(&T11, delims, tokIdx, token, delim)
        if((token.count > 0 && !(token.count == 1 && token.items[0] == '\n')) || delim != ' ')
            printf("token %d: \""VIEW_FMT"\"\tdelim found: '%c'\n", (int)tokIdx, VIEW_ARG(token), delim);
}

void TestParsing()
{
    view TestStr = VIEW_STATIC("123");
    s64 Val;
    bool ok = ViewParseS64(TestStr, &Val, 0);
    Assert(ok);
    printf(S64_Fmt"\n", Val);

    TestStr = VIEW("+123Hello");
    view Remaining;
    Assert(ViewParseS64(TestStr, &Val, &Remaining));
    printf("string: \""VIEW_FMT"\" -> num: "S64_Fmt", remaining: \""VIEW_FMT"\"\n",
           VIEW_ARG(TestStr), Val, VIEW_ARG(Remaining));

    TestStr = VIEW("-123");
    Assert(ViewParseS64(TestStr, &Val, 0));
    printf(S64_Fmt"\n", Val);
    TestStr = VIEW("+123");
    Assert(ViewParseS64(TestStr, &Val, 0));
    printf(S64_Fmt"\n", Val);

    TestStr = VIEW("0xFF");
    Assert(ViewParseS64(TestStr, &Val, 0));
    printf(S64_Fmt"\n", Val);

    TestStr = VIEW("0b0110");
    Assert(ViewParseS64(TestStr, &Val, 0));
    printf(S64_Fmt"\n", Val);

    TestStr = VIEW("-0d1234");
    Assert(ViewParseS64(TestStr, &Val, 0));
    printf(S64_Fmt"\n", Val);

    f64 F64Val;
    TestStr = VIEW("2.71828");
    Assert(ViewParseF64(TestStr, &F64Val, 0) == PARSE_OK);
    printf("%lf\n", F64Val);

    TestStr = VIEW("-.135hello");
    Assert(ViewParseF64(TestStr, &F64Val, &TestStr) == PARSE_OK);
    printf("val: %lf, rem: "VIEW_FMT"\n", F64Val, VIEW_ARG(TestStr));

    TestStr = VIEW("1.32e+4hello");
    Assert(ViewParseF64(TestStr, &F64Val, &TestStr) == PARSE_OK);
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

bool IsSorted(int *arr, int len) {
    for(int i = 0; i < len - 1; i++) {
        if(arr[i] > arr[i + 1]) {
            return false;
        }
    }
    return true;
}

#include <time.h>
void TestSort()
{
    int TestSize = 1024;
    int *arr = (int*)malloc(TestSize*sizeof(int));
    srand((unsigned int)time(0));

    for(int i = 0; i < TestSize; i++)
    {
        arr[i] = rand();
    }

    Sort(arr, TestSize, sizeof(int), int_less_than);
    for(int i = 0; i < TestSize; i++) printf("%d\t", arr[i]);
    printf("\n");
    AssertMsg(IsSorted(arr, TestSize), "Sort failed");
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