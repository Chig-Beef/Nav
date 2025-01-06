#include "Node.h"
#include "Panic.h"
#include "Token.h"
#include "list.h"
#include <stdio.h>

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

void throwError(Parser *p, int line, char expected[], Token got) {
  printf("Error in the Parser!\n"
         "Error found in file: %s\nOn line: %i\nExpected: %s\nGot: %s\n",
         p->sourceName, line, expected, tokenString(&got));
  exit(1);
}

// The main program
Node parse(Parser *p) {
  const char FUNC_NAME[] = "program";

  Node program = newNode(N_PROGRAM, NULL, 0);
  Node *n;

  nextToken(p);

  while (p->tok.kind != T_ILLEGAL) {
    switch (p->tok.kind) {
    case T_ENUM:
      break;
    case T_FUN:
      break;
    case T_STRUCT:
      break;

      // Statements
    case T_CALL:
      break;
    case T_LET:
      break;
    case T_IDENTIFIER:
      break;
    case T_IF:
      break;
    case T_FOR:
      break;
    case T_RETURN:
      break;
    case T_BREAK:
      break;
    case T_CONTINUE:
      break;
    case T_SWITCH:
      break;
    default:
      throwError(p, p->tok.line, "Valid start to line", p->tok);
    }

    nextToken(p);
  }

  return program;
}
