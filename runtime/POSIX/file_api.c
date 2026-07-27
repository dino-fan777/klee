#define _POSIX_C_SOURCE 200809L
#define _LARGEFILE64_SOURCE

#include "fd.h"
#include "klee/file_api.h"
#include "klee/klee.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#pragma message ("KLEE FILE API extension compiled")

/*
The API exposes the following KLEE functions:
   - exe_file_t *__get_file(int fd)

In all of these functions the static keyword was removed and the function exposed in fd.h.
*/

#define DEFAULT_MODE 0644 //-rw-r--r--

int __file_create(char *name){
   return open(name, O_CREAT | O_RDWR, DEFAULT_MODE);
}

//Does memset(f, 0, sizeof *f); but "f" is a file descriptor struct, file still exists in memory but it's pointer is now 0. 
int __file_close(int fd){
   return close(fd);
}

//Dangerous. If any other descriptor is open on the same dfile (dup, or a second open of "A", setting flags/mode) it will null dereference and crash  
int __file_delete(int fd){
   exe_file_t *f;
   f = __get_file(fd);
   if(!f)
      return -1;
   close(fd); //Without this "f" would be a file descriptor pointing to 0x0 file
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
   r = (f->flags & eReadable) != 0;
   w = (f->flags & eWriteable) != 0;
   if (r && w) 
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
   if (accmode == O_RDONLY || accmode == O_RDWR) f->flags |= eReadable;
   if (accmode == O_WRONLY || accmode == O_RDWR) f->flags |= eWriteable;

   return __file_flags(fd); //Not sure if echo back is POSIX
}

mode_t __file_mode(int fd){
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

ssize_t __file_read(int fd, void* buffer, size_t count){
   return read(fd, buffer, count);
}

ssize_t __file_write(int fd, void* buffer, size_t count){
   return write(fd, buffer, count);
}

 int __file_dup(int oldfd){
   return dup(oldfd);
 }

 int __file_dup2(int oldfd, int newfd){
   return dup2(oldfd, newfd);
 }