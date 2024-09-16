#include <stdio.h>
#include <stdlib.h>

#include <error.h>
#include <parse.h>
#include <codegen.h>

int main(int argc, char **argv) {
  if(argc != 2) {
    error("ERROR!: Number of args == 2");
    return 1;
  }

  // tokenize and parse
  user_input = argv[1];
  token = tokenize(user_input);
  node_t *node = program();

  codegen();
  return 0;
}
