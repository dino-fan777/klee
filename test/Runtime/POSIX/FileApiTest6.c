/* gen-API: __gen_assert REPORTS when the constraint is NOT necessarily true
 * (NOT(expr) is SAT). This test is EXPECTED to produce a KLEE error --
 * that is the pass condition. If it completes cleanly, __gen_assert is broken.
 * Run: klee --libc=uclibc --posix-runtime test_assert_fail.bc
 *   EXPECT: "assertion is not necessarily true"
 */
#include "klee/klee.h"
#include "klee/file_api.h"

int main(void) {
   int x;
   klee_make_symbolic(&x, sizeof(x), "x");
   klee_assume(x >= 0);
   klee_assume(x <= 5);                      //0 <= x <= 5

   //NOT(x==3) is SAT (x could be 0) -> must report?
   __gen_assert(_EQ_(x, 3));
   return 0;
}