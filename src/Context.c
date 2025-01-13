#include <stdbool.h>

typedef struct Context {
  bool inLoop;
  bool inFun;
  bool exprType;
} Context;
