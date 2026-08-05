/* Isolation test for klee_is_certain (mustBeTrue).
 * Proves: (a) is_certain returns 1 only when the constraint holds on
 *             EVERY value allowed by the path condition,
 *         (b) is_sat and is_certain DISAGREE on a partially-true constraint
 *             (this is the proof that mustBeTrue was wired, not mayBeTrue).
 * Run: klee --libc=uclibc --posix-runtime test_is_certain.bc
 * Expect: KLEE: done, NO error.
 */
#include "klee/klee.h"
#include "klee/file_api.h"

int main(void) {

   int x;
   klee_make_symbolic(&x, sizeof(x), "x");
   klee_assume(x >= 0);
   klee_assume(x <= 5);                            //path condition: 0 <= x <= 5

   //(a) certainly-true facts under the PC
   klee_assert(klee_is_certain(_LE_(x, 5)) == 1);  //x <= 5 on every value  -> certain
   klee_assert(klee_is_certain(_GE_(x, 0)) == 1);  //x >= 0 on every value  -> certain

   //(b) THE DISCRIMINATING TEST: same constraint, opposite answers.
   //if is_certain wrongly returns 1 here, mayBeTrue was copied by mistake.
   klee_assert(klee_is_sat(_EQ_(x, 3))     == 1);  //x==3 is POSSIBLE        -> SAT
   klee_assert(klee_is_certain(_EQ_(x, 3)) == 0);  //x==3 NOT on every value -> not certain

   //not certain: x could exceed 3 (x=4,5)
   klee_assert(klee_is_certain(_LE_(x, 3)) == 0);  //x<=3 fails for x=4,5    -> not certain

   //certainly false things are also NOT certain-true
   klee_assert(klee_is_certain(_GT_(x, 5)) == 0);  //x>5 never holds         -> not certain

   //(c) pin x, then a previously-uncertain fact becomes certain
   klee_assume(x == 3);                            //PC now forces x == 3
   klee_assert(klee_is_certain(_EQ_(x, 3)) == 1);  //now certainly 3         -> certain
   klee_assert(klee_is_certain(_LE_(x, 3)) == 1);  //3 <= 3 always           -> certain

   return 0;
}