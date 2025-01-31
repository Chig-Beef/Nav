#include "Token.h"
#include "Panic.h"
#include "String.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tokenDestroy(Token *t) { strFree(t->data); }

char *tokenCodeString(TokenCode tc) {
  switch (tc) {
  case T_ILLEGAL:
    return "T_ILLEGAL";

    // Keywords
  case T_BREAK:
    return "T_BREAK";
  case T_CALL:
    return "T_CALL";
  case T_CASE:
    return "T_CASE";
  case T_CONST:
    return "T_CONST";
  case T_CONTINUE:
    return "T_CONTINUE";
  case T_DEFAULT:
    return "T_DEFAULT";
  case T_ELIF:
    return "T_ELIF";
  case T_ELSE:
    return "T_ELSE";
  case T_ENUM:
    return "T_ENUM";
  case T_FOR:
    return "T_FOR";
  case T_FUN:
    return "T_FUN";
  case T_IF:
    return "T_IF";
  case T_LET:
    return "T_LET";
  case T_MAKE:
    return "T_MAKE";
  case T_NEW:
    return "T_NEW";
  case T_RETURN:
    return "T_RETURN";
  case T_STRUCT:
    return "T_STRUCT";
  case T_SWITCH:
    return "T_SWITCH";
  case T_TRUE:
    return "T_TRUE";
  case T_FALSE:
    return "T_FALSE";

    // Various symbols
  case T_ACCESSOR:
    return "T_ACCESSOR";
  case T_ASSIGN:
    return "T_ASSIGN";
  case T_COLON:
    return "T_COLON";
  case T_SEMICOLON:
    return "T_SEMICOLON";
  case T_SEP:
    return "T_SEP";

    // Operators
  case T_ADD:
    return "T_ADD";
  case T_AND:
    return "T_AND";
  case T_ANDAND:
    return "T_ANDAND";
  case T_DIV:
    return "T_DIV";
  case T_EQ:
    return "T_EQ";
  case T_GT:
    return "T_GT";
  case T_GTEQ:
    return "T_GTEQ";
  case T_LT:
    return "T_LT";
  case T_LTEQ:
    return "T_LTEQ";
  case T_MOD:
    return "T_MOD";
  case T_MUL:
    return "T_MUL";
  case T_NEQ:
    return "T_NEQ";
  case T_OR:
    return "T_OR";
  case T_OROR:
    return "T_OROR";
  case T_SUB:
    return "T_SUB";
  case T_XOR:
    return "T_XOR";
  case T_L_SHIFT:
    return "T_L_SHIFT";
  case T_R_SHIFT:
    return "T_R_SHIFT";

    // Unary operators
  case T_DEREF:
    return "T_DEREF";
  case T_DEC:
    return "T_DEC";
  case T_INC:
    return "T_INC";
  case T_NOT:
    return "T_NOT";
  case T_REF:
    return "T_REF";

    // Paired symbols
  case T_L_BLOCK:
    return "T_L_BLOCK";
  case T_R_BLOCK:
    return "T_R_BLOCK";
  case T_L_SQUIRLY:
    return "T_L_SQUIRLY";
  case T_R_SQUIRLY:
    return "T_R_SQUIRLY";
  case T_L_PAREN:
    return "T_L_PAREN";
  case T_R_PAREN:
    return "T_R_PAREN";

    // Values
  case T_CHAR:
    return "T_CHAR";
  case T_FLOAT:
    return "T_FLOAT";
  case T_IDENTIFIER:
    return "T_IDENTIFIER";
  case T_INT:
    return "T_INT";
  case T_STRING:
    return "T_STRING";

  default:
    return "UNKNOWN";
  }
}

// Returns a string describing the token. The resulting string must be freed.
char *tokenString(Token t) {
  // Get text of token code
  char *kind = tokenCodeString(t.kind);

  // The final string
  char *out;

  if (t.data) {
    out = malloc((strlen(kind) + strlen(t.data->data) + 4) * sizeof(char));

    // Format
    sprintf(out, "(%s %s)", t.data->data, kind);
  } else {
    out = malloc((strlen(kind) + 4 + 4) * sizeof(char));

    // Format
    sprintf(out, "(NULL %s)", kind);
  }
  if (out == NULL) {
    panic("Couldn't make string for tokenString");
  }

  // printf("Formatted final string\n");

  return out;
}

errno_t TokenListInit(TokenList *l, int initialSize) {
  if (initialSize < 0) {
    return 1;
  }
  l->len = 0;
  l->cap = initialSize;
  l->p = (Token *)calloc(initialSize, sizeof(Token));
  if (l->p == ((void *)0)) {
    return 1;
  }
  return 0;
}
void TokenListDestroy(TokenList *l) { free(l->p); }
errno_t TokenListAppend(TokenList *l, Token item) {
  if (l->len == l->cap) {
    l->cap *= 2;
    Token *newP = (Token *)calloc(l->cap, sizeof(Token));
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
TokenList TokenListCopy(TokenList *src) {
  TokenList dest;
  dest.len = src->len;
  dest.cap = src->len;
  dest.p = (Token *)calloc(dest.cap, sizeof(Token));
  for (int i = 0; i < src->len; ++i) {
    dest.p[i] = src->p[i];
  }
  return dest;
}
errno_t TokenListRemoveAt(TokenList *l, int index) {
  if (index < 0 || index >= l->len) {
    return 1;
  }
  --l->len;
  for (int i = index; i < l->len; ++i) {
    l->p[i] = l->p[i + 1];
  }
  return 0;
}
