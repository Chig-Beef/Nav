#include "Token.h"
#include "list.h"
#include <stdio.h>

NEW_LIST_TYPE_HEADER(Token, Token)

typedef struct Lexer {
  char *sourceName;
  FILE *source;
  char curChar;
  char peekChar;
  int line;
  TokenList out;
} Lexer;

void lexerInit(Lexer *l, char sourceName[], FILE *source);

// Returns the tokens of the source in the lexer, the resulting list must be
// destroyed
void lex(Lexer *l);
