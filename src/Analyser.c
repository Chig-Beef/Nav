#include "Context.h"
#include "Ident.h"
#include "Node.h"
#include "String.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct Analyser {
  // Source
  Node enums, structs, funcs;

  // Defined variables, types, etc
  Stack vars;
} Analyser;

void throwAnalyserError(Analyser *a, char *fileName, int line, char msg[]) {
  printf("Error in the Hoister!\n"
         "Error found in file: %s\nOn line: %i\n\n%s\n",
         fileName, line, msg);
  exit(1);
}

void analyserInit(Analyser *a, Node enums, Node structs, Node funcs) {
  a->enums = enums;
  a->structs = structs;
  a->funcs = funcs;
  a->vars = (Stack){NULL, 0};

  // Function type,
  stackPush(&a->vars, strNew("fun", false), NULL, TM_NONE);
}

void analyseOperator(Analyser *a, Context c, Node *n);
void analyseIndex(Analyser *a, Context c, Node *n);
void analyseIfBlock(Analyser *a, Context c, Node *n);
void analyseForLoop(Analyser *a, Context c, Node *n);
void analyseRetState(Analyser *a, Context c, Node *n);
void analyseBreakState(Analyser *a, Context c, Node *n);
void analyseContinueState(Analyser *a, Context c, Node *n);
void analyseBracketedValue(Analyser *a, Context c, Node *n);
void analyseStructNew(Analyser *a, Context c, Node *n);
void analyseFuncCall(Analyser *a, Context c, Node *n);
void analyseMakeArray(Analyser *a, Context c, Node *n);
void analyseLoneCall(Analyser *a, Context c, Node *n);
void analyseExpression(Analyser *a, Context c, Node *n);
void analyseCrement(Analyser *a, Context c, Node *n);
void analyseAssignment(Analyser *a, Context c, Node *n);
void analyseNewAssignment(Analyser *a, Context c, Node *n);
void analyseVarDeclaration(Analyser *a, Context c, Node *n);
void analyseUnary(Analyser *a, Context c, Node *n);
void analyseUnaryValue(Analyser *a, Context c, Node *n);
void analyseComplexType(Analyser *a, Context c, Node *n);
void analyseValue(Analyser *a, Context c, Node *n);
void analyseSwitchState(Analyser *a, Context c, Node *n);
void analyseCaseBlock(Analyser *a, Context c, Node *n);
void analyseDefaultBlock(Analyser *a, Context c, Node *n);
void analyseBlock(Analyser *a, Context c, Node *n);

Ident *varExists(Analyser *a, String *name) {
  Ident *curIdent = a->vars.tail;

  while (curIdent != NULL) {
    if (strEql(curIdent->name, name)) {
      return curIdent;
    }

    // Next identifier
    curIdent = curIdent->next;
  }

  return NULL;
}

void analyseEnums(Analyser *a) {
  Node *enumNode, *enumChildNode;
  Ident *enumType;
  String *enumName;

  for (int i = 0; i < a->enums.children.len; ++i) {
    enumNode = a->enums.children.p + i;
    enumName = strGet(enumNode->children.p[1].data);

    if (varExists(a, enumName)) {
      throwAnalyserError(a, NULL, 0, "Enum already exists");
    }

    // Add the actual enum type
    stackPush(&a->vars, enumName, NULL, TM_NONE);
    enumType = a->vars.tail;

    // Each of the constants in the enum
    for (int j = 3; j < enumNode->children.len - 1; j += 2) {
      enumChildNode = enumNode->children.p + j;

      if (varExists(a, enumChildNode->data)) {
        throwAnalyserError(a, NULL, 0, "Enum constant already exists");
      }

      stackPush(&a->vars, enumChildNode->data, enumType, TM_NONE);
    }
  }
}

void analyseStructs(Analyser *a) {
  Node *structNode, *propTypeNode, *propNode;
  Ident *structType;
  int numProps;
  String *structName;

  for (int i = 0; i < a->structs.children.len; ++i) {
    structNode = a->structs.children.p + i;
    structName = strGet(structNode->children.p[1].data);

    if (varExists(a, structName)) {
      throwAnalyserError(a, NULL, 0, "Struct already exists");
    }

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

    structType->props = malloc(sizeof(Ident) * numProps);

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

void analyseFuncs(Analyser *a, Ident *funcType) {
  Node *funcNode, *paramTypeNode, *paramNode;
  Ident *funcDec;
  int numParams;
  String *funcName;

  for (int i = 0; i < a->funcs.children.len; ++i) {
    funcNode = a->structs.children.p + i;
    funcName = strGet(funcNode->children.p[1].data);

    if (varExists(a, funcName)) {
      throwAnalyserError(a, NULL, 0, "Function already exists");
    }

    // Add the function
    stackPush(&a->vars, strGet(funcNode->children.p[1].data), funcType,
              TM_NONE);
    funcDec = a->vars.tail;

    // Each param of this func
    if (funcNode->children.p[funcNode->children.len - 2].kind ==
        N_COMPLEX_TYPE) {
      // We have a return value
      numParams = funcNode->children.len - 5;

      Node *retNode = funcNode->children.p + funcNode->children.len - 2;

      funcDec->ret = malloc(sizeof(Ident));
      *funcDec->ret = (Ident){
          NULL, // No name
          NULL, // TODO: get the type correctly
          NULL, // Not on stack, so no next
          TM_NONE, NULL, NULL, NULL,
      };

    } else {
      numParams = funcNode->children.len - 4;
    }

    numParams /= 3;

    // Empty func
    if (numParams == 0) {
      continue;
    }

    funcDec->params = malloc(sizeof(Ident) * numParams);

    for (int j = 0; j < numParams; ++j) {
      paramTypeNode = funcNode->children.p + 3 + (j * 3);
      paramNode = funcNode->children.p + 4 + (j * 3);

      funcDec->params[j] = (Ident){
          paramNode->data,
          NULL, // TODO: Get the correct type from paramTypeNode

          NULL,    // Next is null, as func params aren't added to the variable
                   // stack (yet)
          TM_NONE, // TODO: Get the correct type modifier from param type node
          NULL,
          NULL,
          NULL,
      };
    }

    int stackBase = a->vars.len;

    analyseBlock(a, (Context){false, NULL},
                 funcNode->children.p + funcNode->children.len - 1);

    // Delete variables used in the function
    while (a->vars.len > stackBase) {
      stackPop(&a->vars);
    }
  }
}

void analyseOperator(Analyser *a, Context c, Node *n) {}
void analyseIndex(Analyser *a, Context c, Node *n) {}
void analyseIfBlock(Analyser *a, Context c, Node *n) {}
void analyseForLoop(Analyser *a, Context c, Node *n) {}
void analyseRetState(Analyser *a, Context c, Node *n) {}
void analyseBreakState(Analyser *a, Context c, Node *n) {}
void analyseContinueState(Analyser *a, Context c, Node *n) {}
void analyseBracketedValue(Analyser *a, Context c, Node *n) {}
void analyseStructNew(Analyser *a, Context c, Node *n) {}
void analyseFuncCall(Analyser *a, Context c, Node *n) {}
void analyseMakeArray(Analyser *a, Context c, Node *n) {}
void analyseLoneCall(Analyser *a, Context c, Node *n) {}
void analyseExpression(Analyser *a, Context c, Node *n) {}
void analyseCrement(Analyser *a, Context c, Node *n) {}
void analyseAssignment(Analyser *a, Context c, Node *n) {}
void analyseNewAssignment(Analyser *a, Context c, Node *n) {}
void analyseVarDeclaration(Analyser *a, Context c, Node *n) {}
void analyseUnary(Analyser *a, Context c, Node *n) {}
void analyseUnaryValue(Analyser *a, Context c, Node *n) {}
void analyseComplexType(Analyser *a, Context c, Node *n) {}
void analyseValue(Analyser *a, Context c, Node *n) {}
void analyseSwitchState(Analyser *a, Context c, Node *n) {}
void analyseCaseBlock(Analyser *a, Context c, Node *n) {}
void analyseDefaultBlock(Analyser *a, Context c, Node *n) {}

void analyseBlock(Analyser *a, Context c, Node *n) {
  for (int i = 1; i < n->children.len - 1; ++i) {
    switch (n->kind) {
    case N_LONE_CALL:
      analyseLoneCall(a, c, n);
    case N_VAR_DEC:
      analyseVarDeclaration(a, c, n);
    case N_IF_BLOCK:
      analyseIfBlock(a, c, n);
    case N_FOR_LOOP:
      analyseForLoop(a, c, n);
    case N_RET_STATE:
      analyseRetState(a, c, n);
    case N_BREAK_STATE:
      analyseBreakState(a, c, n);
    case N_CONTINUE_STATE:
      analyseContinueState(a, c, n);
    case N_SWITCH_STATE:
      analyseSwitchState(a, c, n);

    default:
      throwAnalyserError(a, NULL, 0, "Invalid statement in block");
    }
  }
}

void analyse(Analyser *a) {
  // The type of functions
  Ident *funcType = a->vars.tail;

  analyseEnums(a);
  analyseStructs(a);
  analyseFuncs(a, funcType);

  stackClear(&a->vars);
}
