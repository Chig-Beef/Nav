#pragma once

#include "StringManager.h"
#include "Token.h"
#include "list.h"

#include <stdio.h>

typedef struct Lexer {
  char *sourceName;
  FILE *source;
  char curChar;
  char peekChar;
  int line;
  TokenList out;
  StringManager *sm;
} Lexer;

void lexerInit(Lexer *l, char sourceName[], FILE *source, StringManager *sm);

// Returns the tokens of the source in the lexer, the resulting list must be
// destroyed
void lex(Lexer *l);
