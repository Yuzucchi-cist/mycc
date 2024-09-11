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

  // output a first half of assembry
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // prologue
  // allocate area of 26 variables
  printf("\tpush rbp\n");
  printf("\tmov rbp, rsp\n");
  printf("\tsub rsp, 208\n");

  for (int i=0; code[i]; i++) {
    gen(code[i]);

    // pop stack top that is expresstion value to avoid stack overflow
    printf("\tpop rax\n");
  }

  // rax is last expresstion value and it is return value
  printf("\tmov rsp, rbp\n");
  printf("\tpop rbp\n");
  printf("\tret\n");
  return 0;
}
