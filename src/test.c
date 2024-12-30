#include <stdio.h>

#define VICLIB_IMPLEMENTATION
#include "viclib.h"

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
    printf("To trim: \""VIEW_FMT"\"\tTrimmed: \""VIEW_FMT"\"\n", VIEW_ARG(T7), VIEW_ARG(T6));
}

void TestParsing()
{
    view TestStr = VIEW_STATIC("123");
    s64 Val;
    bool ok = view_ParseS64(TestStr, &Val, 0);
    Assert(ok);
    printf("%lld\n", Val);
    
    TestStr = VIEW("123Hello");
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

void TestAll()
{
    TestViewStrs();
    TestParsing();
    TestLoc();
}

int main()
{
    TestAll();
    
    return 0;
}