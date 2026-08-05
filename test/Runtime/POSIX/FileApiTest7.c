/* Isolation test for __sym_var / __sym_var_named.
 * Proves: (a) the returned value is genuinely symbolic,
 *         (b) the "size" parameter really bounds it to "size" bits.
 * Run: klee --libc=uclibc --posix-runtime test_sym_var.bc
 * Expect: KLEE: done, NO error.
 */
#include "klee/klee.h"
#include "klee/file_api.h"

int main(void) {
   
   symbolic x = __sym_var_named("x", 4);

   //(a) it must actually be symbolic, not a concrete constant
   klee_assert(__is_symbolic(x));

   //(b) size is HONORED: x is necessarily within [0, 16)
   __gen_assert(_GE_(x, 0));            // always >= 0  -> PASS
   __gen_assert(_LT_(x, 16));           // always < 16  -> PASS

   //(b') size is not OVER-constraining: x can still take high values.
   //if these were UNSAT, the masking wrongly pinned x too low. 
   klee_assert(klee_is_sat(_EQ_(x, 15)) == 1);   //15 fits in 4 bits -> SAT
   klee_assert(klee_is_sat(_GT_(x, 10)) == 1);   //11..15 possible    -> SAT

   //(b'') the ceiling is real: 16 cannot fit in 4 bits
   klee_assert(klee_is_sat(_EQ_(x, 16)) == 0);   //UNSAT

   //the unnamed variant: unique auto-name, 8-bit here (0..255)
   symbolic y = __sym_var(8);
   klee_assert(__is_symbolic(y));
   __gen_assert(_LT_(y, 256));                    //always < 256 -> PASS
   klee_assert(klee_is_sat(_EQ_(y, 255)) == 1);   //255 fits -> SAT
   klee_assert(klee_is_sat(_EQ_(y, 256)) == 0);   //256 doesn't -> UNSAT

   return 0;
}