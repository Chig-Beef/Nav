#pragma once

#include "Fun.h"
#include "Node.h"
#include "StringManager.h"
#include "Types.h"

#include "stdio.h"

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

  StringManager *sm;

} Analyser;

void analyserInit(Analyser *a, Node enums, Node structs, Node funcs,
                  StringManager *sm);

void analyse(Analyser *a);
