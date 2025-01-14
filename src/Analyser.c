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
  stackPush(&a->vars, strNew("fun", false), NULL, TM_NONE);
}

void analyseEnums(Analyser *a) {
  Node *enumNode, *enumChildNode;
  Ident *enumType;
  for (int i = 0; i < a->enums.children.len; ++i) {
    enumNode = a->enums.children.p + i;

    // Add the actual enum type
    stackPush(&a->vars, strGet(enumNode->children.p[1].data), NULL, TM_NONE);
    enumType = a->vars.tail;

    // Each of the constants in the enum
    for (int j = 3; j < enumNode->children.len - 1; j += 2) {
      enumChildNode = enumNode->children.p + j;

      stackPush(&a->vars, enumChildNode->data, enumType, TM_NONE);
    }
  }
}

void analyseStructs(Analyser *a) {
  Node *structNode, *propTypeNode, *propNode;
  Ident *structType;
  int numProps;

  for (int i = 0; i < a->structs.children.len; ++i) {
    structNode = a->structs.children.p + i;

    // Add the struct as a type
    stackPush(&a->vars, strGet(structNode->children.p[1].data), NULL, TM_NONE);
    structType = a->vars.tail;

    // Each property of this struct
    numProps = structNode->children.len - 4;

    // Empty struct
    if (numProps == 0 || numProps == 1) {
      continue;
    }

    // (complexType, Identifier, comma)
    numProps = (numProps + 1) / 3;

    structType->params = malloc(sizeof(Ident *) * numProps);

    for (int j = 0; j < numProps; ++j) {
      propTypeNode = structNode->children.p + 3 + (j * 3);
      propNode = structNode->children.p + 4 + (j * 3);

      structType->props[j] = (Ident){
          propNode->data,
          NULL, // TODO: Get the correct type from propTypeNode

          NULL, // Next is null, as struct params aren't added to the variable
                // stack
          TM_NONE, // TODO: Get the correct type modifier from prop type node
          NULL,
          NULL,
          NULL,
      };
    }
  }
}

void analyseFuncs(Analyser *a) {}

void analyse(Analyser *a) {
  analyseEnums(a);
  analyseStructs(a);
  analyseFuncs(a);

  stackClear(&a->vars);
}
