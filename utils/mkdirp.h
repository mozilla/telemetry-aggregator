#ifndef MKDIRP_H
#define MKDIRP_H
 
#include <sys/stat.h>
#include <errno.h>
 
#define DEFAULT_MODE S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
 
/** Utility function to create directory tree */
bool mkdirp(const char* path, mode_t mode = DEFAULT_MODE) {
  // Invalid string
  if (path[0] == '\0') {
    return false;
  }

  // const cast for hack
  char* p = const_cast<char*>(path);

  // Find next slash mkdir() it and until we're at end of string
  while (*p != '\0') {
    // Skip first character
    p++;

    // Find first slash or end
    while(*p != '\0' && *p != '/') p++;

    // Remember value from p
    char v = *p;

    // Write end of string at p
    *p = '\0';

    // Create folder from path to '\0' inserted at p
    if(mkdir(path, mode) != 0 && errno != EEXIST) {
      *p = v;
      return false;
    }

    // Restore path to it's former glory
    *p = v;
  }
 
  return true;
}
 
#endif // MKDIRP_H