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

void emitIdentifier(Emitter *e, CharList *out, Node n) {
  char *name = n.data->data;
  int i = 0;
  while (name[i]) {
    PUSH_CHAR(name[i])
    ++i;
  }
}

void emitIndex(Emitter *e, CharList *out, Node n) {
  PUSH_CHAR('[')
  // TODO: Emit expression
  PUSH_CHAR(']')
}

void emitComplexType(Emitter *e, CharList *out, Node n) {
  if (n.kind == N_IDENTIFIER) {
    emitIdentifier(e, out, n);
    return;
  }

  if (n.children.p[0].kind == N_INDEX) {
    emitComplexType(e, out, n.children.p[1]);
    emitIndex(e, out, n);
  } else { // Pointer
    emitComplexType(e, out, n.children.p[1]);
    PUSH_CHAR('*')
  }
}

void emitStruct(Emitter *e, CharList *out, Node n) {
  char *structName = n.children.p[1].data->data;

  // typedef
  PUSH_CHAR('t')
  PUSH_CHAR('y')
  PUSH_CHAR('p')
  PUSH_CHAR('e')
  PUSH_CHAR('d')
  PUSH_CHAR('e')
  PUSH_CHAR('f')
  PUSH_CHAR(' ')

  // typedef struct
  PUSH_CHAR('s')
  PUSH_CHAR('t')
  PUSH_CHAR('r')
  PUSH_CHAR('u')
  PUSH_CHAR('c')
  PUSH_CHAR('t')
  PUSH_CHAR(' ')

  // typedef struct Point
  int i = 0;
  while (structName[i]) {
    PUSH_CHAR(structName[i])
    ++i;
  }
  PUSH_CHAR(' ')

  // typedef struct Point {
  PUSH_CHAR('{')
  PUSH_CHAR('\n')
  PUSH_CHAR('\t')

  // typedef struct Point { int x; int y;
  for (int i = 3; i < n.children.len - 1; ++i) {
    Node node = n.children.p[i];
    if (node.kind == N_IDENTIFIER) {
      PUSH_CHAR(' ')
      emitIdentifier(e, out, node);
      PUSH_CHAR(' ')
    } else if (node.kind == N_COMPLEX_TYPE) {
      emitComplexType(e, out, node);
    } else if (node.kind == N_SEP) {
      PUSH_CHAR(';')
      PUSH_CHAR('\n')
      PUSH_CHAR('\t')
    }
  }

  // typedef struct Point { int x; int y; }
  PUSH_CHAR('\n')
  PUSH_CHAR('}')
  PUSH_CHAR('\n')
  PUSH_CHAR('\n')
}

void emitStructs(Emitter *e, CharList *out) {
  for (int i = 0; i < e->inStructs.children.len; i++) {
    emitStruct(e, out, e->inStructs.children.p[i]);
  }
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
