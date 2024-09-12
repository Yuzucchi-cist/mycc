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

  func_t *f = funcs;
  printf(".globl ");
  for(;;) {
    printf("%s", f->name);
    f = f->next;
    if(f)  printf(", ");
    else break;
  }
  printf("\n");

  for(localNum=0; locals; locals=locals->next)  localNum++;

  
  for (int i=0; code[i]; i++) {
    gen(code[i]);

  }

  return 0;
}
