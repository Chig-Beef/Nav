#include "Hoister.h"
#include "Node.h"
#include "Panic.h"
#include "Parser.h"
#include <stdio.h>

void throwHoisterError(Hoister *h, char *fileName, int line, char msg[]) {
  printf("Error in the Hoister!\n"
         "Error found in file: %s\nOn line: %i\n\n%s\n",
         fileName, line, msg);
  exit(1);
}

void hoist(Hoister *h, Parser parsers[], int parserCount) {
  h->enums = (Node){N_PROGRAM, (NodeList){}, NULL, 0};
  if (NodeListInit(&h->enums.children, 1)) {
    panic("Couldn't init hoister enums");
  }

  h->structs = (Node){N_PROGRAM, (NodeList){}, NULL, 0};
  if (NodeListInit(&h->structs.children, 1)) {
    panic("Couldn't init hoister structs");
  }

  h->funcs = (Node){N_PROGRAM, (NodeList){}, NULL, 0};
  if (NodeListInit(&h->funcs.children, 1)) {
    panic("Couldn't init hoister funcs");
  }

  for (int i = 0; i < parserCount; ++i) {
    Parser *p = parsers + i;

    for (int j = 0; j < p->out.children.len; ++j) {
      Node *n = p->out.children.p + j;

      switch (n->kind) {
      case N_ENUM_DEF:
        if (NodeListAppend(&h->enums.children, *n)) {
          panic("Couldn't append to hoister's enums");
        }
        break;
      case N_STRUCT_DEF:
        if (NodeListAppend(&h->structs.children, *n)) {
          panic("Couldn't append to hoister's structs");
        }
        break;
      case N_FUNC_DEF:
        if (NodeListAppend(&h->funcs.children, *n)) {
          panic("Couldn't append to hoister's funcs");
        }
        break;
      default:
        throwHoisterError(h, p->sourceName, n->line,
                          "Invalid top level statement");
      }
    }
  }
}
