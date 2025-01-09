#include "Node.h"
#include "Parser.h"
#include <stdio.h>

typedef struct Hoister {
  Node enums, structs, funcs;
} Hoister;

void throwHoisterError(Hoister *h, char *fileName, int line, char msg[]) {
  printf("Error in the Hoister!\n"
         "Error found in file: %s\nOn line: %i\n\n%s\n",
         fileName, line, msg);
  exit(1);
}

void hoist(Hoister *h, Parser parsers[], int parserCount) {
  for (int i = 0; i < parserCount; ++i) {
    Parser *p = parsers + i;

    for (int j = 0; j < p->out.children.len; ++j) {
      Node *n = p->out.children.p + j;

      switch (n->kind) {

      default:
        throwHoisterError(h, p->sourceName, n->line,
                          "Invalid top level statement");
      }
    }
  }
}
