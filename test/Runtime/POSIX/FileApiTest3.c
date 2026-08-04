/* gen-API: klee_is_sat = satisfiability under the current path condition.
 * Run: klee --libc=uclibc --posix-runtime test_is_sat.bc
 */
#include "klee/klee.h"
#include "klee/file_api.h"

int main(void) {
    int x;
    klee_make_symbolic(&x, sizeof(x), "x");
    klee_assume(x >= 0);
    klee_assume(x <= 5);                          

   
    klee_assert(klee_is_sat(_EQ_(x, 3))        == 1);  
    klee_assert(klee_is_sat(_EQ_(x, 99))       == 0);  //out of range
    klee_assert(klee_is_sat(_NOT_(_EQ_(x, 3))) == 1);  //x=0 work
    klee_assert(klee_is_sat(_GT_(x, 5))        == 0);  //impossible
    klee_assert(klee_is_sat(_LE_(x, 5))        == 1);  //always holds

    //concrete cases: tautology SAT, contradiction UNSAT
    klee_assert(klee_is_sat(_EQ_(1, 1)) == 1);
    klee_assert(klee_is_sat(_EQ_(1, 0)) == 0);
    return 0;
}

/*
RESULTS

constraint	raw-expr-shows	   result	expected	✓
x == 3	     (Eq 3 x)	          1	      SAT	    ✓
x == 99	    (Eq 99 x)	          0 	 UNSAT	    ✓
!(x == 3) (Eq false (Eq 3 x))	  1       SAT	    ✓
x > 5	    (Slt 5 x)	          0	     UNSAT	    ✓
x <= 5	    (Sle x 5)	          1	      SAT	    ✓
1 == 1	    1 → true	          1	      SAT	    ✓
1 == 0	   0 → false	          0 	 UNSAT	    ✓
*/