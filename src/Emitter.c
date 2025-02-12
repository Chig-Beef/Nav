#include "Emitter.h"
#include "Node.h"
#include "Panic.h"
#include <stdio.h>

#define PUSH_CHAR(character)                                                   \
  if (CharListAppend(out, (character))) {                                      \
    panic("Couldn't append to output");                                        \
  }

void emitterInit(Emitter *e, Node enums, Node structs, Node funs) {
  e->inEnums = enums;
  e->inStructs = structs;
  e->inFuns = funs;
  e->tabs = 0;

  e->nil = strNew("nil", false);
}

void emitTabs(Emitter *e, CharList *out) {
  for (int i = 0; i < e->tabs; ++i) {
    PUSH_CHAR(' ')
    PUSH_CHAR(' ')
  }
}

void emitBlock(Emitter *e, CharList *out, Node n);
void emitExpression(Emitter *e, CharList *out, Node n);
void emitAccess(Emitter *e, CharList *out, Node n);
void emitFuncCall(Emitter *e, CharList *out, Node n);
void emitSwitchState(Emitter *e, CharList *out, Node n);

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
  if (strEql(n.data, e->nil)) {
    PUSH_CHAR('N')
    PUSH_CHAR('U')
    PUSH_CHAR('L')
    PUSH_CHAR('L')
    return;
  }

  char *name = n.data->data;
  int i = 0;
  while (name[i]) {
    PUSH_CHAR(name[i])
    ++i;
  }
}

void emitOperator(Emitter *e, CharList *out, Node n) {
  switch (n.kind) {
  case N_ADD:
    PUSH_CHAR('+')
    break;
  case N_SUB:
    PUSH_CHAR('-')
    break;
  case N_MUL:
    PUSH_CHAR('*')
    break;
  case N_DIV:
    PUSH_CHAR('/')
    break;
  case N_MOD:
    PUSH_CHAR('%')
    break;
  case N_AND:
    PUSH_CHAR('&')
    break;
  case N_OR:
    PUSH_CHAR('|')
    break;
  case N_XOR:
    PUSH_CHAR('^')
    break;
  case N_ANDAND:
    PUSH_CHAR('&')
    PUSH_CHAR('&')
    break;
  case N_OROR:
    PUSH_CHAR('|')
    PUSH_CHAR('|')
    break;
  case N_EQ:
    PUSH_CHAR('=')
    PUSH_CHAR('=')
    break;
  case N_NEQ:
    PUSH_CHAR('!')
    PUSH_CHAR('=')
    break;
  case N_GT:
    PUSH_CHAR('>')
    break;
  case N_GTEQ:
    PUSH_CHAR('>')
    PUSH_CHAR('=')
    break;
  case N_LT:
    PUSH_CHAR('<')
    break;
  case N_LTEQ:
    PUSH_CHAR('<')
    PUSH_CHAR('=')
    break;
  case N_L_SHIFT:
    PUSH_CHAR('<')
    PUSH_CHAR('<')
    break;
  case N_R_SHIFT:
    PUSH_CHAR('>')
    PUSH_CHAR('>')
    break;
  default:
    printf("%s\n", nodeCodeString(n.kind));
    panic("Invalid operator");
  }
}

void emitBracketedValue(Emitter *e, CharList *out, Node n) {
  PUSH_CHAR('(')
  emitExpression(e, out, n.children.p[1]);
  PUSH_CHAR(')')
}

void emitMakeArray(Emitter *e, CharList *out, Node n) {
  PUSH_CHAR('{')

  for (int i = 2; i < n.children.len - 1; ++i) {
    Node node = n.children.p[i];
    if (node.kind == N_EXPRESSION) {
      emitExpression(e, out, node);
    } else { // Sep
      PUSH_CHAR(',')
      PUSH_CHAR(' ')
    }
  }

  PUSH_CHAR('}')
}

void emitStructNew(Emitter *e, CharList *out, Node n) {
  PUSH_CHAR('(')
  emitIdentifier(e, out, n.children.p[1]);
  PUSH_CHAR(')')
  PUSH_CHAR('{')

  for (int i = 3; i < n.children.len - 1; ++i) {
    Node node = n.children.p[i];
    if (node.kind == N_EXPRESSION) {
      emitExpression(e, out, node);
    } else { // Sep
      PUSH_CHAR(',')
      PUSH_CHAR(' ')
    }
  }

  PUSH_CHAR('}')
}

void emitValue(Emitter *e, CharList *out, Node n) {
  // NOTE: emitIdentifier works on anything that just emits its data, such as
  // numbers

  switch (n.kind) {
  case N_FLOAT:
    emitIdentifier(e, out, n);
    break;
  case N_INT:
    emitIdentifier(e, out, n);
    break;
  case N_CHAR:
    emitIdentifier(e, out, n);
    break;
  case N_STRING:
    emitIdentifier(e, out, n);
    break;
  case N_BRACKETED_VALUE:
    emitBracketedValue(e, out, n);
    break;
  case N_MAKE_ARRAY:
    emitMakeArray(e, out, n);
    break;
  case N_FUNC_CALL:
    emitFuncCall(e, out, n);
    break;
  case N_STRUCT_NEW:
    emitStructNew(e, out, n);
    break;
  case N_IDENTIFIER:
    emitIdentifier(e, out, n);
    break;
  case N_ACCESS:
    emitAccess(e, out, n);
    break;
  case N_TRUE:
    PUSH_CHAR('t');
    PUSH_CHAR('r');
    PUSH_CHAR('u');
    PUSH_CHAR('e');
    break;
  case N_FALSE:
    PUSH_CHAR('f');
    PUSH_CHAR('a');
    PUSH_CHAR('l');
    PUSH_CHAR('s');
    PUSH_CHAR('e');
    break;
  default:
    printf("%s\n", nodeCodeString(n.kind));
    panic("Invalid value");
  }
}

void emitUnary(Emitter *e, CharList *out, Node n) {
  switch (n.kind) {
  case N_DEREF:
    PUSH_CHAR('^')
    break;
  case N_DEC:
    PUSH_CHAR('-')
    PUSH_CHAR('-')
    break;
  case N_INC:
    PUSH_CHAR('+')
    PUSH_CHAR('+')
    break;
  case N_NOT:
    PUSH_CHAR('!')
    break;
  case N_REF:
    PUSH_CHAR('&')
    break;
  case N_ADD:
    PUSH_CHAR('+')
    break;
  case N_SUB:
    PUSH_CHAR('-')
    break;
  default:
    printf("%s\n", nodeCodeString(n.kind));
    panic("Invalid operator");
  }
}

void emitUnaryValue(Emitter *e, CharList *out, Node n) {
  emitUnary(e, out, n.children.p[0]);

  Node node = n.children.p[1];
  if (node.kind == N_EXPRESSION) {
    emitExpression(e, out, node);
  } else { // Unary value
    emitUnaryValue(e, out, node);
  }
}

void emitExpression(Emitter *e, CharList *out, Node n) {

  Node node = n.children.p[0];

  if (node.kind == N_UNARY_VALUE) {
    emitUnaryValue(e, out, node);
  } else {
    emitValue(e, out, node);
  }

  for (int i = 1; i < n.children.len - 1; i += 2) {
    node = n.children.p[i];
    emitOperator(e, out, node);

    node = n.children.p[i + 1];
    if (node.kind == N_UNARY_VALUE) {
      emitUnaryValue(e, out, node);
    } else {
      emitValue(e, out, node);
    }
  }
}

void emitIndex(Emitter *e, CharList *out, Node n) {
  PUSH_CHAR('[')
  emitExpression(e, out, n.children.p[1]);
  PUSH_CHAR(']')
}

void emitComplexType(Emitter *e, CharList *out, Node n) {
  // printf("CompType: %s\n", nodeCodeString(n.kind));
  // printf("CompType: %i\n", n.line);

  if (n.kind == N_IDENTIFIER) {
    emitIdentifier(e, out, n);
    return;
  }

  if (n.children.p[0].kind == N_INDEX) {
    emitComplexType(e, out, n.children.p[1]);
    emitIndex(e, out, n.children.p[0]);
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

  emitIdentifier(e, out, n.children.p[1]);
  PUSH_CHAR(' ')
  emitIdentifier(e, out, n.children.p[1]);

  PUSH_CHAR(';')
  PUSH_CHAR('\n')

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

  // typedef struct Point { int x; int y; } Point;
  PUSH_CHAR('\n')
  PUSH_CHAR('}')

  PUSH_CHAR(' ')
  i = 0;
  while (structName[i]) {
    PUSH_CHAR(structName[i])
    ++i;
  }
  PUSH_CHAR(';')

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
  emitIdentifier(e, out, n.children.p[0]);

  Node node = n.children.p[1];

  if (node.kind == N_ACCESSOR) {
    PUSH_CHAR('.')
  } else { // P accessor
    PUSH_CHAR('-')
    PUSH_CHAR('>')
  }

  node = n.children.p[2];

  if (node.kind == N_ACCESS) {
    emitAccess(e, out, node);
  } else {
    emitIdentifier(e, out, node);
  }
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
  PUSH_CHAR('i')
  PUSH_CHAR('f')
  PUSH_CHAR(' ')
  PUSH_CHAR('(')

  emitExpression(e, out, n.children.p[2]);

  PUSH_CHAR(')')
  PUSH_CHAR(' ')

  emitBlock(e, out, n.children.p[4]);

  int i = 5;

  if (i == n.children.len) {
    PUSH_CHAR('\n')
    PUSH_CHAR('\n')
    return;
  }

  while (n.children.p[i].kind == N_ELIF) {
    PUSH_CHAR(' ')
    PUSH_CHAR('e')
    PUSH_CHAR('l')
    PUSH_CHAR('s')
    PUSH_CHAR('e')
    PUSH_CHAR(' ')
    PUSH_CHAR('i')
    PUSH_CHAR('f')
    PUSH_CHAR(' ')
    PUSH_CHAR('(')

    emitExpression(e, out, n.children.p[i + 2]);

    PUSH_CHAR(')')
    PUSH_CHAR(' ')

    emitBlock(e, out, n.children.p[i + 4]);

    i += 5;

    if (i == n.children.len) {
      PUSH_CHAR('\n')
      PUSH_CHAR('\n')
      return;
    }
  }

  if (n.children.p[i].kind == N_ELSE) {
    PUSH_CHAR(' ')
    PUSH_CHAR('e')
    PUSH_CHAR('l')
    PUSH_CHAR('s')
    PUSH_CHAR('e')
    PUSH_CHAR(' ')

    emitBlock(e, out, n.children.p[i + 1]);
  }

  PUSH_CHAR('\n')
  PUSH_CHAR('\n')
}

void emitForLoop(Emitter *e, CharList *out, Node n) {

  PUSH_CHAR('f')
  PUSH_CHAR('o')
  PUSH_CHAR('r')
  PUSH_CHAR(' ')
  PUSH_CHAR('(')
  PUSH_CHAR(' ')

  for (int i = 1; i < n.children.len - 1; ++i) {
    Node node = n.children.p[i];

    if (node.kind == N_ASSIGNMENT) {
      emitAssignment(e, out, node);
    } else if (node.kind == N_NEW_ASSIGNMENT) {
      emitNewAssignment(e, out, node);
    } else if (node.kind == N_EXPRESSION) {
      emitExpression(e, out, node);
    } else if (node.kind == N_SEMICOLON) {
      PUSH_CHAR(';')
      PUSH_CHAR(' ')
    }
  }

  PUSH_CHAR(')')
  PUSH_CHAR(' ')
  emitBlock(e, out, n.children.p[n.children.len - 1]);
  PUSH_CHAR('\n')
  PUSH_CHAR('\n')
}

void emitRetState(Emitter *e, CharList *out, Node n) {
  PUSH_CHAR('r')
  PUSH_CHAR('e')
  PUSH_CHAR('t')
  PUSH_CHAR('u')
  PUSH_CHAR('r')
  PUSH_CHAR('n')
  PUSH_CHAR(' ')

  if (n.children.p[1].kind == N_EXPRESSION) {
    emitExpression(e, out, n.children.p[1]);
  }

  PUSH_CHAR(';')
  PUSH_CHAR('\n')
}

void emitBreakState(Emitter *e, CharList *out, Node n) {
  PUSH_CHAR('b')
  PUSH_CHAR('r')
  PUSH_CHAR('e')
  PUSH_CHAR('a')
  PUSH_CHAR('k')
  PUSH_CHAR(';')
  PUSH_CHAR('\n')
}

void emitContinueState(Emitter *e, CharList *out, Node n) {
  PUSH_CHAR('c')
  PUSH_CHAR('o')
  PUSH_CHAR('n')
  PUSH_CHAR('t')
  PUSH_CHAR('i')
  PUSH_CHAR('n')
  PUSH_CHAR('u')
  PUSH_CHAR('e')
  PUSH_CHAR(';')
  PUSH_CHAR('\n')
}

void emitCaseBlock(Emitter *e, CharList *out, Node n) {
  PUSH_CHAR('c')
  PUSH_CHAR('a')
  PUSH_CHAR('s')
  PUSH_CHAR('e')
  PUSH_CHAR(' ')

  emitExpression(e, out, n.children.p[1]);

  PUSH_CHAR(':')
  PUSH_CHAR('\n')

  ++e->tabs;

  for (int i = 3; i < n.children.len; ++i) {
    Node node = n.children.p[i];

    emitTabs(e, out);

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
      panic("Invalid statement in case block");
      break;
    }
  }

  --e->tabs;

  PUSH_CHAR('\n')
}

void emitDefaultBlock(Emitter *e, CharList *out, Node n) {
  PUSH_CHAR('d')
  PUSH_CHAR('e')
  PUSH_CHAR('f')
  PUSH_CHAR('a')
  PUSH_CHAR('u')
  PUSH_CHAR('l')
  PUSH_CHAR('t')
  PUSH_CHAR(' ')
  PUSH_CHAR(':')
  PUSH_CHAR('\n')

  ++e->tabs;

  for (int i = 2; i < n.children.len; ++i) {
    Node node = n.children.p[i];

    emitTabs(e, out);

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
      panic("Invalid statement in default block");
      break;
    }
  }

  --e->tabs;

  PUSH_CHAR('\n')
}

void emitSwitchState(Emitter *e, CharList *out, Node n) {
  PUSH_CHAR('s')
  PUSH_CHAR('w')
  PUSH_CHAR('i')
  PUSH_CHAR('t')
  PUSH_CHAR('c')
  PUSH_CHAR('h')
  PUSH_CHAR(' ')
  PUSH_CHAR('(')

  emitExpression(e, out, n.children.p[2]);

  PUSH_CHAR(')')

  PUSH_CHAR('{')
  PUSH_CHAR('\n')

  for (int i = 5; i < n.children.len - 1; ++i) {
    Node node = n.children.p[i];
    if (node.kind == N_CASE_BLOCK) {
      emitTabs(e, out);
      emitCaseBlock(e, out, node);
    } else if (node.kind == N_DEFAULT_BLOCK) {
      emitTabs(e, out);
      emitDefaultBlock(e, out, node);
    }
  }

  emitTabs(e, out);
  PUSH_CHAR('}')
  PUSH_CHAR('\n')
  PUSH_CHAR('\n')
}

void emitBlock(Emitter *e, CharList *out, Node n) {

  // char *p = nodeString(&n);
  // printf("%s\n", p);
  // free(p);

  PUSH_CHAR('{')
  PUSH_CHAR('\n')

  ++e->tabs;

  for (int i = 1; i < n.children.len - 1; ++i) {
    Node node = n.children.p[i];

    emitTabs(e, out);

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
      break;
    }
  }

  --e->tabs;

  emitTabs(e, out);

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
  PUSH_CHAR(' ')

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

void emitHeaders(Emitter *e, CharList *out){
    PUSH_CHAR('#') PUSH_CHAR('i') PUSH_CHAR('n') PUSH_CHAR('c') PUSH_CHAR('l')
        PUSH_CHAR('u') PUSH_CHAR('d') PUSH_CHAR('e') PUSH_CHAR(' ')
            PUSH_CHAR('<') PUSH_CHAR('s') PUSH_CHAR('t') PUSH_CHAR('r')
                PUSH_CHAR('i') PUSH_CHAR('n') PUSH_CHAR('g') PUSH_CHAR('.')
                    PUSH_CHAR('h') PUSH_CHAR('>') PUSH_CHAR('\n')
                        PUSH_CHAR('\n')}

CharList emit(Emitter *e) {
  CharList out;

  if (CharListInit(&out, 1)) {
    panic("Couldn't initialise output list");
  }

  emitHeaders(e, &out);

  emitEnums(e, &out);
  emitStructs(e, &out);
  emitFuns(e, &out);

  return out;
}
