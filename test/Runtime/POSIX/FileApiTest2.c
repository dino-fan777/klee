/* gen-API: __is_symbolic distinguishes symbolic values from concrete ones.
 * Run: klee --libc=uclibc --posix-runtime test_is_symbolic.bc
 */
#include "klee/klee.h"
#include "klee/file_api.h"

int main(void) {
    int x;
    klee_make_symbolic(&x, sizeof(x), "x");

    klee_assert(__is_symbolic(x)  == 1);   
    klee_assert(__is_symbolic(42) == 0);   //a literal is not symbolic
    return 0;
}