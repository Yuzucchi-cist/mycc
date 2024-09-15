#include "codegen.h"

char *argregw[6] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
char *argregd[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_addr(node_t *node) {
  printf("\tmov rax, rbp\n");
  printf("\tsub rax, %d\n", node->offset);
  printf("\tpush rax\n");
}

void gen_lval(node_t *node) {
  if (node->kind != ND_LVAR)
    error("lhs of assignment is not variable, but got %d", node->kind);
  gen_addr(node);
}

int beginLabelCnt = 0;
int endLabelCnt = 0;
int elseLabelCnt = 0;
int lvarOffset = 0;

void load(type_t *ty) {
  printf("\tpop rax\n");

  if(ty->ty == INT)
    printf("\tmovsx rax, dword ptr [rax]\n");
  else
    printf("\tmov rax, [rax]\n");

  printf("\tpush rax\n");
}

void load_arg(node_t *node, int idx) {
  if(node->type->ty == INT)
    printf("\tmov [rbp-%d], %s\n", node->offset, argregw[idx]);
  else
    printf("\tmov [rbp-%d], %s\n", node->offset, argregd[idx]);
}

void gen(node_t *node) {
  if(node->kind == ND_FUNC) {
    printf("%s:\n", node->name);
    // prologue
    // allocate area of 26 variables
    printf("\tpush rbp\n");
    printf("\tmov rbp, rsp\n");
    printf("\tsub rsp, %d\n", node->offset);
    lvarOffset = node->offset;
    
    int argnum = 0;
    for(node_t *arg = node->arg; arg; arg = arg->arg) {
      load_arg(arg, argnum++);
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

    case ND_CALL:
      int argnum = 0;
      for(node_t *arg = node->arg; arg; arg = arg->arg) {
        gen(arg);
        printf("\tpop %s\n", argregd[argnum++]);
      }

      // adjust rsp to multiple of 16
      int offset = 16-lvarOffset%16;
      printf("\tsub rsp, %d\n", offset);

      printf("\tcall %s\n", node->name);
      printf("\tadd rsp, %d\n", offset);
      printf("\tpush rax\n");
      return;

    case ND_NUM:
      printf("\tpush %d\n", node->val);
      return;

    case ND_LVAR:
      gen_lval(node);
      load(node->type);
      return;

    case ND_ASSIGN:
      if(node->lhs->kind == ND_DEREF) {
        gen_addr(node->lhs);
        load(node->lhs->type);
      }
      else  gen_lval(node->lhs);
      gen(node->rhs);
      
      printf("\tpop rdi\n");
      printf("\tpop rax\n");
      if(node->lhs->type->ty == INT)
        printf("\tmov dword ptr [rax], edi\n");
      else
        printf("\tmov [rax], rdi\n");
      printf("\tpush rdi\n");
      return;

    case ND_ADDR:
      gen_addr(node);
      printf("\n\n\n");
      return;

    case ND_DEREF:
      gen_addr(node);
      load(node->type);
      load(node->type->ptr_to);
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
