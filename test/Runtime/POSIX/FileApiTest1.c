/* File-API backend: state read/write over KLEE's fd model.
 * Run: klee --libc=uclibc --posix-runtime test_file_api.bc --sym-files 1 10
 */
#include "klee/klee.h"
#include "klee/file_api.h"
#include <fcntl.h>

int main(void) {
    int fd = __file_open("A");
    if (fd < 0) return 0;          // perms may not allow O_RDWR; exit cleanly

    klee_assert(__file_flags(fd) == O_RDWR);
    __file_set_flags(fd, O_RDONLY);
    klee_assert(__file_flags(fd) == O_RDONLY);

    klee_assert(__file_offset(fd) == 0);
    __file_set_offset(fd, 5);
    klee_assert(__file_offset(fd) == 5);

    __file_set_size(fd, 99);
    klee_assert(__file_size(fd) == 99);
    return 0;
}