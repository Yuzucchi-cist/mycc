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
  node_t *node = expr();

  // output a first half of assembry
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  
  gen(node);

  // load stack top that is expresstion value to rax register as return value
  printf("\tpop rax\n");
  printf("\tret\n");
  return 0;
}
