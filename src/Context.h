#pragma once

#include "Ident.h"

typedef struct Context {
  bool inLoop;
  Ident *exprType;
} Context;
