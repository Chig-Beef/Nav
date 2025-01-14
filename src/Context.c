#include "Ident.h"
#include <stdbool.h>

typedef struct Context {
  bool canBreak;
  bool canCont;
  Ident *expType; // What the caller expects this things type to be
  Ident *retType; // What the function we're in expects the return type to be
} Context;
