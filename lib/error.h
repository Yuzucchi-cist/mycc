#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

extern char *user_input;

void error(char *fmt, ...);

void error_at(char *loc, char *fmt, ...);

#endif
