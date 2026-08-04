/* gen-API: __gen_assert PASSES when the constraint is necessarily true
 * (NOT(expr) is UNSAT). This test must complete with NO error.
 * Run: klee --libc=uclibc --posix-runtime test_assert_pass.bc
 */
#include "klee/klee.h"
#include "klee/file_api.h"

int main(void) {
    int x;
    klee_make_symbolic(&x, sizeof(x), "x");
    klee_assume(x >= 0);
    klee_assume(x <= 5);                      //0 <= x <= 5

    __gen_assert(_LE_(x, 5));                     //always true -> PASS
    __gen_assert(_GE_(x, 0));                     //always true -> PASS

    __assume(_EQ_(x, 3));                     //pin x to 3        
    __gen_assert(_EQ_(x, 3));                 //now necessarily true -> PASS
    return 0;
}