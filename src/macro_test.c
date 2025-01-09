// note to self: for standard macros, in msvc, use /Zc:preprocessor
#define SELECT_PROC_ONE_DEFAULT_(_1, _2, NAME, ...) NAME
#define SELECT_PROC_ONE_DEFAULT(A,B,...) SELECT_PROC_ONE_DEFAULT_(dummy, ##__VA_ARGS__, A, B)
#define proc(A, ...) SELECT_PROC_ONE_DEFAULT(proc_specific, proc_default, ##__VA_ARGS__)(A, ##__VA_ARGS__)

int proc_specific(int A, int val)
{
    // some implementation
    (void)A; val = 4; val += 2;
    return val + A;
}
#define proc_default(A) proc_specific(A, 5)

int main()
{
    int value = 1;
    SELECT_PROC_ONE_DEFAULT(proc_specific, proc_default)(value);
    // SELECT_PROC_ONE_DEFAULT(proc_default, proc_specific)(value); // line 20
    // SELECT_PROC_ONE_DEFAULT(proc_default, proc_specific, 6)(value, 6); // line 21
    SELECT_PROC_ONE_DEFAULT(proc_specific, proc_default, 6)(value, 6);   
    
    int a = proc(1); // proc_specific(1 )
    proc(a, 7); // proc_specific(a,7)
    
    return 0;
}