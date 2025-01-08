#include "Node.h"
#include "list.h"
#include <string.h>

NEW_LIST_TYPE(char, Char)

Node newNode(NodeCode kind, char *data, int line) {
  NodeList children;

  if (NodeListInit(&children, 1)) {
    return ZERO_NODE;
  }

  return (Node){kind, children, data, line};
}

errno_t NodeListInit(NodeList *l, int initialSize) {
  if (initialSize < 0) {
    return 1;
  }
  l->len = 0;
  l->cap = initialSize;
  l->p = (Node *)calloc(initialSize, sizeof(Node));
  if (l->p == ((void *)0)) {
    return 1;
  }
  return 0;
}
void NodeListDestroy(NodeList *l) { free(l->p); }
errno_t NodeListAppend(NodeList *l, Node item) {
  if (l->len == l->cap) {
    l->cap *= 2;
    Node *newP = (Node *)calloc(l->cap, sizeof(Node));
    if (newP == ((void *)0)) {
      return 1;
    }
    for (int i = 0; i < l->len; ++i) {
      newP[i] = l->p[i];
    }
    free(l->p);
    l->p = newP;
  }
  l->p[l->len] = item;
  ++l->len;
  return 0;
}
NodeList NodeListCopy(NodeList *src) {
  NodeList dest;
  dest.len = src->len;
  dest.cap = src->len;
  dest.p = (Node *)calloc(dest.cap, sizeof(Node));
  for (int i = 0; i < src->len; ++i) {
    dest.p[i] = src->p[i];
  }
  return dest;
}
errno_t NodeListRemoveAt(NodeList *l, int index) {
  if (index < 0 || index >= l->len) {
    return 1;
  }
  --l->len;
  for (int i = index; i < l->len; ++i) {
    l->p[i] = l->p[i + 1];
  }
  return 0;
}

// Recursively frees all children
void nodeDestroy(Node *n) {
  if (n->data) {
    free(n->data);
  }

  for (int i = 0; i < n->children.len; ++i) {
    nodeDestroy(n->children.p + i);
    free(n->children.p + i);
  }
}

char *nodeCodeString(NodeCode nc) {
  switch (nc) {
  case N_ILLEGAL:
    return "N_ILLEGAL";

    // Structures
  case N_PROGRAM:
    return "N_PROGRAM";
  case N_ENUM_DEF:
    return "N_ENUM_DEF";
  case N_FUNC_DEF:
    return "N_FUNC_DEF";
  case N_STRUCT_DEF:
    return "N_STRUCT_DEF";
  case N_COMPLEX_TYPE:
    return "N_COMPLEX_TYPE";
  case N_BLOCK:
    return "N_BLOCK";
  case N_INDEX:
    return "N_INDEX";
  case N_EXPRESSION:
    return "N_EXPRESSION";
  case N_UNARY_VALUE:
    return "N_UNARY_VALUE";
  case N_VALUE:
    return "N_VALUE";
  case N_OPERATOR:
    return "N_OPERATOR";
  case N_BRACKETED_VALUE:
    return "N_BRACKETED_VALUE";
  case N_LONE_CALL:
    return "N_LONE_CALL";
  case N_MAKE_ARRAY:
    return "N_MAKE_ARRAY";
  case N_FUNC_CALL:
    return "N_FUNC_CALL";
  case N_STRUCT_NEW:
    return "N_STRUCT_NEW";
  case N_VAR_DEC:
    return "N_VAR_DEC";
  case N_NEW_ASSIGNMENT:
    return "N_NEW_ASSIGNMENT";
  case N_CREMENT:
    return "N_CREMENT";
  case N_ASSIGNMENT:
    return "N_ASSIGNMENT";
  case N_IF_BLOCK:
    return "N_IF_BLOCK";
  case N_IF_SEC:
    return "N_IF_SEC";
  case N_ELIF_SEC:
    return "N_ELIF_SEC";
  case N_ELSE_SEC:
    return "N_ELSE_SEC";
  case N_FOR_LOOP:
    return "N_FOR_LOOP";
  case N_RET_STATE:
    return "N_RET_STATE";
  case N_BREAK_STATE:
    return "N_BREAK_STATE";
  case N_CONTINUE_STATE:
    return "N_CONTINUE_STATE";
  case N_SWITCH_STATE:
    return "N_SWITCH_STATE";
  case N_CASE_BLOCK:
    return "N_CASE_BLOCK";
  case N_DEFAULT_BLOCK:
    return "N_DEFAULT_BLOCK";

    // Keywords
  case N_BREAK:
    return "N_BREAK";
  case N_CALL:
    return "N_CALL";
  case N_CASE:
    return "N_CASE";
  case N_CONST:
    return "N_CONST";
  case N_CONTINUE:
    return "N_CONTINUE";
  case N_DEFAULT:
    return "N_DEFAULT";
  case N_ELIF:
    return "N_ELIF";
  case N_ELSE:
    return "N_ELSE";
  case N_ENUM:
    return "N_ENUM";
  case N_FOR:
    return "N_FOR";
  case N_FUN:
    return "N_FUN";
  case N_IF:
    return "N_IF";
  case N_LET:
    return "N_LET";
  case N_MAKE:
    return "N_MAKE";
  case N_NEW:
    return "N_NEW";
  case N_RETURN:
    return "N_RETURN";
  case N_STRUCT:
    return "N_STRUCT";
  case N_SWITCH:
    return "N_SWITCH";
  case N_TRUE:
    return "N_TRUE";
  case N_FALSE:
    return "N_FALSE";
  case N_NIL:
    return "N_NIL";

    // Various symbols
  case N_ACCESS:
    return "N_ACCESS";
  case N_ASSIGN:
    return "N_ASSIGN";
  case N_COLON:
    return "N_COLON";
  case N_SEMICOLON:
    return "N_SEMICOLON";
  case N_SEP:
    return "N_SEP";

    // Operators
  case N_ADD:
    return "N_ADD";
  case N_AND:
    return "N_AND";
  case N_ANDAND:
    return "N_ANDAND";
  case N_DIV:
    return "N_DIV";
  case N_EQ:
    return "N_EQ";
  case N_GT:
    return "N_GT";
  case N_GTEQ:
    return "N_GTEQ";
  case N_LT:
    return "N_LT";
  case N_LTEQ:
    return "N_LTEQ";
  case N_MOD:
    return "N_MOD";
  case N_MUL:
    return "N_MUL";
  case N_NEQ:
    return "N_NEQ";
  case N_OR:
    return "N_OR";
  case N_OROR:
    return "N_OROR";
  case N_SUB:
    return "N_SUB";
  case N_XOR:
    return "N_XOR";
  case N_L_SHIFT:
    return "N_L_SHIFT";
  case N_R_SHIFT:
    return "N_R_SHIFT";

    // Unary operators
  case N_DEREF:
    return "N_DEREF";
  case N_DEC:
    return "N_DEC";
  case N_INC:
    return "N_INC";
  case N_NOT:
    return "N_NOT";
  case N_REF:
    return "N_REF";

    // Paired symbols
  case N_L_BLOCK:
    return "N_L_BLOCK";
  case N_R_BLOCK:
    return "N_R_BLOCK";
  case N_L_SQUIRLY:
    return "N_L_SQUIRLY";
  case N_R_SQUIRLY:
    return "N_R_SQUIRLY";
  case N_L_PAREN:
    return "N_L_PAREN";
  case N_R_PAREN:
    return "N_R_PAREN";

    // Values
  case N_CHAR:
    return "N_CHAR";
  case N_FLOAT:
    return "N_FLOAT";
  case N_IDENTIFIER:
    return "N_IDENTIFIER";
  case N_INT:
    return "N_INT";
  case N_STRING:
    return "N_STRING";

  default:
    return "UNKNOWN";
  }
}

errno_t stringRec(Node *n, CharList *out, int indent) {
  // Indent
  for (int i = 0; i < indent; ++i) {
    if (CharListAppend(out, '\t')) {
      return 1;
    }
  }

  // Node code
  char *code = nodeCodeString(n->kind);
  for (int i = 0; code[i]; ++i) {
    if (CharListAppend(out, code[i])) {
      return 1;
    }
  }

  if (CharListAppend(out, ':')) {
    return 1;
  }
  if (CharListAppend(out, ' ')) {
    return 1;
  }

  // Data
  for (int i = 0; n->data[i]; ++i) {
    if (CharListAppend(out, n->data[i])) {
      return 1;
    }
  }

  if (CharListAppend(out, '\n')) {
    return 1;
  }

  for (int i = 0; i < n->children.len; ++i) {
    if (stringRec((n->children.p) + i, out, indent + 1)) {
      return 1;
    }
  }

  return 0;
}

// Returns a string describing the token. The resulting string must be freed.
char *nodeString(Node *n) {
  CharList out;

  if (CharListInit(&out, 8)) {
    return NULL;
  }

  // Node code
  char *code = nodeCodeString(n->kind);
  for (int i = 0; code[i]; ++i) {
    if (CharListAppend(&out, code[i])) {
      return NULL;
    }
  }

  if (CharListAppend(&out, ':')) {
    return NULL;
  }
  if (CharListAppend(&out, ' ')) {
    return NULL;
  }

  // Data
  for (int i = 0; n->data[i]; ++i) {
    if (CharListAppend(&out, n->data[i])) {
      return NULL;
    }
  }

  if (CharListAppend(&out, '\n')) {
    return NULL;
  }

  for (int i = 0; i < n->children.len; ++i) {
    if (stringRec((n->children.p) + i, &out, 1)) {
      return NULL;
    }
  }

  return out.p;
}
