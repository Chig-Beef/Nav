#include "Ident.h"
#include <stdbool.h>

typedef struct Context {
  bool inLoop;
  Ident *exprType;
} Context;
