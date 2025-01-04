#include "Panic.h"
#include "Token.h"
#include "list.h"

NEW_LIST_TYPE(Token, Token)

typedef struct Parser {
  char *sourceName;
  TokenList source;
  Token tok;
  int index;
} Parser;

void nextToken(Parser *p) {
  Token t = {NULL, T_ILLEGAL, 0};

  if (p->index < p->source.len) {
    t = p->source.p[p->index];
  }

  ++p->index;
  p->tok = t;
}

Token peekToken(Parser *p) {
  if (p->index < p->source.len) {
    return p->source.p[p->index];
  }

  return (Token){NULL, T_ILLEGAL, 0};
}
