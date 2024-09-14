#include "codegen.h"

void gen_lval(node_t *node) {
  if (node->kind != ND_LVAR)
    error("lhs of assignment is not variable, but got %d", node->kind);
  printf("\tmov rax, rbp\n");
  printf("\tsub rax, %d\n", node->offset);
  printf("\tpush rax\n");
}

int beginLabelCnt = 0;
int endLabelCnt = 0;
int elseLabelCnt = 0;

int localNum = 0;

void gen(node_t *node) {
  if(node->kind == ND_FUNC) {
    printf("%s:\n", node->name);
    // prologue
    // allocate area of 26 variables
    printf("\tpush rbp\n");
    printf("\tmov rbp, rsp\n");
    printf("\tsub rsp, %d\n", localNum * 8);

    int argNum = 0;
    for(node_t *arg = node->arg; arg; arg = arg->arg, argNum++) {
      gen_lval(arg);
      printf("\tpop rax\n\n");
      switch(argNum) {
        case 0:
          //printf("\tpush rdi\n");
          printf("\tmov [rax], rdi\n");
          break;
        case 1:
          // printf("\tpush rsi\n");
          printf("\tmov [rax], rsi\n");
          break;
        case 2:
          // printf("\tpush rdx\n");
          printf("\tmov [rax], rdx\n");
          break;
        case 3:
          // printf("\tpush rcx\n");
          printf("\tmov [rax], rcx\n");
          break;
        case 4:
          // printf("\tpush r8\n");
          printf("\tmov [rax], r8\n");
          break;
        case 5:
          // printf("\tpush r9\n");
          printf("\tmov [rax], r9\n");
          break;
      }
    }

    gen(node->stmt);
    // pop stack top that is expresstion value to avoid stack overflow
    printf("\tpop rax\n");
    // rax is last expresstion value and it is return value
    printf("\tmov rsp, rbp\n");
    printf("\tpop rbp\n");
    printf("\tret\n");
    return;
  }

  switch(node->kind) {
    case ND_BLOCK:
      while(node->stmt) {
        gen(node->stmt);
        node = node->stmt;
        printf("\tpop rax\n");
      }
      printf("\tpush rax\n");
      return;

    case ND_RETURN:
      gen(node->lhs);
      printf("\tpop rax\n");
      printf("\tmov rsp, rbp\n");
      printf("\tpop rbp\n");
      printf("\tret\n");
      return;

    case ND_IF:
      gen(node->cond);
      printf("\tpop rax\n");
      printf("\tcmp rax, 0\n");

      if(node->els) {
        printf("\tje .Lelse%d\n", elseLabelCnt);
        gen(node->then);
        printf("\tjmp .Lend%d\n", endLabelCnt);
        printf(".Lelse%d:\n", elseLabelCnt++);
        gen(node->els);
      } else {
        printf("\tje .Lend%d\n", endLabelCnt);
        gen(node->then);
      }
      printf(".Lend%d:\n", endLabelCnt++);
      printf("\tpush rax\n");
      return;

    case ND_WHILE:
      printf(".Lbegin%d:\n", beginLabelCnt);
      gen(node->cond);
      printf("\tpop rax\n");
      printf("\tcmp rax, 0\n");
      printf("\tje .Lend%d\n", endLabelCnt);
      gen(node->then);
      printf("\tjmp .Lbegin%d\n", beginLabelCnt++);
      printf(".Lend%d:\n", endLabelCnt++);
      printf("\tpush rax\n");
      return;

    case ND_FOR:
      gen(node->init);
      printf(".Lbegin%d:\n", beginLabelCnt);
      gen(node->cond);
      printf("\tpop rax\n");
      printf("\tcmp rax, 0\n");
      printf("\tje .Lend%d\n", endLabelCnt);
      gen(node->then);
      gen(node->adv);
      printf("\tjmp .Lbegin%d\n", beginLabelCnt++);
      printf(".Lend%d:\n", endLabelCnt++);
      printf("\tpush rax\n");
      return;
  }

  switch(node->kind) {
    case ND_CALL:
      int argNum = 0;
      for(node_t *arg = node->arg; arg; arg = arg->arg, argNum++) {
        gen(arg);
        switch(argNum) {
          case 0:
            printf("\tpop rdi\n");
            break;
          case 1:
            printf("\tpop rsi\n");
            break;
          case 2:
            printf("\tpop rdx\n");
            break;
          case 3:
            printf("\tpop rcx\n");
            break;
          case 4:
            printf("\tpop r8\n");
            break;
          case 5:
            printf("\tpop r9\n");
            break;
        }
      }
      // adjust rsp to multiple of 16
      int rspSup = (argNum%2) * 8;
      printf("\tsub rsp, %d\n", rspSup);

      printf("\tcall %s\n", node->name);
      printf("\tadd rsp, %d\n", rspSup);
      printf("\tpush rax\n");
      return;

    case ND_NUM:
      printf("\tpush %d\n", node->val);
      return;

    case ND_LVAR:
      gen_lval(node);
      printf("\tpop rax\n");
      printf("\tmov rax, [rax]\n");
      printf("\tpush rax\n");
      return;

    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("\tpop rdi\n");
      printf("\tpop rax\n");
      printf("\tmov [rax], rdi\n");
      printf("\tpush rdi\n");
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("\tpop rdi\n");
  printf("\tpop rax\n");

  switch(node->kind) {
    case ND_ADD:
      printf("\tadd rax, rdi\n");
      break;
    case ND_SUB:
      printf("\tsub rax, rdi\n");
      break;
    case ND_MUL:
      printf("\timul rax, rdi\n");
      break;
    case ND_DIV:
      printf("\tcqo\n");
      printf("\tidiv rdi\n");
      break;
    case ND_EQ:
      printf("\tcmp rax, rdi\n");
      printf("\tsete al\n");
      printf("\tmovzb rax, al\n");
      break;
    case ND_NE:
      printf("\tcmp rax, rdi\n");
      printf("\tsetne al\n");
      printf("\tmovzb rax, al\n");
      break;
    case ND_LT:
      printf("\tcmp rax, rdi\n");
      printf("\tsetl al\n");
      printf("\tmovzb rax, al\n");
      break;
    case ND_LE:
      printf("\tcmp rax, rdi\n");
      printf("\tsetle al\n");
      printf("\tmovzb rax, al\n");
      break;
      
  }

  printf("\tpush rax\n");
}
