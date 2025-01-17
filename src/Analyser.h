#pragma once

#include "Fun.h"
#include "Node.h"
#include "Types.h"
#include "stdio.h"

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

void analyserInit(Analyser *a, Node enums, Node structs, Node funcs);

void analyse(Analyser *a);
