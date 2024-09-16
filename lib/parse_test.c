#include <codegen.h>
#include <stdbool.h>

#define STR(var) #var

char *node_kind_to_string(NodeKind kind) {
  if(kind == ND_ADD) return "+";
  else if(kind == ND_SUB) return"-";
  else if(kind == ND_MUL) return "*";
  else if(kind == ND_DIV) return "/";
  else if(kind == ND_EQ)  return "==";
  else if(kind == ND_NE)  return "!=";
  else if(kind == ND_LT)  return "<";
  else if(kind == ND_LE)  return "<=";
  else if(kind == ND_ASSIGN)  return "=";
  else if(kind == ND_BLOCK) return "{}";
  else if(kind == ND_FOR) return "for";
  else if(kind == ND_CALL)  return "call";
  else if(kind == ND_FUNC) return "func";
  else if(kind == ND_RETURN) return "ret";
  else if(kind == ND_ADDR) return "&";
  else if(kind == ND_DEREF) return "deref";
  else return "?";
}

void print_node(node_t *node, int layer) {
  if(node->kind == ND_NUM) {
    printf("%d\n", node->val);
    return;
  }
  if(node->kind == ND_VAR) {
    char c = (char)('a' + (node->offset / 8) - 1);
    var_t *var = node->var;
    printf("%s -il:%d - offset: %d\n", var->name, var->is_local, var->offset);
    return;
  }
  if(node->kind == ND_FOR) {
    printf("%s\n", node_kind_to_string(node->kind));
    for(int i=0; i<layer; i++)  printf("  ");
    printf("|- ");
    print_node(node->init, layer+1);
    printf("\n");
    for(int i=0; i<layer; i++)  printf("  ");
    printf("|- ");
    print_node(node->cond, layer+1);
    printf("|- ");
    print_node(node->adv, layer+1);
    printf("|- ");
    print_node(node->then, layer+1);
  } else if(node->kind == ND_BLOCK) {
    printf("%s\n", node_kind_to_string(node->kind));
    while(node->stmt) {
      for(int i=0; i<layer; i++)  printf("  ");
      printf("|- ");
      print_node(node->stmt, layer+1);
      node=node->stmt;
    }
  } else if(node->kind == ND_FUNC) {
    printf("%s\n", node_kind_to_string(node->kind));
    node_t *arg = node->arg;
    while(arg) {
      for(int i=0; i<layer; i++)  printf("  ");
      printf("|- ");
      print_node(arg, layer+1);
      arg=arg->arg;
    }
    for(int i=0; i<layer; i++)  printf("  ");
    printf("|- ");
    print_node(node->stmt, layer+1);
    printf("\n");
  } else if(node->kind == ND_CALL) {
    printf("%s\n", node_kind_to_string(node->kind));
    node_t *arg = node->arg;
    while(arg) {
      for(int i=0; i<layer; i++)  printf("  ");
      printf("|- ");
      print_node(arg, layer+1);
      arg=arg->arg;
    }
  } else if(node->kind == ND_RETURN) {
    printf("%s\n", node_kind_to_string(node->kind));
    for(int i=0; i<layer; i++)  printf("  ");
    printf("|- ");
    print_node(node->lhs, layer+1);
  } else if(node->kind == ND_ADDR) {
    printf("%s\n", node_kind_to_string(node->kind));
    for(int i=0; i<layer; i++)  printf("  ");
    printf("|- ");
    print_node(node->lhs, layer+1);
  } else if(node->kind == ND_DEREF) {
    printf("%s\n", node_kind_to_string(node->kind));
    for(int i=0; i<layer; i++)  printf("  ");
    printf("|- ");
    print_node(node->lhs, layer+1);
  } else {
    printf("%s\n", node_kind_to_string(node->kind));
    for(int i=0; i<layer; i++)  printf("  ");
    printf("|- ");
    print_node(node->lhs, layer+1);
    for(int i=0; i<layer; i++)  printf("  ");
    printf("|- ");
    print_node(node->rhs, layer+1);
  }
}

bool statement_test(char *test_statement) {
  printf("%s\n", test_statement);
  token = tokenize(test_statement);
  node_t *node = program();
  for(int i=0; code[i]; i++) {
    print_node(code[i], 0);
  }
  printf("\n\n");
}

int main() {
  /*
  statement_test("int func() {1+2;}");
  statement_test("int func() {1+3*2*(3+4)+5;}");
  statement_test("int func() {int a;a=0;}");
  statement_test("int func() {int i;int k;for(i=0;i<10;i=i+1) { k+1; }}");
  statement_test("int func(int i, int j) {int a;funca(a, 1);}");
  statement_test("int main() {int a;a=2;return foo(1, a);} foo(int b, int c) {return b+c;}");
  statement_test("int main() {int a; a=1; int *b; b=&a; return *b;}");
  statement_test("int main() {int *a; *(a+1) = 0;}");
  statement_test("int main() {int a[5]; *a = 0; *(a+1) = 0;}");
  statement_test("int main() {int a[5]; a[2] = 0; 2[a] = 0;}");
  statement_test("int a; int main() {return 0;}");
  */
  statement_test("int main() {int a;int b; a=0;b=0;return 0;}");
}
