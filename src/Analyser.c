#include "Context.h"
#include "Fun.h"
#include "Ident.h"
#include "Node.h"
#include "String.h"
#include "TypeModifier.h"
#include "Types.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct PreDefs {
  // Primitive types
  Type *INT, *BOOL, *CHAR, *FLOAT, *FUN, *VOIDPTR, *STRING;

  // Functions
  Fun *PRINT;

  // Vars
  Ident *NIL;
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

void throwAnalyserError(Analyser *a, char *fileName, int line, char func[],
                        char msg[]) {
  printf("Error in the Analyser!\nError found in file: %s\nOn line: %i\nIn "
         "function: %s\n\n%s\n",
         fileName, line, func, msg);
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

  typeStackPush(&a->types, TK_COMP, NULL, TM_POINTER, NULL);
  a->preDefs.VOIDPTR = a->types.tail;

  typeStackPush(&a->types, TK_COMP, NULL, TM_POINTER, a->preDefs.CHAR);
  a->preDefs.STRING = a->types.tail;

  funStackPush(&a->funs, strNew("print", false));
  a->preDefs.PRINT = a->funs.tail;

  identStackPush(&a->vars, strNew("nil", false), a->preDefs.VOIDPTR);
  a->preDefs.NIL = a->vars.tail;
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
Type *analyseComplexType(Analyser *a, Context c, Node *n);
Type *analyseValue(Analyser *a, Context c, Node *n);
Type *analyseAccess(Analyser *a, Context c, Node *n);
void analyseSwitchState(Analyser *a, Context c, Node *n);
void analyseCaseBlock(Analyser *a, Context c, Node *n);
void analyseDefaultBlock(Analyser *a, Context c, Node *n);
void analyseBlock(Analyser *a, Context c, Node *n);
void analyseOperator(Analyser *a, Context c, Node *n, Type *left, Type *right);

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
  char FUNC_NAME[] = "analyseEnums";

  Node *enumNode, *enumChildNode;
  Type *enumType;
  String *enumName;

  for (int i = 0; i < a->inEnums.children.len; ++i) {
    enumNode = a->inEnums.children.p + i;
    enumName = strGet(enumNode->children.p[1].data);

    printf("Analysing enum %s\n", enumName->data);

    if (typeExists(a, enumName)) {
      throwAnalyserError(a, enumNode->sourceName, enumNode->line, FUNC_NAME,
                         "Enum already exists");
    }

    // Add the actual enum type
    typeStackPush(&a->types, TK_ABS, enumName, TM_NONE, NULL);
    enumType = a->types.tail;

    // Each of the constants in the enum
    for (int j = 3; j < enumNode->children.len - 1; j += 2) {
      enumChildNode = enumNode->children.p + j;

      Ident *exists = varExists(a, enumChildNode->data);

      if (exists != NULL) {
        throwAnalyserError(a, enumChildNode->sourceName, enumChildNode->line,
                           FUNC_NAME, "Enum constant already exists");
      }

      identStackPush(&a->vars, enumChildNode->data, enumType);
    }
  }
}

void analyseStructs(Analyser *a) {
  char FUNC_NAME[] = "analyseStructs";

  Node *structNode, *propTypeNode, *propNode;
  Type *structType;
  int numProps;
  String *structName;

  for (int i = 0; i < a->inStructs.children.len; ++i) {
    structNode = a->inStructs.children.p + i;
    structName = strGet(structNode->children.p[1].data);

    if (typeExists(a, structName)) {
      throwAnalyserError(a, structNode->sourceName, structNode->line, FUNC_NAME,
                         "Struct already exists");
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
          propNode->data, analyseComplexType(a, ZERO_CONTEXT, propTypeNode),

          NULL, // Next is null, as struct params aren't added to the variable
                // stack
      };
    }
  }
}

void analyseFuncs(Analyser *a) {
  char FUNC_NAME[] = "analyseFuncs";

  Node *funcNode, *paramTypeNode, *paramNode;
  Fun *funcDec;
  int numParams;
  String *funcName;

  for (int i = 0; i < a->inFuns.children.len; ++i) {

    funcNode = a->inFuns.children.p + i;
    funcName = strGet(funcNode->children.p[1].data);

    printf("Analysing function %s\n", funcName->data);

    if (funExists(a, funcName)) {
      throwAnalyserError(a, funcNode->sourceName, funcNode->line, FUNC_NAME,
                         "Function already exists");
    }

    // Add the function
    funStackPush(&a->funs, strGet(funcNode->children.p[1].data));
    funcDec = a->funs.tail;

    // Each param of this func
    if (funcNode->children.p[funcNode->children.len - 2].kind != N_R_PAREN) {
      // We have a return value
      numParams = funcNode->children.len - 5;

      Node *retNode = funcNode->children.p + funcNode->children.len - 2;

      funcDec->ret = analyseComplexType(a, ZERO_CONTEXT, retNode);

    } else {
      numParams = funcNode->children.len - 4;
    }

    numParams /= 3;

    // printf("Got function return type\n");

    if (numParams != 0) {
      funcDec->params = malloc(sizeof(Ident) * numParams);
      funcDec->paramsLen = numParams;

      for (int j = 0; j < numParams; ++j) {
        paramTypeNode = funcNode->children.p + 3 + (j * 3);
        paramNode = funcNode->children.p + 4 + (j * 3);

        funcDec->params[j] = (Ident){
            paramNode->data, analyseComplexType(a, ZERO_CONTEXT, paramTypeNode),

            NULL, // Next is null, as func params aren't added to the variable
                  // stack (yet)
        };
      }
    }

    // printf("Got function params\n");

    // Add params to the stack
    for (int i = 0; i < funcDec->paramsLen; ++i) {
      ++a->vars.len;

      if (a->vars.len == 1) {
        a->vars.tail = funcDec->params + i;
        return;
      }

      funcDec->params[i].next = a->vars.tail;
      a->vars.tail = funcDec->params + i;
    }

    printf("Added params to stack\n");

    int stackBase = a->vars.len;

    analyseBlock(a, (Context){false, false, NULL, funcDec->ret},
                 funcNode->children.p + funcNode->children.len - 1);

    // Delete variables used in the function
    while (a->vars.len > stackBase) {
      identStackPop(&a->vars);
    }

    // Remove all params
    for (int i = 0; i < funcDec->paramsLen; ++i) {
      --a->vars.len;
      a->vars.tail = a->vars.tail->next;
    }
  }
}

bool typeEqual(Type *t1, Type *t2) {
  if (t1 == t2) {
    return true;
  }

  // Void type
  if (t1 == NULL || t2 == NULL) {
    return true;
  }

  // Are they both primitive/struct
  if (t1->propsLen != t2->propsLen) {
    return false;
  }

  if (t1->mod != t2->mod) {
    return false;
  }

  if (!strEql(t1->name, t2->name)) {
    return false;
  }

  return typeEqual(t1->parent, t2->parent);
}

void analyseForLoop(Analyser *a, Context c, Node *n) {
  // printf("ForLoop %s\n", nodeCodeString(n->kind));
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
  char FUNC_NAME[] = "analyseRetState";

  // printf("RetState %s\n", nodeCodeString(n->kind));

  // We don't expect a return value
  if (c.retType == NULL && n->children.len == 3) {
    throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                       "This function expected no return value, but got one.");
  }

  // Now check the return value, and expect a type
  c.expType = c.retType;
  analyseExpression(a, c, n->children.p + 1);
}

void analyseBreakState(Analyser *a, Context c, Node *n) {
  char FUNC_NAME[] = "analyseBreakState";

  // printf("BreakState %s\n", nodeCodeString(n->kind));

  if (!c.canBreak) {
    throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                       "Can't have break statement outside of loop");
  }
}

void analyseContinueState(Analyser *a, Context c, Node *n) {
  char FUNC_NAME[] = "analyseContinue";

  // printf("ContinueState %s\n", nodeCodeString(n->kind));

  if (!c.canCont) {
    throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                       "Can't have continue statement outside of loop");
  }
}

void analyseIndex(Analyser *a, Context c, Node *n) {
  // printf("Index %s\n", nodeCodeString(n->kind));

  c.expType = a->preDefs.INT;
  analyseExpression(a, c, n->children.p + 1);
}

void analyseIfBlock(Analyser *a, Context c, Node *n) {
  // printf("IfBlock %s\n", nodeCodeString(n->kind));

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
  char FUNC_NAME[] = "analyseCrement";

  // printf("Crement %s\n", nodeCodeString(n->kind));

  Type *type = NULL;

  if (n->children.p[1].kind == N_IDENTIFIER) {
    Ident *var = varExists(a, n->children.p[1].data);
    if (var == NULL) {
      printf("\nVariable name: %s\n\n", n->children.p[1].data->data);
      throwAnalyserError(a, n->sourceName, n->children.p[1].line, FUNC_NAME,
                         "Variable doesn't exist");
    }
    type = var->type;
  } else { // Access
    type = analyseAccess(a, c, n->children.p + 1);
  }

  if (type != a->preDefs.INT) {
    throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                       "Can only use -- and ++ operators on integers");
  }
}

void analyseVarDeclaration(Analyser *a, Context c, Node *n) {
  // printf("VarDec %s\n", nodeCodeString(n->kind));

  Node *assignment = n->children.p;

  if (assignment->kind == N_ASSIGNMENT) {
    analyseAssignment(a, c, assignment);
  } else if (assignment->kind == N_NEW_ASSIGNMENT) {
    analyseNewAssignment(a, c, assignment);
  }
}

void analyseNewAssignment(Analyser *a, Context c, Node *n) {
  char FUNC_NAME[] = "analyseNewAssignment";

  // printf("NewAssign %s\n", nodeCodeString(n->kind));

  String *varName = n->children.p[2].data;

  if (varExists(a, varName) != NULL) {
    throwAnalyserError(a, n->sourceName, n->children.p[2].line, FUNC_NAME,
                       "Variable name already exists");
  }

  Type *t = analyseComplexType(a, c, n->children.p + 1);

  // Expect the correct type from expression
  c.expType = t;

  // printf("Got new var type\n");

  Type *exprType = analyseExpression(a, c, n->children.p + n->children.len - 1);
  if (exprType != t) {
    printf("\nExpected type %s\n", typeString(t));
    printf("Recieved type %s\n\n", typeString(exprType));
    throwAnalyserError(a, n->sourceName,
                       n->children.p[n->children.len - 1].line, FUNC_NAME,
                       "Expression in assignment wasn't correct type");
  }

  identStackPush(&a->vars, strGet(n->children.p[2].data), t);

  // printf("End NewAssign %s\n", nodeCodeString(n->kind));
}

void analyseAssignment(Analyser *a, Context c, Node *n) {
  char FUNC_NAME[] = "analyseAssignment";

  // printf("Assign %s\n", nodeCodeString(n->kind));

  if (n->children.p[0].kind == N_CREMENT) {
    analyseCrement(a, c, n->children.p);
    return;
  }

  Type *varType = NULL;

  if (n->children.p[0].kind == N_IDENTIFIER) {
    String *varName = n->children.p[0].data;
    Ident *var = varExists(a, varName);
    if (var == NULL) {
      throwAnalyserError(a, n->sourceName, n->children.p[0].line, FUNC_NAME,
                         "Variable does not exists");
    }
    varType = var->type;
  } else { // Access
    varType = analyseAccess(a, c, n->children.p);
  }

  if (n->children.p[1].kind == N_INDEX) {
    if (varType->mod != TM_ARRAY) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't index non-array");
    }
    analyseIndex(a, c, n->children.p + 1);
  }

  c.expType = varType;
  analyseExpression(a, c, n->children.p + n->children.len - 1);
}

Type *analyseBracketedValue(Analyser *a, Context c, Node *n) {
  // printf("BracketedValue %s\n", nodeCodeString(n->kind));

  // Keep same expected type and everything
  return analyseExpression(a, c, n->children.p + 1);
}

void analyseLoneCall(Analyser *a, Context c, Node *n) {
  // printf("LoneCall %s\n", nodeCodeString(n->kind));

  analyseFuncCall(a, c, n->children.p);
}

void analyseSwitchState(Analyser *a, Context c, Node *n) {
  // printf("SwitchState %s\n", nodeCodeString(n->kind));

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
  char FUNC_NAME[] = "analyseCaseBlock";

  // printf("CaseBlock %s\n", nodeCodeString(n->kind));

  // Expect the type of the arg to the switch
  c.expType = analyseExpression(a, c, n->children.p + 1);

  c.canBreak = true;

  for (int i = 3; i < n->children.len; ++i) {
    switch (n->children.p[i].kind) {
    case N_LONE_CALL:
      analyseLoneCall(a, c, n);
      break;
    case N_VAR_DEC:
      analyseVarDeclaration(a, c, n);
      break;
    case N_IF_BLOCK:
      analyseIfBlock(a, c, n);
      break;
    case N_FOR_LOOP:
      analyseForLoop(a, c, n);
      break;
    case N_RET_STATE:
      analyseRetState(a, c, n);
      break;
    case N_BREAK_STATE:
      analyseBreakState(a, c, n);
      break;
    case N_CONTINUE_STATE:
      analyseContinueState(a, c, n);
      break;
    case N_SWITCH_STATE:
      analyseSwitchState(a, c, n);
      break;

    default:
      throwAnalyserError(a, n->sourceName, n->children.p[i].line, FUNC_NAME,
                         "Invalid statement in case block");
    }
  }
}

void analyseDefaultBlock(Analyser *a, Context c, Node *n) {
  char FUNC_NAME[] = "analyseDefaultBlock";

  // printf("DefaultBlock %s\n", nodeCodeString(n->kind));

  c.canBreak = true;

  for (int i = 2; i < n->children.len; ++i) {
    switch (n->children.p[i].kind) {
    case N_LONE_CALL:
      analyseLoneCall(a, c, n);
      break;
    case N_VAR_DEC:
      analyseVarDeclaration(a, c, n);
      break;
    case N_IF_BLOCK:
      analyseIfBlock(a, c, n);
      break;
    case N_FOR_LOOP:
      analyseForLoop(a, c, n);
      break;
    case N_RET_STATE:
      analyseRetState(a, c, n);
      break;
    case N_BREAK_STATE:
      analyseBreakState(a, c, n);
      break;
    case N_CONTINUE_STATE:
      analyseContinueState(a, c, n);
      break;
    case N_SWITCH_STATE:
      analyseSwitchState(a, c, n);
      break;

    default:
      throwAnalyserError(a, n->sourceName, n->children.p[i].line, FUNC_NAME,
                         "Invalid statement in default block");
    }
  }
}

Type *analyseValue(Analyser *a, Context c, Node *n) {
  char FUNC_NAME[] = "analyseValue";

  // printf("Value %s\n", nodeCodeString(n->kind));

  switch (n->kind) {
  case N_INT:
    return a->preDefs.INT;
  case N_FLOAT:
    return a->preDefs.FLOAT;
  case N_CHAR:
    return a->preDefs.CHAR;
  case N_STRING:
    return a->preDefs.STRING;
  case N_TRUE:
    return a->preDefs.BOOL;
  case N_FALSE:
    return a->preDefs.BOOL;
  case N_IDENTIFIER:
    Ident *v = varExists(a, n->data);
    if (v == NULL) {
      printf("%s\n", n->data->data);
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "That variable doesn't exist");
    }

    return v->type;
  case N_MAKE_ARRAY:
    return analyseMakeArray(a, c, n);
  case N_FUNC_CALL:
    return analyseFuncCall(a, c, n);
  case N_STRUCT_NEW:
    return analyseStructNew(a, c, n);
  case N_BRACKETED_VALUE:
    return analyseBracketedValue(a, c, n);
  case N_ACCESS:
    return analyseAccess(a, c, n);

  default:
    printf("%s\n", nodeCodeString(n->kind));
    throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME, "Invalid value?");
    return NULL;
  }
}

Type *analyseAccess(Analyser *a, Context c, Node *n) {
  char FUNC_NAME[] = "analyseAccess";

  String *propName;

  Node *parentNode = n;
  Ident *parent = varExists(a, n->children.p[0].data);
  Type *parentType = parent->type;
  if (n->children.p[1].kind == N_P_ACCESSOR) {
    if (parentType->mod != TM_POINTER) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Attempted to use pointer access on non-pointer type");
    }
    parentType = parentType->parent;
  }

  while (parentNode->children.p[2].kind == N_ACCESS) {
    propName = parentNode->children.p[2].children.p[0].data;

    Ident *child = NULL;

    // Search through the properties of the parent for a match
    for (int i = 0; i < parentType->propsLen; ++i) {
      if (strEql(parentType->props[i].name, propName)) {
        child = parentType->props + i;
      }
    }

    if (child == NULL) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Tried to acces a non-existant property");
    }

    // Next recursion
    parentNode = parentNode->children.p + 2;
    parentType = child->type;

  if (parentNode->children.p[1].kind == N_P_ACCESSOR) {
    if (parentType->mod != TM_POINTER) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Attempted to use pointer access on non-pointer type");
    }
    parentType = parentType->parent;
  }
  }

  if (parentNode->children.p[2].kind != N_IDENTIFIER) {
    throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                       "Expected an access chain to end in an identifier");
  }

  propName = parentNode->children.p[2].data;

  // Search through the properties of the parent for a match
  for (int i = 0; i < parentType->propsLen; ++i) {
    if (strEql(parentType->props[i].name, propName)) {
      return parentType->props[i].type;
    }
  }

  // We didn't find a matching prop
  throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                     "Tried to acces a non-existant property");

  return NULL;
}

Type *analyseUnaryValue(Analyser *a, Context c, Node *n) {
  char FUNC_NAME[] = "analyseUnaryValue";

  // printf("UnaryValue %s\n", nodeCodeString(n->kind));

  Type *type;

  // Get the new expected type for the value inside
  Type *recType = c.expType;

  switch (n->children.p[0].kind) {
  case N_DEREF:
    Type *curType = a->types.tail;

    bool found = false;

    // Try to find this exact type already on the stack
    while (curType != NULL) {
      if (curType->parent == c.expType && curType->mod == TM_POINTER) {
        c.expType = curType;
        found = true;
        break;
      }
      curType = curType->next;
    }

    if (!found) {
      // Couldn't find it? Make it
      typeStackPush(&a->types, TK_COMP, NULL, TM_POINTER, c.expType);
      c.expType = a->types.tail;
    }

    break;

  case N_REF:
    if (c.expType->mod != TM_POINTER) {
      throwAnalyserError(a, n->sourceName, n->children.p[0].line, FUNC_NAME,
                         "Expected type wasn't a pointer, but got one");
    }
    c.expType = c.expType->parent;
    break;
  default:
    break;
  }

  if (n->children.p[1].kind == N_UNARY_VALUE) {
    type = analyseUnaryValue(a, c, n->children.p + 1);
  } else {
    type = analyseExpression(a, c, n->children.p + 1);
  }

  switch (n->children.p[0].kind) {
  case N_DEREF:
    if (type->mod != TM_POINTER) {
      throwAnalyserError(a, n->sourceName, n->children.p[0].line, FUNC_NAME,
                         "Can only deref pointers");
    }
    if (type == a->preDefs.VOIDPTR) {
      throwAnalyserError(a, n->sourceName, n->children.p[0].line, FUNC_NAME,
                         "Can't deref void ptr (nil)");
    }
    return type->parent;
  case N_DEC:
    if (type != a->preDefs.INT && type != a->preDefs.CHAR) {
      throwAnalyserError(a, n->sourceName, n->children.p[0].line, FUNC_NAME,
                         "Can only decrement ints or chars");
    }
    return type;
  case N_INC:
    if (type != a->preDefs.INT && type != a->preDefs.CHAR) {
      throwAnalyserError(a, n->sourceName, n->children.p[0].line, FUNC_NAME,
                         "Can only increment ints or chars");
    }
    return type;
  case N_NOT:
    if (type != a->preDefs.INT && type != a->preDefs.CHAR &&
        type != a->preDefs.BOOL) {
      throwAnalyserError(a, n->sourceName, n->children.p[0].line, FUNC_NAME,
                         "Can only not int, char, bool, or float");
    }
    return type;
  case N_REF:
    Type *curType = a->types.tail;

    // Try to find this exact type already on the stack
    while (curType != NULL) {
      if (curType->parent == type && curType->mod == TM_POINTER) {
        return curType;
      }
      curType = curType->next;
    }

    // Couldn't find it? Make it
    typeStackPush(&a->types, TK_COMP, NULL, TM_POINTER, type);

    return a->types.tail;

  case N_ADD:
    if (type != a->preDefs.INT && type != a->preDefs.CHAR) {
      throwAnalyserError(a, n->sourceName, n->children.p[0].line, FUNC_NAME,
                         "Can only positive int or char");
    }
    return type;
  case N_SUB:
    if (type != a->preDefs.INT && type != a->preDefs.CHAR) {
      throwAnalyserError(a, n->sourceName, n->children.p[0].line, FUNC_NAME,
                         "Can only negative int or char");
    }
    return type;
  default:
    throwAnalyserError(a, n->sourceName, n->children.p[0].line, FUNC_NAME,
                       "Unexpected unary");
  }
  return NULL;
}

Type *analyseFuncCall(Analyser *a, Context c, Node *n) {
  char FUNC_NAME[] = "analyseFuncCall";

  // printf("FuncCall %s\n", nodeCodeString(n->kind));

  Fun *fun = funExists(a, n->children.p[1].data);
  if (fun == NULL) {
    throwAnalyserError(a, n->sourceName, n->children.p[1].line, FUNC_NAME,
                       "Function doesn't exist");
  }

  int paramIndex = 0;
  int nodeIndex = 3;

  while (paramIndex < fun->paramsLen) {
    if (nodeIndex >= n->children.len) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Not enough args for function");
    }

    if (n->children.p[nodeIndex].kind != N_EXPRESSION) {
      throwAnalyserError(a, n->sourceName, n->children.p[nodeIndex].line,
                         FUNC_NAME, "Not enough args for function");
    }

    // Correct expected type
    c.expType = fun->params[paramIndex].type;
    analyseExpression(a, c, n->children.p + nodeIndex);

    ++paramIndex;
    nodeIndex += 2;
  }

  if (n->children.p[nodeIndex].kind == N_EXPRESSION) {
    throwAnalyserError(a, n->sourceName, n->children.p[nodeIndex].line,
                       FUNC_NAME, "Too many args for function");
  }

  return fun->ret;
}

Type *analyseStructNew(Analyser *a, Context c, Node *n) {
  char FUNC_NAME[] = "analyseStructNew";

  // printf("StructNew %s\n", nodeCodeString(n->kind));

  Type *stt = typeExists(a, n->children.p[1].data);
  if (stt == NULL) {
    throwAnalyserError(a, n->sourceName, n->children.p[1].line, FUNC_NAME,
                       "Struct doesn't exist");
  }
  if (stt->propsLen == 0) {
    throwAnalyserError(a, n->sourceName, n->children.p[1].line, FUNC_NAME,
                       "Type used in struct new must be struct");
  }

  int propIndex = 0;
  int nodeIndex = 3;

  while (propIndex < stt->propsLen) {
    if (nodeIndex >= n->children.len) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Not enough args for struct");
    }

    if (n->children.p[nodeIndex].kind != N_EXPRESSION) {
      throwAnalyserError(a, n->sourceName, n->children.p[nodeIndex].line,
                         FUNC_NAME, "Not enough args for struct");
    }

    // Correct expected type
    c.expType = stt->props[propIndex].type;
    analyseExpression(a, c, n->children.p + nodeIndex);

    ++propIndex;
    nodeIndex += 2;
  }

  if (n->children.p[nodeIndex].kind == N_EXPRESSION) {
    throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                       "Too many args for struct");
  }

  return stt;
}

Type *analyseMakeArray(Analyser *a, Context c, Node *n) {
  char FUNC_NAME[] = "analyseMakeArray";

  // printf("MakeArray %s\n", nodeCodeString(n->kind));

  Type *expType = NULL;
  Type *subType = NULL;

  if (c.expType == NULL) {
    // We don't have to worry about array, etc
  } else {
    if (c.expType->mod != TM_ARRAY) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Expected type wasn't array");
    }

    // Unwrap type from array
    expType = c.expType;
    subType = expType->parent;
  }

  int i = 2;

  Type *exprType = NULL;

  while (i < n->children.len) {
    if (n->children.p[i].kind != N_EXPRESSION) {
      break;
    }

    c.expType = subType;

    exprType = analyseExpression(a, c, n->children.p + i);

    if (subType == NULL) { // Expect that we get consistent typing
      subType = exprType;
    } else if (!typeEqual(subType, exprType)) {
      throwAnalyserError(a, n->sourceName, n->children.p[i].line, FUNC_NAME,
                         "Expected correct typing for elements of new array");
    }

    i += 2;
  }

  // We need to create the type if it doesn't exist
  if (expType == NULL) {
    Type *curType = a->types.tail;

    // Try to find this exact type already on the stack
    while (curType != NULL) {
      if (curType->parent == subType && curType->mod == TM_ARRAY) {
        expType = curType;
        break;
      }
      curType = curType->next;
    }

    // Couldn't find it?
    if (expType == NULL) {
      // Make it
      typeStackPush(&a->types, TK_COMP, NULL, TM_ARRAY, subType);
      expType = a->types.tail;
    }
  }

  // printf("Exit MakeArray %s\n", nodeCodeString(n->kind));

  return expType;
}

// analyseExpression also returns the type of the expression
Type *analyseExpression(Analyser *a, Context c, Node *n) {
  char FUNC_NAME[] = "analyseExpression";

  // printf("Expr %s\n", nodeCodeString(n->kind));

  Type *exprType = NULL;

  // Only one value
  if (n->children.len == 1) {
    // printf("Testing single element expression on line %i\n", n->line);
    // Unary or value?
    if (n->children.p[0].kind == N_UNARY_VALUE) {
      exprType = analyseUnaryValue(a, c, n->children.p);
    } else {
      exprType = analyseValue(a, c, n->children.p);
    }

    // Did we get the expected type?
    if (c.expType == NULL) { // Multiple types, caller will decide
      return exprType;
    } else if (typeEqual(c.expType, exprType)) { // We got the right type
      return exprType;
    } else { // bad type
      printf("\nExpected type %s\n", typeString(c.expType));
      printf("Recieved type %s\n\n", typeString(exprType));
      throwAnalyserError(a, n->sourceName, n->children.p[0].line, FUNC_NAME,
                         "Expression did not have the correct type");
    }
  }

  // Unary or value?
  if (n->children.p[0].kind == N_UNARY_VALUE) {
    exprType = analyseUnaryValue(a, c, n->children.p);
  } else {
    exprType = analyseValue(a, c, n->children.p);
  }

  // Check all the values are the same
  for (int i = 2; i < n->children.len; i += 2) {
    if (n->children.p[i].kind == N_UNARY_VALUE) {
      if (!typeEqual(analyseUnaryValue(a, c, n->children.p + i), exprType)) {
        throwAnalyserError(a, n->sourceName, n->children.p[i].line, FUNC_NAME,
                           "Unexpected type in expression");
      }
    } else {
      if (!typeEqual(analyseValue(a, c, n->children.p + i), exprType)) {
        throwAnalyserError(a, n->sourceName, n->children.p[i].line, FUNC_NAME,
                           "Unexpected type in expression");
      }
    }
  }

  // Check the operators make sense
  for (int i = 1; i < n->children.len - 1; i += 2) {
    analyseOperator(a, c, n->children.p + i, exprType, exprType);
  }

  return exprType;
}

#define CHECK_TYPE_EQUAL(operator)                                             \
  if (!typeEqual(left, right)) {                                               \
    throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,                   \
                       "Can't use " operator" operator on different types");   \
  }

#define CHECK_NO_ARRAYS(operator)                                              \
  if (left->mod == TM_ARRAY || right->mod == TM_ARRAY) {                       \
    throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,                   \
                       "Can't " operator" arrays");                            \
  }

#define CHECK_NO_STRUCTS(operator)                                             \
  if (left->propsLen > 0 || right->propsLen > 0) {                             \
    throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,                   \
                       "Can't " operator" structs");                           \
  }

#define ALLOW_POINTERS                                                         \
  if (left->mod == TM_POINTER && right->mod == TM_POINTER) {                   \
    return;                                                                    \
  }

#define DONT_ALLOW_POINTERS(operator)                                          \
  if (left->mod == TM_POINTER && right->mod == TM_POINTER) {                   \
    throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,                   \
                       "Can't " operator" pointers");                          \
  }

void analyseOperator(Analyser *a, Context c, Node *n, Type *left, Type *right) {
  char FUNC_NAME[] = "analyseOperator";

  switch (n->kind) {
  case N_EQ:
    CHECK_TYPE_EQUAL("equal")
    CHECK_NO_ARRAYS("equal")
    CHECK_NO_STRUCTS("equal")
    return;

  case N_NEQ:
    CHECK_TYPE_EQUAL("not equal")
    CHECK_NO_ARRAYS("not equal")
    CHECK_NO_STRUCTS("not equal")
    return;

  case N_GT:
    CHECK_TYPE_EQUAL("greater than")
    CHECK_NO_ARRAYS("greater than")
    CHECK_NO_STRUCTS("greater than")
    ALLOW_POINTERS

    // Can't compare bools in this way
    if (left == a->preDefs.BOOL || right == a->preDefs.BOOL) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't use less than operator on different types");
    }

    return;

  case N_GTEQ:
    CHECK_TYPE_EQUAL("greater than or equal")
    CHECK_NO_ARRAYS("greater than or equal")
    CHECK_NO_STRUCTS("greater than or equal")
    ALLOW_POINTERS

    // Can't compare bools in this way
    if (left == a->preDefs.BOOL || right == a->preDefs.BOOL) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't use less than operator on different types");
    }

    return;

  case N_LT:
    CHECK_TYPE_EQUAL("less than")
    CHECK_NO_ARRAYS("less than")
    CHECK_NO_STRUCTS("less than")
    ALLOW_POINTERS

    // Can't compare bools in this way
    if (left == a->preDefs.BOOL || right == a->preDefs.BOOL) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't use less than operator on different types");
    }

    return;

  case N_LTEQ:
    CHECK_TYPE_EQUAL("less than or equal")
    CHECK_NO_ARRAYS("less than or equal")
    CHECK_NO_STRUCTS("less than or equal")
    ALLOW_POINTERS

    // Can't compare bools in this way
    if (left == a->preDefs.BOOL || right == a->preDefs.BOOL) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't use less than operator on different types");
    }

    return;

  case N_L_SHIFT:
    CHECK_TYPE_EQUAL("left shift")
    CHECK_NO_ARRAYS("left shift")
    CHECK_NO_STRUCTS("left shift")
    DONT_ALLOW_POINTERS("left shift")

    if (left == a->preDefs.BOOL || right == a->preDefs.BOOL) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't left shift booleans");
    }

    if (left == a->preDefs.CHAR || right == a->preDefs.CHAR) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't left shift characters");
    }

    if (left == a->preDefs.FLOAT || right == a->preDefs.FLOAT) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't left shift floats");
    }

    return;

  case N_R_SHIFT:
    CHECK_TYPE_EQUAL("right shift")
    CHECK_NO_ARRAYS("right shift")
    CHECK_NO_STRUCTS("right shift")
    DONT_ALLOW_POINTERS("right shift")

    if (left == a->preDefs.BOOL || right == a->preDefs.BOOL) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't right shift booleans");
    }

    if (left == a->preDefs.CHAR || right == a->preDefs.CHAR) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't right shift characters");
    }

    if (left == a->preDefs.FLOAT || right == a->preDefs.FLOAT) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't right shift floats");
    }

    return;

  case N_ANDAND:
    CHECK_TYPE_EQUAL("and and")
    CHECK_NO_ARRAYS("and and")
    CHECK_NO_STRUCTS("and and")
    DONT_ALLOW_POINTERS("and and")

    if (left != a->preDefs.BOOL || right != a->preDefs.BOOL) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't and and anything other than booleans");
    }

    return;

  case N_OROR:
    CHECK_TYPE_EQUAL("or or")
    CHECK_NO_ARRAYS("or or")
    CHECK_NO_STRUCTS("or or")
    DONT_ALLOW_POINTERS("or or")

    if (left != a->preDefs.BOOL || right != a->preDefs.BOOL) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't or or anything other than booleans");
    }

    return;

  case N_AND:
    CHECK_TYPE_EQUAL("and")
    CHECK_NO_ARRAYS("and")
    CHECK_NO_STRUCTS("and")
    ALLOW_POINTERS

    if (left != a->preDefs.INT || right != a->preDefs.INT) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't and anything other than int");
    }

    return;

  case N_OR:
    CHECK_TYPE_EQUAL("or")
    CHECK_NO_ARRAYS("or")
    CHECK_NO_STRUCTS("or")
    ALLOW_POINTERS

    if (left != a->preDefs.INT || right != a->preDefs.INT) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't or anything other than int");
    }

    return;

  case N_XOR:
    CHECK_TYPE_EQUAL("xor")
    CHECK_NO_ARRAYS("xor")
    CHECK_NO_STRUCTS("xor")
    ALLOW_POINTERS

    if (left != a->preDefs.INT || right != a->preDefs.INT) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't xor anything other than int");
    }

    return;

  case N_ADD:
    CHECK_TYPE_EQUAL("add")
    CHECK_NO_ARRAYS("add")
    CHECK_NO_STRUCTS("add")
    ALLOW_POINTERS

    if ((left != a->preDefs.INT || right != a->preDefs.INT) &&
        (left != a->preDefs.FLOAT || right != a->preDefs.FLOAT)) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't add anything other than int or float");
    }

    return;

  case N_DIV:
    CHECK_TYPE_EQUAL("div")
    CHECK_NO_ARRAYS("div")
    CHECK_NO_STRUCTS("div")
    ALLOW_POINTERS

    if ((left != a->preDefs.INT || right != a->preDefs.INT) &&
        (left != a->preDefs.FLOAT || right != a->preDefs.FLOAT)) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't div anything other than int or float");
    }

    return;

  case N_MOD:
    CHECK_TYPE_EQUAL("mod")
    CHECK_NO_ARRAYS("mod")
    CHECK_NO_STRUCTS("mod")
    ALLOW_POINTERS

    if ((left != a->preDefs.INT || right != a->preDefs.INT) &&
        (left != a->preDefs.FLOAT || right != a->preDefs.FLOAT)) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't mod anything other than int or float");
    }

    return;

  case N_MUL:
    CHECK_TYPE_EQUAL("mul")
    CHECK_NO_ARRAYS("mul")
    CHECK_NO_STRUCTS("mul")
    ALLOW_POINTERS

    if ((left != a->preDefs.INT || right != a->preDefs.INT) &&
        (left != a->preDefs.FLOAT || right != a->preDefs.FLOAT)) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't mul anything other than int or float");
    }

    return;

  case N_SUB:
    CHECK_TYPE_EQUAL("sub")
    CHECK_NO_ARRAYS("sub")
    CHECK_NO_STRUCTS("sub")
    ALLOW_POINTERS

    if ((left != a->preDefs.INT || right != a->preDefs.INT) &&
        (left != a->preDefs.FLOAT || right != a->preDefs.FLOAT)) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Can't sub anything other than int or float");
    }

    return;

  default:
    throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                       "This shouldn't occur, invalid operator");
  }
}

Type *analyseComplexType(Analyser *a, Context c, Node *n) {
  char FUNC_NAME[] = "analyseComplexType";

  // printf("ComplexType %s\n", nodeCodeString(n->kind));

  // Base level type
  if (n->kind == N_IDENTIFIER) {
    Type *t = typeExists(a, n->data);
    if (t == NULL) {
      throwAnalyserError(a, n->sourceName, n->line, FUNC_NAME,
                         "Couldn't find type");
    }

    return t;
  }

  TypeModifier mod = TM_NONE;

  if (n->children.p[0].kind == N_INDEX) {
    analyseIndex(a, c, n->children.p);
    mod = TM_ARRAY;
  } else {
    mod = TM_POINTER;
  }

  Type *t = analyseComplexType(a, c, n->children.p + 1);

  Type *curType = a->types.tail;

  Type *finalType = NULL;

  // Try to find this exact type already on the stack
  while (curType != NULL) {
    if ((curType->parent == t) && (curType->mod == mod)) {
      finalType = curType;
      break;
    }
    curType = curType->next;
  }

  // Couldn't find it?
  if (finalType == NULL) {
    // Make it
    typeStackPush(&a->types, TK_COMP, NULL, mod, t);
    finalType = a->types.tail;
  }

  return finalType;
}

void analyseBlock(Analyser *a, Context c, Node *n) {
  char FUNC_NAME[] = "analyseBlock";

  // printf("Block %s\n", nodeCodeString(n->kind));

  for (int i = 1; i < n->children.len - 1; ++i) {
    // printf("Statement %s\n", nodeCodeString(n->children.p[i].kind));
    switch (n->children.p[i].kind) {
    case N_LONE_CALL:
      analyseLoneCall(a, c, n->children.p + i);
      break;
    case N_VAR_DEC:
      analyseVarDeclaration(a, c, n->children.p + i);
      break;
    case N_IF_BLOCK:
      analyseIfBlock(a, c, n->children.p + i);
      break;
    case N_FOR_LOOP:
      analyseForLoop(a, c, n->children.p + i);
      break;
    case N_RET_STATE:
      analyseRetState(a, c, n->children.p + i);
      break;
    case N_BREAK_STATE:
      analyseBreakState(a, c, n->children.p + i);
      break;
    case N_CONTINUE_STATE:
      analyseContinueState(a, c, n->children.p + i);
      break;
    case N_SWITCH_STATE:
      analyseSwitchState(a, c, n->children.p + i);
      break;

    default:
      throwAnalyserError(a, n->sourceName, n->children.p[i].line, FUNC_NAME,
                         "Invalid statement in block");
    }
  }
}

void analyse(Analyser *a) {
  analyseEnums(a);
  printf("Analysed enums\n");
  analyseStructs(a);
  printf("Analysed structs\n");
  analyseFuncs(a);
  printf("Analysed funs\n");

  identStackClear(&a->vars);
  funStackClear(&a->funs);
  typeStackClear(&a->types);
  printf("Cleared stacks\n");
}
