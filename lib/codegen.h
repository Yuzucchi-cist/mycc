#ifndef __CODEGEN_H__
#define __CODEGEN_H__

#include <parse.h>

extern int beginLabelCnt;
extern int endLabelCnt;
extern int elseLabelCnt;

void gen_lval(node_t *node);
void gen(node_t *node);

#endif
