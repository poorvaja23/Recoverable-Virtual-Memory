#include "rvm.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
using std::string;

rvm_t rvm_init(const char *directory){
  struct stat st = {0};
  if (stat(directory, &st) == -1) {
    mkdir(directory, 0700);
  }
  rvm_t store;
  store.segment_dir = string(directory);
  return store;
}
