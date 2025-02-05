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

void emitExpression(Emitter *e, CharList *out, Node n) {
  // TODO: Implement
}

void emitIndex(Emitter *e, CharList *out, Node n) {
  PUSH_CHAR('[')
  emitExpression(e, out, n.children.p[1]);
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

void emitFuncCall(Emitter *e, CharList *out, Node n) {
  emitIdentifier(e, out, n.children.p[1]);
  PUSH_CHAR('(')

  for (int i = 3; i < n.children.len - 1; ++i) {
    Node node = n.children.p[i];

    if (node.kind == N_EXPRESSION) {
      emitExpression(e, out, node);
    } else { // Sep
      PUSH_CHAR(',')
      PUSH_CHAR('\n')
    }
  }

  PUSH_CHAR(')')
}

void emitLoneCall(Emitter *e, CharList *out, Node n) {
  emitFuncCall(e, out, n.children.p[0]);
  PUSH_CHAR(';')
  PUSH_CHAR('\n')
}

void emitAccess(Emitter *e, CharList *out, Node n) {
  // TODO: Implement
}

void emitCrement(Emitter *e, CharList *out, Node n) {
  Node node = n.children.p[0];

  if (node.kind == N_INC) {
    PUSH_CHAR('+')
    PUSH_CHAR('+')
  } else {
    PUSH_CHAR('-')
    PUSH_CHAR('-')
  }

  node = n.children.p[1];

  if (node.kind == N_ACCESS) {
    emitAccess(e, out, node);
  } else {
    emitIdentifier(e, out, node);
  }

  if (n.children.len == 3) {
    emitIndex(e, out, n.children.p[2]);
  }
}

void emitAssignment(Emitter *e, CharList *out, Node n) {
  if (n.children.p[0].kind == N_CREMENT) {
    emitCrement(e, out, n.children.p[0]);
    return;
  }

  if (n.children.p[0].kind == N_ACCESS) {
    emitAccess(e, out, n.children.p[0]);
  } else {
    emitIdentifier(e, out, n.children.p[0]);
  }

  if (n.children.p[1].kind == N_INDEX) {
    emitIndex(e, out, n.children.p[1]);
  }

  PUSH_CHAR(' ')
  PUSH_CHAR('=')
  PUSH_CHAR(' ')

  emitExpression(e, out, n.children.p[n.children.len - 1]);
}

void emitNewAssignment(Emitter *e, CharList *out, Node n) {
  emitComplexType(e, out, n.children.p[1]);
  PUSH_CHAR(' ')
  emitIdentifier(e, out, n.children.p[2]);
  PUSH_CHAR(' ')
  PUSH_CHAR('=')
  PUSH_CHAR(' ')
  emitExpression(e, out, n.children.p[4]);
}

void emitVarDec(Emitter *e, CharList *out, Node n) {
  if (n.children.p[0].kind == N_ASSIGNMENT) {
    emitAssignment(e, out, n.children.p[0]);
  } else {
    emitNewAssignment(e, out, n.children.p[0]);
  }

  PUSH_CHAR(';');
  PUSH_CHAR('\n');
}

void emitIfBlock(Emitter *e, CharList *out, Node n) {
  // TODO: Implement
}

void emitForLoop(Emitter *e, CharList *out, Node n) {
  // TODO: Implement
}

void emitRetState(Emitter *e, CharList *out, Node n) {
  // TODO: Implement
}

void emitBreakState(Emitter *e, CharList *out, Node n) {
  // TODO: Implement
}

void emitContinueState(Emitter *e, CharList *out, Node n) {
  // TODO: Implement
}

void emitSwitchState(Emitter *e, CharList *out, Node n) {
  // TODO: Implement
}

void emitBlock(Emitter *e, CharList *out, Node n) {
  PUSH_CHAR('{')
  PUSH_CHAR('\n')

  for (int i = 1; i < n.children.len - 1; ++i) {
    Node node = n.children.p[i];

    switch (node.kind) {
    case N_LONE_CALL:
      emitLoneCall(e, out, node);
      break;
    case N_VAR_DEC:
      emitVarDec(e, out, node);
      break;
    case N_IF_BLOCK:
      emitIfBlock(e, out, node);
      break;
    case N_FOR_LOOP:
      emitForLoop(e, out, node);
      break;
    case N_RET_STATE:
      emitRetState(e, out, node);
      break;
    case N_BREAK_STATE:
      emitBreakState(e, out, node);
      break;
    case N_CONTINUE_STATE:
      emitContinueState(e, out, node);
      break;
    case N_SWITCH_STATE:
      emitSwitchState(e, out, node);
      break;
    default:
      panic("Invalid statement in block");
    }
  }

  PUSH_CHAR('}')
}

void emitFun(Emitter *e, CharList *out, Node n) {
  int sub = 1;
  // Return type
  Node returnNode = n.children.p[n.children.len - 2];
  if (returnNode.kind == N_IDENTIFIER || returnNode.kind == N_COMPLEX_TYPE) {
    emitComplexType(e, out, n.children.p[n.children.len - 2]);
    sub++;
  } else {
    PUSH_CHAR('v')
    PUSH_CHAR('o')
    PUSH_CHAR('i')
    PUSH_CHAR('d')
  }

  // Name
  PUSH_CHAR(' ')
  emitIdentifier(e, out, n.children.p[1]);

  // l brace
  PUSH_CHAR('(')

  // Params
  for (int i = 3; i < n.children.len - sub; ++i) {
    Node node = n.children.p[i];

    if (node.kind == N_COMPLEX_TYPE) {
      PUSH_CHAR(' ')
      emitComplexType(e, out, node);
    } else if (node.kind == N_IDENTIFIER) {
      PUSH_CHAR(' ')
      emitIdentifier(e, out, node);
    } else if (node.kind == N_SEP) {
      PUSH_CHAR(',')
    }
  }

  // r brace
  PUSH_CHAR(')')

  // block
  emitBlock(e, out, n.children.p[n.children.len - 1]);
  PUSH_CHAR('\n')
  PUSH_CHAR('\n')
}

void emitFuns(Emitter *e, CharList *out) {
  for (int i = 0; i < e->inFuns.children.len; i++) {
    emitFun(e, out, e->inFuns.children.p[i]);
  }
}

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
