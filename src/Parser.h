#pragma once

#include "Node.h"
#include "StringManager.h"
#include "Token.h"

typedef struct Parser {
  char *sourceName;
  TokenList source;
  Token tok;
  int index;
  Node out;
  StringManager *sm;
} Parser;

void parse(Parser *p);
void parserInit(Parser *p, char *sourceName, TokenList source,
                StringManager *sm);
