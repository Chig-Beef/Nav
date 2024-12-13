#include "Panic.h"
#include "Token.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>

NEW_LIST_TYPE(Token, Token)

typedef struct Lexer {
  char *sourceName;
  char *source;
  int len;
  int index;
  int line;
} Lexer;

char peekChar(Lexer *l) {
  int nextIndex = l->index + 1;

  if (nextIndex >= l->len) {
    return 0;
  }

  return l->source[nextIndex];
}

void throwError(Lexer *l, char caller[], int line, char expected[], char *got) {
  printf("Error in the Lexer!\nWhen the Lexer was trying to decipher: "
         "%s\nError found in file: %s\nOn line: %i\nExpected: %s\nGot: %s\n",
         caller, l->sourceName, line, expected, got);
  exit(1);
}

// Returns the tokens of the source in the lexer, the resulting list must be
// destroyed
TokenList lex(Lexer *l) {
  TokenList tokens;
  if (TokenListInit(&tokens, 8)) {
    panic("Couldn't initialise token list.");
  }
  Token token;

  return tokens;
}
