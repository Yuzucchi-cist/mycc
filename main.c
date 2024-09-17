#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <error.h>
#include <parse.h>
#include <codegen.h>

char *read_file(char *path) {
  FILE *fp = fopen(path, "r");
  // open file
  if(!fp)
    error("ERROR!: cannot open %s:%s", path, strerror(errno));

  // check file length
  if(fseek(fp, 0, SEEK_END) == -1)
    error("%s: fseek: %s", path, strerror(errno));
  size_t size = ftell(fp);
  if(fseek(fp, 0, SEEK_SET) == -1)
    error("%s: fseek: %s", path, strerror(errno));

  // read file
  char *buf = calloc(1, size + 2);
  fread(buf, size, 1, fp);

  // file must terminate "\n\0"
  if(size == 0 || buf[size-1] != '\n')
    buf[size++] = '\n';
  buf[size] = '\0';

  // close file
  fclose(fp);
  return buf;
}

int main(int argc, char **argv) {
  if(argc != 2) {
    error("ERROR!: Number of args == 2");
    return 1;
  }

  char *filename = argv[1];

  // tokenize and parse
  user_input = read_file(filename);
  token = tokenize(user_input);
  node_t *node = program();

  codegen();
  return 0;
}
