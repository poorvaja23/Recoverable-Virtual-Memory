#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <string>
#include <iostream>
// using std::string;
using namespace std;

#define TEST_STRING "hello, world"
#define OFFSET2 1000

int main(int argc, char **argv)
{
  rvm_t rvm;
  rvm = rvm_init("rvm_segments");
  cout << rvm.segment_dir << "\n";
  return 0;
}

// g++ -o test rvm.cpp test.cpp -std=c++11 -I -ld
