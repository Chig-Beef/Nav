#include "Ident.h"
#include "Node.h"
#include "String.h"

typedef struct Analyser {
  // Source
  Node enums, structs, funcs;

  // Defined variables, types, etc
  Stack vars;
} Analyser;

void analyserInit(Analyser *a, Node enums, Node structs, Node funcs) {
  a->enums = enums;
  a->structs = structs;
  a->funcs = funcs;
  a->vars = (Stack){NULL, 0};

  // Function type,
  stackPush(&a->vars, strNew("fun", false), NULL, TM_NONE, NULL, NULL);
}

void analyseEnums(Analyser *a) {
  Node *enumNode, *enumChildNode;
  Ident *enumType;
  for (int i = 0; i < a->enums.children.len; ++i) {
    enumNode = a->enums.children.p + i;

    // Add the actual enum type
    stackPush(&a->vars, strGet(enumNode->children.p[1].data), NULL, TM_NONE,
              NULL, NULL);

    enumType = a->vars.tail;

    // Each of the constants in the enum
    for (int j = 3; j < enumNode->children.len - 1; j += 2) {
      enumChildNode = enumNode->children.p + j;

      stackPush(&a->vars, enumChildNode->data, enumType, TM_NONE, NULL, NULL);
    }
  }
}

void analyseStructs(Analyser *a) {}

void analyseFuncs(Analyser *a) {}

void analyse(Analyser *a) {
  analyseEnums(a);
  analyseStructs(a);
  analyseFuncs(a);
}
