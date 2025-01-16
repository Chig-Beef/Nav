#pragma once

#include "Types.h"

typedef struct Context {
  bool canBreak;
  bool canCont;
  Type *expType; // What the caller expects this things type to be
  Type *retType; // What the function we're in expects the return type to be
} Context;
