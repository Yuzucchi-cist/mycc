#ifndef __CODEGEN_H__
#define __CODEGEN_H__

#include <parse.h>

extern int beginLabelCnt;
extern int endLabelCnt;
extern int elseLabelCnt;
extern int localNum;

void gen_lval(node_t *node);
void gen(node_t *node);
void codegen();

#endif
