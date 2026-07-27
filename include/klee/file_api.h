#ifndef FILE_API_H
#define FILE_API_H

#include "klee.h"
#include <stdio.h> //FILE *
#include <sys/types.h> //ssize_t/mode_t

#ifdef __cplusplus
extern "C" {
#endif

/* ══════════════════════════════════════════════════════════════════════
 **** OPEN/FOPEN
 * ══════════════════════════════════════════════════════════════════════ */
 int __file_create(char* name);
 int __file_close(int fd);
 int __file_delete(int fd);
 int __file_open(char* name);
 FILE *__file_fopen(char *name);
 FILE* __FILE_from_fd(int fd);
 int __fd_from_FILE(FILE* fp);
 ssize_t __file_size(int fd);
 ssize_t __file_offset(int fd);
 ssize_t __file_set_size(int fd, size_t size);
 ssize_t __file_set_offset(int fd, size_t offset);
 int __file_set_flags(int fd, int flags);
 int __file_set_mode(int fd, mode_t mode);
 mode_t __file_mode(int fd);
 int __file_flags(int fd);

 /* ══════════════════════════════════════════════════════════════════════
 **** READ/WRITE
 * ══════════════════════════════════════════════════════════════════════ */

 ssize_t __file_read(int fd, void* buffer, size_t count);
 ssize_t __file_write(int fd, void* buffer, size_t count);

  /* ══════════════════════════════════════════════════════════════════════
 **** DUP/DUP2
 * ══════════════════════════════════════════════════════════════════════ */

 int __file_dup(int oldfd);
 int __file_dup2(int oldfd, int newfd);

 #ifdef __cplusplus
}
#endif

#endif