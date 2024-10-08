#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

extern char *user_input;
extern char *filename;

void error(char *fmt, ...);

void error_at(bool will_finish, char *loc, char *fmt, ...);

#endif
