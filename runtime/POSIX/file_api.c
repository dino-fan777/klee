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
**** OPEN/FOPEN
* ══════════════════════════════════════════════════════════════════════ */

// Size of a symbolic file created by __file_create(). Changing it breaks tests
// that assert values derived from it: lseek test_07 expects SEEK_END == 10,
// lseek test_08 expects -3 from the end == 7, read test_09 assumes 10 bytes.
#define SYM_FILE_DEFAULT_SIZE 10

// Symbolic files creatable at runtime. The array is allocated once and never
// grown: open descriptors point into it, so realloc would leave them dangling.
#define MAX_RUNTIME_SYM_FILES 8

static exe_disk_file_t *__runtime_sym_files = NULL;

// Creates a file the symbolic file system knows about: symbolic contents and
// stat, recognised by open() and file_exists(), like one from --sym-files.
// KLEE identifies symbolic files by position ('A', 'B', ...), so the new file
// takes the next letter; name only labels the contents, it is not the path.
// Returns an open read/write descriptor, or -1 if the runtime limit is hit.
int __file_create(char *name){
   unsigned idx = __exe_fs.n_sym_files;
   char symname[64];
   struct stat64 defaults;
   exe_file_t *f;
   int nfd;
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

   i = 0;
   if (name && name[0]) {
      while (i < 56 && name[i]) {
         symname[i] = name[i];
         i++;
      }
   } else {
      symname[0] = (char)('A' + idx);
      i = 1;
   }
   symname[i] = '\0';

   stat64(".", &defaults);
   __create_new_dfile(&__exe_fs.sym_files[idx], SYM_FILE_DEFAULT_SIZE,
                      symname, &defaults);
   __exe_fs.n_sym_files = idx + 1;

   // Bind the descriptor directly instead of calling open(): open() would fork on
   // has_permission() against the new symbolic st_mode, and on success overwrite
   // st_mode with (mode & ~umask), clearing the bits the caller is about to test.
   for (nfd = 0; nfd < MAX_FDS; ++nfd)
      if (!(__exe_env.fds[nfd].flags & eOpen))
         break;
   if (nfd == MAX_FDS) {
      errno = EMFILE;
      return -1;
   }

   f = &__exe_env.fds[nfd];
   memset(f, 0, sizeof *f);
   f->dfile = &__exe_fs.sym_files[idx];
   f->flags = eOpen | eReadable | eWriteable;

   return nfd;
}

int __file_close(int fd){
   return close(fd);
}

//Dangerous. If any other descriptor is open on the same dfile (dup, or a second open of "A", setting flags/mode) it will null dereference  
int __file_delete(int fd){
   exe_file_t *f;
   f = __get_file(fd);
   if(!f)
      return -1;
   close(fd);
   memset(f->dfile, 0, sizeof *f->dfile);
   return 1;
}

int __file_open(char *name){
   return open(name, O_RDWR);
}

FILE *__file_fopen(char *name){
   return fopen(name, "r+");
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

ssize_t __file_size(int fd){
   exe_file_t *f;
   f = __get_file(fd);
   if(!f)
      return -1;
   return f->dfile->size;
}

ssize_t __file_set_size(int fd, size_t size){
   exe_file_t *f;
   f = __get_file(fd);
   if(!f)
      return -1;
   f->dfile->size = size;
   return (ssize_t)f->dfile->size;
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

int __file_flags(int fd){
   exe_file_t *f;
   int r, w;
   f = __get_file(fd);
   if(!f) 
      return -1;
   r = _NEQ_(f->flags & eReadable, 0); 
   w = _NEQ_(f->flags & eWriteable, 0);
   if (_AND_(r, w))  
      return O_RDWR;
   if (w)      
      return O_WRONLY;
   return O_RDONLY;
}

int __file_set_flags(int fd, int flags){
   exe_file_t *f;
   f = __get_file(fd);
   int accmode;
   if(!f)
      return -1;

   accmode = flags & O_ACCMODE;
   f->flags &= ~(eReadable | eWriteable);
   if (_OR_(_EQ_(accmode, O_RDONLY), _EQ_(accmode, O_RDWR))) f->flags |= eReadable;
   if (_OR_(_EQ_(accmode, O_WRONLY), _EQ_(accmode, O_RDWR))) f->flags |= eWriteable;

   return __file_flags(fd); //Not sure if echo back is POSIX
}

int __file_mode(int fd){
   exe_file_t *f;
   f = __get_file(fd);
   if(!f)
      return -1;
   return f->dfile->stat->st_mode;
}

int __file_set_mode(int fd, mode_t mode){
   exe_file_t *f;
   f = __get_file(fd);
   if(!f)
      return -1;
   f->dfile->stat->st_mode = mode;
   return f->dfile->stat->st_mode;
}

/* ══════════════════════════════════════════════════════════════════════
**** READ/WRITE
* ══════════════════════════════════════════════════════════════════════ */

ssize_t __file_read(int fd, void* buffer, size_t count){
   return read(fd, buffer, count);
}

ssize_t __file_write(int fd, void* buffer, size_t count){
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
 **** GEN-API
 * ══════════════════════════════════════════════════════════════════════ */

 //For now concretize returns only longs
long __concretize(symbolic var){
   return klee_get_valuel(var);
}

void __gen_assert(cnstr_t expr){
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

int __is_certain(cnstr_t cnstr){
   return klee_is_certain(cnstr);
}

int __is_sat(cnstr_t cnstr){
   return klee_is_sat(cnstr);
}

/*
cnstr_t file_exists(const char *fname){
    return _AND_(_GE_(fname[0], 'A'), _LE_(fname[0], 'Z'));
}
*/

cnstr_t file_exists(const char *fname) {
    cnstr_t m = _EQ_(0, 1);
    unsigned i;
    for (i = 0; i < __exe_fs.n_sym_files; i++) {
        cnstr_t name_match  = _EQ_(fname[0], (long)('A' + i));
        cnstr_t not_deleted = _NEQ_(__exe_fs.sym_files[i].stat->st_ino, 0);
        m = _OR_(m, _AND_(name_match, not_deleted));
    }
    return m;
}

/*
cnstr_t file_exists(const char *fname) {
    return _AND_(_GE_(fname[0], 'A'),
                 _LT_(fname[0], (long)('A' + __exe_fs.n_sym_files)));
}
*/

/*
cnstr_t file_exists(const char *fname) {
   struct stat sb;
   cnstr_t in_range = _AND_(_GE_(fname[0], 'A'), _LE_(fname[0], 'Z'));
   cnstr_t stat_ok  = _EQ_(stat(fname, &sb), 0);
   return _AND_(in_range, stat_ok);
}
*/

cnstr_t file_not_exists(const char *fname) {
    return _NOT_(file_exists(fname));
}

/*
cnstr_t file_not_exists(const char *fname) {
    cnstr_t in_domain = _AND_(_GE_(fname[0], 'A'), _LE_(fname[0], 'Z'));
    return _AND_(in_domain, _NOT_(file_exists(fname)));
}
*/