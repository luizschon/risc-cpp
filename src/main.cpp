#include <iostream>
#include <string>
#include <stdio.h>

#include "../include/runtime.h"
#include "../include/riscv.h"

extern bool verbose;

int main(int argc, char** argv) {
 
  if (argc < 3) {
    std::cout << "Wrong number of arguments" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << argv[0] << " /path/to/code/binary/file /path/to/data/binary/file" << std::endl;
    return 1;
  }

  const char* text_file = argv[1];
  const char* data_file = argv[2];
  
  if (argv[3])
    verbose = true;

  init();
  load_mem(text_file, data_file);
  run();

  return 0;
}
