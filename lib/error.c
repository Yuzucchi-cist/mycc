#include "error.h"

char *filename;
char *user_input;

/*
void initialize_error(char *initialize_user_input) {
  user_input = initialize_user_input;
}
*/

// print error
// args are same as printf
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// print error with format:
// foo.c:10: x = y + * 5;
//                   ^ not expresstion
void error_at(bool will_finish, char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  
  // calc start and end point of line that includes loc
  char *line = loc;
  while(user_input < line && line[-1] != '\n') line --;

  char *end = loc;
  while(*end != '\n') end++;
  
  // calc line number that includes loc
  int line_num = 1;
  for(char *p=user_input; p < line; p++)
    if(*p=='\n')
      line_num++;

  // show found line with file name and line number
  int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
  fprintf(stderr, "%.*s\n", (int)(end-line), line);
  
  // show error massage pointed to error by "^" 
  int pos = loc - line + indent;
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  if(will_finish)  exit(1);
}

