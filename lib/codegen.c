#include "codegen.h"

char *argregw[6] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
char *argregd[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_addr(node_t *node) {
  var_t *var = node->var;
  if(var->is_local) {
    printf("\tmov rax, rbp\n");
    printf("\tsub rax, %d\n", var->offset);
    printf("\tpush rax\n");
  } else {
    printf("\tpush offset %s\n", var->name);
  }
}

void gen_lval(node_t *node) {
  if (node->kind != ND_VAR)
    error("lhs of assignment is not variable, but got %d", node->kind);
  gen_addr(node);
}

int beginLabelCnt = 0;
int endLabelCnt = 0;
int elseLabelCnt = 0;
int callLabelCnt = 0;
int lvarOffset = 0;

void load(type_t *ty) {
  printf("\tpop rax\n");

  if(ty->ty == ARRAY) ty = ty->ptr_to;

  int size = size_of(ty);

  switch(size) {
    case 1:
      printf("\tmovsx rax, byte ptr [rax]\n");
      break;
    case 4:
      printf("\tmovsx rax, dword ptr [rax]\n");
      break;
    default:
      printf("\tmov rax, [rax]\n");
      break;
  }
  printf("\tpush rax\n");
}

void load_arg(node_t *node, int idx) {
  var_t *var = node->var;
  if(var->type->ty == INT)
    printf("\tmov [rbp-%d], %s\n", var->offset, argregw[idx]);
  else
    printf("\tmov [rbp-%d], %s\n", var->offset, argregd[idx]);
}

void gen(node_t *node) {
  if(node->kind == ND_FUNC) {
    printf("%s:\n", node->name);
    // prologue
    // allocate area of 26 variables
    printf("\tpush rbp\n");
    printf("\tmov rbp, rsp\n");
    printf("\tsub rsp, %d\n", node->offset + (16 - node->offset%16));
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
        printf("\tpop rax\n");
        printf("\tjmp .Lend%d\n", endLabelCnt);
        printf(".Lelse%d:\n", elseLabelCnt++);
        gen(node->els);
        printf("\tpop rax\n");
      } else {
        printf("\tje .Lend%d\n", endLabelCnt);
        gen(node->then);
        printf("\tpop rax\n");
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
      printf("\tpop rax\n");
      printf("\tjmp .Lbegin%d\n", beginLabelCnt++);
      printf(".Lend%d:\n", endLabelCnt++);
      printf("\tpush rax\n");
      return;

    case ND_FOR:
      gen(node->init);
      printf("\tpop rax\n");
      printf(".Lbegin%d:\n", beginLabelCnt);
      gen(node->cond);
      printf("\tpop rax\n");
      printf("\tcmp rax, 0\n");
      printf("\tje .Lend%d\n", endLabelCnt);
      gen(node->then);
      printf("\tpop rax\n");
      gen(node->adv);
      printf("\tpop rax\n");
      printf("\tjmp .Lbegin%d\n", beginLabelCnt++);
      printf(".Lend%d:\n", endLabelCnt++);
      printf("\tpush rax\n");
      return;

    case ND_CALL:
      int argnum = 0;
      for(node_t *arg = node->arg; arg; arg = arg->arg) {
        gen(arg);
        argnum++;
      }
      for(int i=argnum-1; i>=0; i--)
        printf("\tpop %s\n", argregd[i]);

      // adjust rsp to multiple of 16
      // printf("\tsub rsp, %d\n", offset);

      /*
      printf("\tcall %s\n", node->name);
      printf("\tadd rsp, %d\n", offset);
      printf("\tpush rax\n");
      return;
      */
      int seq = callLabelCnt++;
      printf("\tmov rax, rsp\n");
      printf("\tand rax, 15\n");
      printf("\tjnz .Lcall%d\n", seq);
      printf("\tmov rax, 0\n");
      printf("\tcall %s\n", node->name);
      printf("\tjmp .Lcend%d\n", seq);
      printf("\t.Lcall%d:\n", seq);
      printf("\tsub rsp, %d\n", 8);
      printf("\tmov rax, 0\n");
      printf("\tcall %s\n", node->name);
      printf("\tadd rsp, %d\n", 8);
      printf("\t.Lcend%d:\n", seq);
      printf("\tpush rax\n");
      return;

    case ND_NUM:
      printf("\tpush %d\n", node->val);
      return;

    case ND_VAR:
      gen_addr(node);
      if(node->type->ty!=ARRAY)
        load(node->type);
      return;

    case ND_ASSIGN:
      type_t *ty = node->lhs->type;
      if(node->lhs->kind == ND_DEREF) {
        gen(node->lhs->lhs);
        ty = ty->ptr_to;
      }
      else  gen_lval(node->lhs);
      gen(node->rhs);
      
      printf("\tpop rdi\n");
      printf("\tpop rax\n");
      if(ty->ty == INT)
        printf("\tmov dword ptr [rax], edi\n");
      else
        printf("\tmov [rax], rdi\n");
      int size = size_of(ty);

      switch(size) {
        case 1:
          printf("\tmovsx rax, byte ptr [rax]\n");
          break;
        case 4:
          printf("\tmovsx rax, dword ptr [rax]\n");
          break;
        default:
          printf("\tmov rax, [rax]\n");
          break;
      }
      printf("\tpush rdi\n");
      return;

    case ND_ADDR:
      gen_addr(node->lhs);
      printf("\n\n\n");
      return;

    case ND_DEREF:
      gen(node->lhs);
      load(node->type);
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("\tpop rdi\n");
  printf("\tpop rax\n");

  switch(node->kind) {
    case ND_ADD:
      if(node->type->ptr_to) {
        if(node->type->ptr_to->ty == INT)
          printf("\timul rdi, %d\n", 4);
        if(node->type->ptr_to->ty == PTR)
          printf("\timul rdi, %d\n", 8);
      }
      printf("\tadd rax, rdi\n");
      break;
    case ND_SUB:
      if(node->type->ptr_to) {
        if(node->type->ptr_to->ty == INT)
          printf("\timul rdi, %d\n", 4);
        if(node->type->ptr_to->ty == PTR)
          printf("\timul rdi, %d\n", 8);
      }
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

void emit_data() {
  printf(".data\n");
  for(var_t *var=globals; var; var=var->next) {
    printf("%s:\n", var->name);
    if(!var->str)
      printf("\t.zero %d\n", size_of(var->type));
    else
      printf("\t.string \"%s\"\n", var->str);
  }
}

void emit_text() {
  printf(".text\n");

  func_t *f = funcs;
  printf(".globl ");
  for(;;) {
    printf("%s", f->name);
    f = f->next;
    if(f)  printf(", ");
    else break;
  }
  printf("\n");

  for (int i=0; code[i]; i++) {
    gen(code[i]);
  }
}

void codegen() {
  // output a first half of assembry
  printf(".intel_syntax noprefix\n");

  emit_data();
  emit_text();
}

