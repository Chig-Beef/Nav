#include "Context.h"
#include "Fun.h"
#include "Ident.h"
#include "Node.h"
#include "String.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct PreDefs {
  // Primitive types
  Type *INT, *BOOL, *CHAR, *FLOAT, *FUN;

  // Functions
  Fun *PRINT;
} PreDefs;

typedef struct Analyser {
  // Source
  Node inEnums, inStructs, inFuns;

  // Defined variables, types, etc
  IdentStack vars;
  TypeStack types;
  FunStack funs;

  PreDefs preDefs;

} Analyser;

void throwAnalyserError(Analyser *a, char *fileName, int line, char msg[]) {
  printf("Error in the Hoister!\n"
         "Error found in file: %s\nOn line: %i\n\n%s\n",
         fileName, line, msg);
  exit(1);
}

void analyserInit(Analyser *a, Node enums, Node structs, Node funcs) {
  a->inEnums = enums;
  a->inStructs = structs;
  a->inFuns = funcs;
  a->vars = (IdentStack){NULL, 0};
  a->types = (TypeStack){NULL, 0};
  a->funs = (FunStack){NULL, 0};

  typeStackPush(&a->types, TK_ABS, strNew("int", false), TM_NONE, NULL);
  a->preDefs.INT = a->types.tail;

  typeStackPush(&a->types, TK_ABS, strNew("bool", false), TM_NONE, NULL);
  a->preDefs.BOOL = a->types.tail;

  typeStackPush(&a->types, TK_ABS, strNew("char", false), TM_NONE, NULL);
  a->preDefs.CHAR = a->types.tail;

  typeStackPush(&a->types, TK_ABS, strNew("float", false), TM_NONE, NULL);
  a->preDefs.FLOAT = a->types.tail;

  funStackPush(&a->funs, strNew("print", false));
  a->preDefs.PRINT = a->funs.tail;
}

void analyseIndex(Analyser *a, Context c, Node *n);
void analyseIfBlock(Analyser *a, Context c, Node *n);
void analyseForLoop(Analyser *a, Context c, Node *n);
void analyseRetState(Analyser *a, Context c, Node *n);
void analyseBreakState(Analyser *a, Context c, Node *n);
void analyseContinueState(Analyser *a, Context c, Node *n);
Type *analyseBracketedValue(Analyser *a, Context c, Node *n);
Type *analyseStructNew(Analyser *a, Context c, Node *n);
Type *analyseFuncCall(Analyser *a, Context c, Node *n);
Type *analyseMakeArray(Analyser *a, Context c, Node *n);
void analyseLoneCall(Analyser *a, Context c, Node *n);
Type *analyseExpression(Analyser *a, Context c, Node *n);
void analyseCrement(Analyser *a, Context c, Node *n);
void analyseAssignment(Analyser *a, Context c, Node *n);
void analyseNewAssignment(Analyser *a, Context c, Node *n);
void analyseVarDeclaration(Analyser *a, Context c, Node *n);
Type *analyseUnaryValue(Analyser *a, Context c, Node *n);
void analyseComplexType(Analyser *a, Context c, Node *n);
Type *analyseValue(Analyser *a, Context c, Node *n);
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

Fun *funExists(Analyser *a, String *name) {
  Fun *curFun = a->funs.tail;

  while (curFun != NULL) {
    if (strEql(curFun->name, name)) {
      return curFun;
    }

    // Next identifier
    curFun = curFun->next;
  }

  return NULL;
}

Type *typeExists(Analyser *a, String *name) {
  Type *curType = a->types.tail;

  while (curType != NULL) {
    if (strEql(curType->name, name)) {
      return curType;
    }

    // Next identifier
    curType = curType->next;
  }

  return NULL;
}

void analyseEnums(Analyser *a) {
  Node *enumNode, *enumChildNode;
  Type *enumType;
  String *enumName;

  for (int i = 0; i < a->inEnums.children.len; ++i) {
    enumNode = a->inEnums.children.p + i;
    enumName = strGet(enumNode->children.p[1].data);

    if (typeExists(a, enumName)) {
      throwAnalyserError(a, NULL, 0, "Enum already exists");
    }

    // Add the actual enum type
    typeStackPush(&a->types, TK_ABS, enumName, TM_NONE, NULL);
    enumType = a->types.tail;

    // Each of the constants in the enum
    for (int j = 3; j < enumNode->children.len - 1; j += 2) {
      enumChildNode = enumNode->children.p + j;

      if (varExists(a, enumChildNode->data)) {
        throwAnalyserError(a, NULL, 0, "Enum constant already exists");
      }

      identStackPush(&a->vars, enumChildNode->data, enumType, TM_NONE);
    }
  }
}

void analyseStructs(Analyser *a) {
  Node *structNode, *propTypeNode, *propNode;
  Type *structType;
  int numProps;
  String *structName;

  for (int i = 0; i < a->inStructs.children.len; ++i) {
    structNode = a->inStructs.children.p + i;
    structName = strGet(structNode->children.p[1].data);

    if (typeExists(a, structName)) {
      throwAnalyserError(a, NULL, 0, "Struct already exists");
    }

    // Add the struct as a type
    typeStackPush(&a->types, TK_ABS, strGet(structNode->children.p[1].data),
                  TM_NONE, NULL);
    structType = a->types.tail;

    // Each property of this struct
    numProps = structNode->children.len - 4;

    // Empty struct
    if (numProps == 0 || numProps == 1) {
      continue;
    }

    // (complexType, Identifier, comma)
    numProps = (numProps + 1) / 3;

    structType->props = malloc(sizeof(Ident) * numProps);
    structType->propsLen = numProps;

    for (int j = 0; j < numProps; ++j) {
      propTypeNode = structNode->children.p + 3 + (j * 3);
      propNode = structNode->children.p + 4 + (j * 3);

      structType->props[j] = (Ident){
          propNode->data,
          NULL, // TODO: Get the correct type from propTypeNode

          NULL, // Next is null, as struct params aren't added to the variable
                // stack
          TM_NONE, // TODO: Get the correct type modifier from prop type node
      };
    }
  }
}

void analyseFuncs(Analyser *a) {
  Node *funcNode, *paramTypeNode, *paramNode;
  Fun *funcDec;
  int numParams;
  String *funcName;

  for (int i = 0; i < a->inFuns.children.len; ++i) {
    funcNode = a->inStructs.children.p + i;
    funcName = strGet(funcNode->children.p[1].data);

    if (funExists(a, funcName)) {
      throwAnalyserError(a, NULL, 0, "Function already exists");
    }

    // Add the function
    funStackPush(&a->funs, strGet(funcNode->children.p[1].data));
    funcDec = a->funs.tail;

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
          TM_NONE,
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
    funcDec->paramsLen = numParams;

    for (int j = 0; j < numParams; ++j) {
      paramTypeNode = funcNode->children.p + 3 + (j * 3);
      paramNode = funcNode->children.p + 4 + (j * 3);

      funcDec->params[j] = (Ident){
          paramNode->data,
          NULL, // TODO: Get the correct type from paramTypeNode

          NULL,    // Next is null, as func params aren't added to the variable
                   // stack (yet)
          TM_NONE, // TODO: Get the correct type modifier from param type node
      };
    }

    int stackBase = a->vars.len;

    // TODO: Add in return type correctly
    analyseBlock(a, (Context){false, false, NULL, NULL},
                 funcNode->children.p + funcNode->children.len - 1);

    // Delete variables used in the function
    while (a->vars.len > stackBase) {
      identStackPop(&a->vars);
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
    identStackPop(&a->vars);
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
    identStackPop(&a->vars);
  }
}

void analyseCrement(Analyser *a, Context c, Node *n) {
  Ident *var = varExists(a, n->children.p[1].data);
  if (var == NULL) {
    throwAnalyserError(a, NULL, 0, "Variable doesn't exist");
  }

  Type *type = var->type;
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

Type *analyseBracketedValue(Analyser *a, Context c, Node *n) {
  // Keep same expected type and everything
  return analyseExpression(a, c, n->children.p + 1);
}

void analyseLoneCall(Analyser *a, Context c, Node *n) {
  analyseFuncCall(a, c, n->children.p);
}

void analyseSwitchState(Analyser *a, Context c, Node *n) {
  c.expType = NULL;

  analyseExpression(a, c, n->children.p + 2);

  int i = 5;

  while (n->children.p[i].kind == N_CASE_BLOCK) {
    analyseCaseBlock(a, c, n->children.p + i);
    ++i;
  }

  if (n->children.p[i].kind == N_DEFAULT_BLOCK) {
    analyseDefaultBlock(a, c, n->children.p + i);
  }
}

void analyseCaseBlock(Analyser *a, Context c, Node *n) {
  c.expType = NULL;
  // TODO: Expect the type of the arg to the switch

  analyseExpression(a, c, n->children.p + 1);

  c.canBreak = true;

  for (int i = 3; i < n->children.len; ++i) {
    switch (n->children.p[i].kind) {
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
      throwAnalyserError(a, NULL, 0, "Invalid statement in case block");
    }
  }
}

void analyseDefaultBlock(Analyser *a, Context c, Node *n) {
  c.canBreak = true;

  for (int i = 2; i < n->children.len; ++i) {
    switch (n->children.p[i].kind) {
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
      throwAnalyserError(a, NULL, 0, "Invalid statement in default block");
    }
  }
}

Type *analyseValue(Analyser *a, Context c, Node *n) {
  switch (n->kind) {
  case N_INT:
    return a->preDefs.INT;
  case N_FLOAT:
    return a->preDefs.FLOAT;
  case N_CHAR:
    return a->preDefs.CHAR;
  case N_STRING:
    // TODO: Char array
    return a->preDefs.CHAR;
  case N_TRUE:
    return a->preDefs.BOOL;
  case N_FALSE:
    return a->preDefs.BOOL;
  case N_IDENTIFIER:

    // TODO: Get type of identifier
    return NULL;
  case N_MAKE_ARRAY:
    return analyseMakeArray(a, c, n);
  case N_FUNC_CALL:
    return analyseFuncCall(a, c, n);
  case N_STRUCT_NEW:
    return analyseStructNew(a, c, n);
  case N_BRACKETED_VALUE:
    return analyseBracketedValue(a, c, n);

  default:
    return NULL;
  }
}

Type *analyseUnaryValue(Analyser *a, Context c, Node *n) {
  Type *type;

  if (n->children.p[1].kind == N_UNARY_VALUE) {
    type = analyseUnaryValue(a, c, n->children.p + 1);
  } else {
    type = analyseValue(a, c, n->children.p + 1);
  }

  switch (n->children.p[0].kind) {
  case N_DEREF:
    // TODO: Check for pointer
    return NULL;
  case N_DEC:
    if (type != a->preDefs.INT && type != a->preDefs.CHAR) {
      throwAnalyserError(a, NULL, 0, "Can only decrement ints or chars");
    }
    return type;
  case N_INC:
    if (type != a->preDefs.INT && type != a->preDefs.CHAR) {
      throwAnalyserError(a, NULL, 0, "Can only increment ints or chars");
    }
    return type;
  case N_NOT:
    if (type != a->preDefs.INT && type != a->preDefs.CHAR &&
        type != a->preDefs.BOOL) {
      throwAnalyserError(a, NULL, 0, "Can only not int, char, bool, or float");
    }
    return type;
  case N_REF:
    // TODO: Return pointer to type
    return type;
  case N_ADD:
    if (type != a->preDefs.INT && type != a->preDefs.CHAR) {
      throwAnalyserError(a, NULL, 0, "Can only positive int or char");
    }
    return type;
  case N_SUB:
    if (type != a->preDefs.INT && type != a->preDefs.CHAR) {
      throwAnalyserError(a, NULL, 0, "Can only negative int or char");
    }
    return type;
  default:
    throwAnalyserError(a, NULL, 0, "Unexpected unary");
  }
  return NULL;
}

Type *analyseFuncCall(Analyser *a, Context c, Node *n) {
  Fun *fun = funExists(a, n->children.p[1].data);
  if (fun == NULL) {
    throwAnalyserError(a, NULL, 0, "Function doesn't exist");
  }

  int paramIndex = 0;
  int nodeIndex = 3;

  while (paramIndex < fun->paramsLen) {
    if (nodeIndex >= n->children.len) {
      throwAnalyserError(a, NULL, 0, "Not enough args for function");
    }

    if (n->children.p[nodeIndex].kind != N_EXPRESSION) {
      throwAnalyserError(a, NULL, 0, "Not enough args for function");
    }

    // TODO: Correct expected type
    // c.expType = ???
    analyseExpression(a, c, n->children.p + nodeIndex);

    ++paramIndex;
    nodeIndex += 2;
  }

  if (n->children.p[nodeIndex].kind == N_EXPRESSION) {
    throwAnalyserError(a, NULL, 0, "Too many args for function");
  }

  return NULL;
}

Type *analyseStructNew(Analyser *a, Context c, Node *n) {
  Type *stt = typeExists(a, n->children.p[1].data);
  if (stt == NULL) {
    throwAnalyserError(a, NULL, 0, "Struct doesn't exist");
  }
  // TODO: Make sure thingy is actually a struct, not primitive or enum

  int propIndex = 0;
  int nodeIndex = 3;

  while (propIndex < stt->propsLen) {
    if (nodeIndex >= n->children.len) {
      throwAnalyserError(a, NULL, 0, "Not enough args for struct");
    }

    if (n->children.p[nodeIndex].kind != N_EXPRESSION) {
      throwAnalyserError(a, NULL, 0, "Not enough args for struct");
    }

    // TODO: Correct expected type
    // c.expType = ???
    analyseExpression(a, c, n->children.p + nodeIndex);

    ++propIndex;
    nodeIndex += 2;
  }

  if (n->children.p[nodeIndex].kind == N_EXPRESSION) {
    throwAnalyserError(a, NULL, 0, "Too many args for struct");
  }

  return NULL;
}

Type *analyseMakeArray(Analyser *a, Context c, Node *n) {
  // TODO: Expected type must be array

  // TODO: Unwrap type from array

  int i = 2;

  while (i < n->children.len) {
    if (n->children.p[i].kind != N_EXPRESSION) {
      break;
    }

    analyseExpression(a, c, n->children.p + i);

    i += 2;
  }

  return NULL;
}

// analyseExpression also returns the type of the expression
Type *analyseExpression(Analyser *a, Context c, Node *n) { return NULL; }

void analyseComplexType(Analyser *a, Context c, Node *n) {}

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

  identStackClear(&a->vars);
  funStackClear(&a->funs);
  typeStackClear(&a->types);
}
