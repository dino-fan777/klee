/* gen-API: __concretize returns one value the symbolic var may take,
 * consistent with the current path condition.
 * Run: klee --libc=uclibc --posix-runtime test_concretize.bc
 */
#include "klee/klee.h"
#include "klee/file_api.h"

int main(void) {
    int x;
    klee_make_symbolic(&x, sizeof(x), "x");
    klee_assume(x >= 10);
    klee_assume(x <= 20);                     //10 <= x <= 20

    long cx = __concretize(x);
    klee_assert(cx >= 10 && cx <= 20); //has to respect the condition or fail
    return 0;
}