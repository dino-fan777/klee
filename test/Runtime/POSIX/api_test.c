#include "klee/klee.h"
#include "klee/file_api.h"
#include <fcntl.h>

int main(void) {
    int fd = __file_open("A");
    if (fd < 0) return 0;          /* on paths where perms don't allow O_RDWR, just exit */

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
