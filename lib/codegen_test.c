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
}

void print_node(node_t *node, int layer) {
  if(node->kind == ND_NUM) {
    printf("%d\n", node->val);
    return;
  }
  if(node->kind == ND_LVAR) {
    char t = (char)('a' + (node->offset)/8 -1);
    printf("%c\n", t);
    return;
  }
  printf("%s\n", node_kind_to_string(node->kind));
  for(int i=0; i<layer; i++)  printf("  ");
  printf("|-");
  print_node(node->lhs, layer+1);
  printf("\n");
  for(int i=0; i<layer; i++)  printf("  ");
  printf("|-");
  print_node(node->rhs, layer+1);
}

bool statement_test(char *test_statement) {
  printf("%s\n", test_statement);
  token = tokenize(test_statement);
  node_t *node = stmt();

  print_node(node, 0);
  printf("\n\n");
}

int main() {
  statement_test("1+2;");
  statement_test("1+3*2*(3+4)+5;");
  statement_test("a=0;");
}
