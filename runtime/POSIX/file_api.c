#define _POSIX_C_SOURCE 200809L
#define _LARGEFILE64_SOURCE

#include "fd.h"
#include "klee/file_api.h"
#include "klee/klee.h"

/*
The API exposes the following KLEE functions:
   - exe_file_t *__get_file(int fd)

In all of these functions the static keyword was removed and the function exposed in fd.h.
*/

#define DEFAULT_MODE 0644 //-rw-r--r--

cnstr_t _EQ_ (long a, long b) { return a == b; }
cnstr_t _NEQ_(long a, long b) { return a != b; }
cnstr_t _LT_ (long a, long b) { return a <  b; }
cnstr_t _LE_ (long a, long b) { return a <= b; }
cnstr_t _GT_ (long a, long b) { return a >  b; }
cnstr_t _GE_ (long a, long b) { return a >= b; }
cnstr_t _AND_(cnstr_t a, cnstr_t b) { return a && b; }
cnstr_t _OR_ (cnstr_t a, cnstr_t b) { return a || b; }
cnstr_t _NOT_(cnstr_t a)            { return !a; }

/* ══════════════════════════════════════════════════════════════════════
**** FILES
* ══════════════════════════════════════════════════════════════════════ */

// Size of a symbolic file created by __file_create(). Changing it breaks tests
// that assert values derived from it: lseek test_07 expects SEEK_END == 10,
// lseek test_08 expects -3 from the end == 7, read test_09 assumes 10 bytes.
#define SYM_FILE_DEFAULT_SIZE 10

// Symbolic files creatable at runtime. The array is allocated once and never
// grown: open descriptors point into it, so realloc would leave them dangling.
#define MAX_RUNTIME_SYM_FILES 8

static exe_disk_file_t *__runtime_sym_files = NULL;

// Creates the file `name`: a file the symbolic file system knows about, with
// symbolic contents and stat, like one from --sym-files.
//
// KLEE names its symbolic files by position ('A', 'B', ...), so the only file
// that can be created is the next letter. On paths where `name` is that
// letter, the file is created; elsewhere it cannot be, and this returns -1.
// A symbolic name forks on it, as KLEE does on any symbolic file name.
// Returns 1, or -1 if the file cannot be created or the runtime limit is hit.
int __file_create(const char *name){
   unsigned idx = __exe_fs.n_sym_files;
   char label[7] = "?_data";
   struct stat64 defaults;
   unsigned i;

   if (!__runtime_sym_files) {
      __runtime_sym_files =
          malloc(sizeof(*__runtime_sym_files) * MAX_RUNTIME_SYM_FILES);
      if (!__runtime_sym_files)
         return -1;
      for (i = 0; i < idx && i < MAX_RUNTIME_SYM_FILES; i++)   // keep --sym-files entries
         __runtime_sym_files[i] = __exe_fs.sym_files[i];
      __exe_fs.sym_files = __runtime_sym_files;
   }

   if (idx >= MAX_RUNTIME_SYM_FILES)
      return -1;

   if (name[0] != (char)('A' + idx) || name[1] != '\0')
      return -1;

   // The label names the symbolic contents, as klee_init_fds names them
   label[0] = (char)('A' + idx);

   stat64(".", &defaults);
   __create_new_dfile(&__exe_fs.sym_files[idx], SYM_FILE_DEFAULT_SIZE,
                      label, &defaults);
   __exe_fs.n_sym_files = idx + 1;
   return 1;
}

int __file_exists(const char *name){
   return __get_sym_file(name) ? 1 : 0;
}

// unlink(name), as KLEE's own unlink does it
int __file_delete(const char *name){
   return unlink(name) == 0 ? 1 : -1;
}

// The open flags glibc's fopen uses for `mode`
static int __fopen_flags(const char *mode){
   int flags;

   switch (mode[0]) {
   case 'r': flags = O_RDONLY; break;
   case 'w': flags = O_WRONLY | O_CREAT | O_TRUNC; break;
   case 'a': flags = O_WRONLY | O_CREAT | O_APPEND; break;
   default:  return -1;
   }

   if (strchr(mode, '+'))
      flags = (flags & ~O_ACCMODE) | O_RDWR;
   if (strchr(mode, 'x'))
      flags |= O_EXCL;
   if (strchr(mode, 'e'))
      flags |= O_CLOEXEC;

   return flags;
}

// fopen(name, mode), returning the descriptor instead of a FILE *
int __file_open(const char *name, const char *mode){
   int flags = __fopen_flags(mode);
   if (flags < 0)
      return -1;
   return open(name, flags, 0666);
}

int __file_close(int fd){
   return close(fd);
}

FILE *__FILE_from_fd(int fd) {
   klee_report_error(__FILE__, __LINE__,
      "__FILE_from_fd: fd->FILE* is not supported by the KLEE backend "
      "(KLEE models descriptors, not stdio streams)", "unsupported.err");
   return 0; //returns null pointer
}

int __fd_from_FILE(FILE* fp){
   return fileno(fp);
}

// The file open on `fd`, if it is one of KLEE's symbolic files
static exe_disk_file_t *__get_dfile(int fd){
   exe_file_t *f = __get_file(fd);
   return f ? f->dfile : NULL;
}

ssize_t __file_size(int fd){
   exe_disk_file_t *df = __get_dfile(fd);
   if (!df)
      return -1;
   return df->size;
}

ssize_t __file_set_size(int fd, size_t size){
   exe_disk_file_t *df = __get_dfile(fd);
   if (!df)
      return -1;
   df->size = size;
   return (ssize_t)df->size;
}

ssize_t __file_offset(int fd){
   exe_file_t *f;
   f = __get_file(fd);
   if(!f)
      return -1;
   return f->off;
}

ssize_t __file_set_offset(int fd, size_t offset){
   exe_file_t *f;
   f = __get_file(fd);
   if(!f)
      return -1;
   f->off = (off64_t)offset;
   return (ssize_t)f->off;
}

// The flags `fd` was opened with, which __fd_open keeps for symbolic files
int __file_flags(int fd){
   exe_file_t *f;
   f = __get_file(fd);
   if(!f)
      return -1;
   if (f->dfile)
      return f->open_flags;
   return fcntl(fd, F_GETFL);
}

// fstat(fd)'s st_mode, stored in *mode
int __file_mode(int fd, mode_t *mode){
   exe_disk_file_t *df = __get_dfile(fd);
   if (!df)
      return -1;
   *mode = df->stat->st_mode;
   return 1;
}

// fchmod(fd, mode): sets the permission bits, keeping the file type
int __file_set_mode(int fd, mode_t mode){
   exe_disk_file_t *df = __get_dfile(fd);
   if (!df)
      return -1;
   df->stat->st_mode = (df->stat->st_mode & ~07777) | (mode & 07777);
   return 1;
}

/* ══════════════════════════════════════════════════════════════════════
**** READ/WRITE
* ══════════════════════════════════════════════════════════════════════ */

ssize_t __file_read(int fd, void* buffer, size_t count){
   return read(fd, buffer, count);
}

ssize_t __file_write(int fd, const void* buffer, size_t count){
   return write(fd, buffer, count);
}

/* ══════════════════════════════════════════════════════════════════════
**** DUP/DUP2
* ══════════════════════════════════════════════════════════════════════ */

int __file_dup(int oldfd){
   return dup(oldfd);
}

int __file_dup2(int oldfd, int newfd){
   return dup2(oldfd, newfd);
}

// setrlimit(RLIMIT_NOFILE, n): KLEE ignores setrlimit, and its descriptor
// table is fixed at MAX_FDS, so the limit can be lowered but not raised
int __file_set_max_fds(int n){
   if (n < 0)
      return -1;
   __exe_max_fds = n < MAX_FDS ? n : MAX_FDS;
   return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 **** ERROR
 * ══════════════════════════════════════════════════════════════════════ */

void __report_error(const char* filename, unsigned int line, const char* message){
   klee_report_error(filename, line, message, ".error");
}

unsigned int __get_errno(void){
   return klee_get_errno();
}


/* ══════════════════════════════════════════════════════════════════════
 **** CORE
 * ══════════════════════════════════════════════════════════════════════ */

 //For now concretize returns only longs
long __concretize(symbolic var){
   return klee_get_valuel(var);
}

// Not __assert, which uClibc defines
void __sra_assert(cnstr_t expr){
   return klee_assert(expr);
}

int __is_symbolic(symbolic var){
   return klee_is_symbolic(var);
}

void __assume(cnstr_t c) { 
   klee_assume(c); 
}

symbolic __sym_var_named(char* name, size_t size){
   symbolic v;
   klee_make_symbolic(&v, sizeof(v), name);

   //smaller than symbolic
   if (size < sizeof(v) * 8) {
      symbolic max = ((symbolic)1 << size);
      __assume(_AND_(_GE_(v, 0), _LT_(v, max)));
   }
   return v;
}

symbolic __sym_var(size_t size){
   static int counter = 0;
   char name[32];
   snprintf(name, sizeof(name), "sym_var_%d", counter++);
   return __sym_var_named(name, size);
}

// The `index`th element of the symbolic array `name`, named name_index
symbolic __sym_var_array(char* name, size_t index, size_t size){
   char label[64];
   snprintf(label, sizeof(label), "%s_%zu", name, index);
   return __sym_var_named(label, size);
}

int __is_certain(cnstr_t cnstr){
   return klee_is_certain(cnstr);
}

int __is_sat(cnstr_t cnstr){
   return klee_is_sat(cnstr);
}
