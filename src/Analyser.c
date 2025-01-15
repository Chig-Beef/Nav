#include "Context.h"
#include "Ident.h"
#include "Node.h"
#include "String.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct PreDefs {
  // Primitive types
  Ident *INT;
  Ident *BOOL;
  Ident *CHAR;
  Ident *FLOAT;
  Ident *FUN;

  // Functions
  Ident *PRINT;
} PreDefs;

typedef struct Analyser {
  // Source
  Node enums, structs, funcs;

  // Defined variables, types, etc
  Stack vars;

  PreDefs preDefs;

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

  stackPush(&a->vars, strNew("int", false), NULL, TM_NONE);
  a->preDefs.INT = a->vars.tail;

  stackPush(&a->vars, strNew("bool", false), NULL, TM_NONE);
  a->preDefs.BOOL = a->vars.tail;

  stackPush(&a->vars, strNew("char", false), NULL, TM_NONE);
  a->preDefs.CHAR = a->vars.tail;

  stackPush(&a->vars, strNew("float", false), NULL, TM_NONE);
  a->preDefs.FLOAT = a->vars.tail;

  // Function type
  stackPush(&a->vars, strNew("fun", false), NULL, TM_NONE);
  a->preDefs.FUN = a->vars.tail;

  stackPush(&a->vars, strNew("print", false), a->preDefs.FUN, TM_NONE);
  a->preDefs.PRINT = a->vars.tail;
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

void analyseFuncs(Analyser *a) {
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
    stackPush(&a->vars, strGet(funcNode->children.p[1].data), a->preDefs.FUN,
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

    // TODO: Add in return type correctly
    analyseBlock(a, (Context){false, false, NULL, NULL},
                 funcNode->children.p + funcNode->children.len - 1);

    // Delete variables used in the function
    while (a->vars.len > stackBase) {
      stackPop(&a->vars);
    }
  }
}

void analyseForLoop(Analyser *a, Context c, Node *n) {
  // To get rid of all vars defined in the loop, including the one defined in
  // the loop header
  int stackBase = a->vars.len;

  int i = 1;

  // Assignment
  if (n->children.p[i].kind == N_ASSIGNMENT) {
    analyseAssignment(a, c, n->children.p + i);
    i += 2;
  } else if (n->children.p[i].kind == N_NEW_ASSIGNMENT) {
    analyseNewAssignment(a, c, n->children.p + i);
    i += 2;
  } else {
    ++i;
  }

  // Expression
  if (n->children.p[i].kind == N_EXPRESSION) {
    c.expType = a->preDefs.BOOL;
    analyseExpression(a, c, n->children.p + i);
    i += 2;
  } else {
    ++i;
  }

  // Assignment
  if (n->children.p[i].kind == N_ASSIGNMENT) {
    analyseAssignment(a, c, n->children.p + i);
  }

  analyseBlock(a, (Context){true, true, c.expType, c.retType},
               n->children.p + n->children.len - 1);

  // Delete variables used in the loop
  while (a->vars.len > stackBase) {
    stackPop(&a->vars);
  }
}

void analyseRetState(Analyser *a, Context c, Node *n) {
  // We don't expect a return value
  if (c.retType == NULL && n->children.len == 3) {
    throwAnalyserError(a, NULL, 0,
                       "This function expected no return value, but got one.");
  }

  // Now check the return value, and expect a type
  c.expType = c.retType;
  analyseExpression(a, c, n->children.p + 1);
}

void analyseBreakState(Analyser *a, Context c, Node *n) {
  if (!c.canBreak) {
    throwAnalyserError(a, NULL, 0,
                       "Can't have break statement outside of loop");
  }
}

void analyseContinueState(Analyser *a, Context c, Node *n) {
  if (!c.canCont) {
    throwAnalyserError(a, NULL, 0,
                       "Can't have continue statement outside of loop");
  }
}

void analyseIndex(Analyser *a, Context c, Node *n) {
  c.expType = a->preDefs.INT;
  analyseExpression(a, c, n->children.p + 1);
}

void analyseIfBlock(Analyser *a, Context c, Node *n) {
  int stackBase = a->vars.len;

  c.expType = a->preDefs.BOOL;
  analyseExpression(a, c, n->children.p + 2);
  analyseBlock(a, c, n->children.p + 4);

  int i = 5;

  while (n->children.len > i) {
    if (n->children.p[i].kind == N_ELIF) {
      c.expType = a->preDefs.BOOL;
      analyseExpression(a, c, n->children.p + i + 2);
      analyseBlock(a, c, n->children.p + i + 4);

      i += 5;
    } else if (n->children.p[i].kind == N_ELSE) {

      analyseBlock(a, c, n->children.p + i + 1);

      break;
    }
  }

  // Delete variables used in the if block
  while (a->vars.len > stackBase) {
    stackPop(&a->vars);
  }
}

void analyseCrement(Analyser *a, Context c, Node *n) {
  Ident *var = varExists(a, n->children.p[1].data);
  if (var == NULL) {
    throwAnalyserError(a, NULL, 0, "Variable doesn't exist");
  }

  Ident *type = var->type;
  if (type != a->preDefs.INT) {
    throwAnalyserError(a, NULL, 0,
                       "Can only use -- and ++ operators on integers");
  }
}

void analyseVarDeclaration(Analyser *a, Context c, Node *n) {
  Node *assignment = n->children.p;

  if (assignment->kind == N_ASSIGNMENT) {
    analyseAssignment(a, c, assignment);
  } else if (assignment->kind == N_NEW_ASSIGNMENT) {
    analyseNewAssignment(a, c, assignment);
  }
}

void analyseNewAssignment(Analyser *a, Context c, Node *n) {
  String *varName = n->children.p[2].data;

  if (varExists(a, varName) != NULL) {
    throwAnalyserError(a, NULL, 0, "Variable name already exists");
  }

  // TODO: Get type from complexType

  // TODO: Expect the correct type from expression
  // c.expType = ??

  analyseExpression(a, c, n->children.p + n->children.len - 1);
}

void analyseAssignment(Analyser *a, Context c, Node *n) {
  if (n->children.p[0].kind == N_CREMENT) {
    analyseCrement(a, c, n->children.p);
    return;
  }

  String *varName = n->children.p[0].data;
  Ident *var = varExists(a, varName);
  if (var == NULL) {
    throwAnalyserError(a, NULL, 0, "Variable does not exists");
  }

  if (n->children.p[1].kind == N_INDEX) {
    analyseIndex(a, c, n->children.p + 1);
  }

  c.expType = var->type;
  analyseExpression(a, c, n->children.p + n->children.len - 1);
}

void analyseOperator(Analyser *a, Context c, Node *n) {}
void analyseBracketedValue(Analyser *a, Context c, Node *n) {}
void analyseStructNew(Analyser *a, Context c, Node *n) {}
void analyseFuncCall(Analyser *a, Context c, Node *n) {}
void analyseMakeArray(Analyser *a, Context c, Node *n) {}
void analyseLoneCall(Analyser *a, Context c, Node *n) {}
void analyseExpression(Analyser *a, Context c, Node *n) {}
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
  analyseEnums(a);
  analyseStructs(a);
  analyseFuncs(a);

  stackClear(&a->vars);
}
