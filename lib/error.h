#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

extern char *user_input;

// void initialize_error(char *initialize_user_input);

// print error
// args are same as printf
void error(char *fmt, ...);

// print error with place
void error_at(char *loc, char *fmt, ...);

#endif
