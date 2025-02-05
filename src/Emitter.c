#include "Emitter.h"
#include "Node.h"
#include "Panic.h"
#include "list.h"
#include <stdio.h>

NEW_LIST_TYPE_HEADER(char, Char)

#define PUSH_CHAR(character)                                                   \
  if (CharListAppend(out, (character))) {                                      \
    panic("Couldn't append to output");                                        \
  }

void emitterInit(Emitter *e, Node enums, Node structs, Node funs) {
  e->inEnums = enums;
  e->inStructs = structs;
  e->inFuns = funs;
}

void emitEnum(Emitter *e, CharList *out, Node n) {
  char *enumName = n.children.p[1].data->data;

  // typedef
  PUSH_CHAR('t')
  PUSH_CHAR('y')
  PUSH_CHAR('p')
  PUSH_CHAR('e')
  PUSH_CHAR('d')
  PUSH_CHAR('e')
  PUSH_CHAR('f')
  PUSH_CHAR(' ')

  // typedef enum
  PUSH_CHAR('e')
  PUSH_CHAR('n')
  PUSH_CHAR('u')
  PUSH_CHAR('m')
  PUSH_CHAR(' ')

  // typedef enum Test
  int i = 0;
  while (enumName[i]) {
    PUSH_CHAR(enumName[i])
    ++i;
  }
  PUSH_CHAR(' ')

  // typedef enum Test {
  PUSH_CHAR('{')

  for (int i = 3; i < n.children.len - 1; ++i) {
    Node node = n.children.p[i];

    if (node.kind == N_SEP) {
      PUSH_CHAR(',')
    } else if (node.kind == N_IDENTIFIER) {
      PUSH_CHAR('\n')
      PUSH_CHAR('\t')

      char *childName = node.data->data;
      int i = 0;
      while (childName[i]) {
        PUSH_CHAR(childName[i])
        ++i;
      }
    }
  }

  // typedef enum Test { }
  PUSH_CHAR('\n')
  PUSH_CHAR('}')
  PUSH_CHAR(' ')

  // typedef enum Test { } Test;

  // typedef enum Test
  i = 0;
  while (enumName[i]) {
    PUSH_CHAR(enumName[i])
    ++i;
  }
  PUSH_CHAR(';')
  PUSH_CHAR('\n')
  PUSH_CHAR('\n')
}

void emitEnums(Emitter *e, CharList *out) {
  for (int i = 0; i < e->inEnums.children.len; i++) {
    emitEnum(e, out, e->inEnums.children.p[i]);
  }
}

void emitStructs(Emitter *e, CharList *out) {
  // char *enumName = n.children.p[1].data->data;
}

void emitFuns(Emitter *e, CharList *out) {}

char *emit(Emitter *e) {
  CharList out;

  if (CharListInit(&out, 1)) {
    panic("Couldn't initialise output list");
  }

  emitEnums(e, &out);
  emitStructs(e, &out);
  emitFuns(e, &out);

  printf("%s\n", out.p);

  return out.p;
}
