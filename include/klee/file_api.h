#ifndef FILE_API_H
#define FILE_API_H

#include "klee.h"
#include <stdio.h> //FILE *
#include <sys/types.h> //ssize_t/mode_t
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef long cnstr_t;
typedef long symbolic;

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
 int __file_mode(int fd);
 int __file_flags(int fd);
 cnstr_t file_exists(const char *fname);
 cnstr_t file_not_exists(const char *fname);

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

/* ══════════════════════════════════════════════════════════════════════
**** ERROR
* ══════════════════════════════════════════════════════════════════════ */

 void __report_error(const char* filename, unsigned int line, const char* message);
 unsigned int __get_errno(void);

/* ══════════════════════════════════════════════════════════════════════
 **** GEN-API
 * ══════════════════════════════════════════════════════════════════════ */

 long __concretize(symbolic var);
 void __gen_assert(cnstr_t cnstr);
 void __assume(cnstr_t cnstr);
 int __is_certain(cnstr_t cnstr);
 int __is_sat(cnstr_t cnstr);
 int __is_symbolic(symbolic var);
 symbolic __sym_var(size_t size);
 symbolic __sym_var_named(char* name, size_t size);
 cnstr_t _EQ_ (long a, long b);
 cnstr_t _NEQ_(long a, long b);
 cnstr_t _LT_ (long a, long b);
 cnstr_t _LE_ (long a, long b);
 cnstr_t _GT_ (long a, long b);
 cnstr_t _GE_ (long a, long b);
 cnstr_t _AND_(cnstr_t a, cnstr_t b);
 cnstr_t _OR_ (cnstr_t a, cnstr_t b);
 cnstr_t _NOT_(cnstr_t a);


 #ifdef __cplusplus
}
#endif

#endif