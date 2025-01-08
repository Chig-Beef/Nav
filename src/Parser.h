#pragma once

#include "Node.h"

typedef struct Parser {
  char *sourceName;
  TokenList source;
  Token tok;
  int index;
  Node out;
} Parser;

Node parse(Parser *p);
void parserInit(Parser *p, char *sourceName, TokenList source);
