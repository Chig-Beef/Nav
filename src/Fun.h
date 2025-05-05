#pragma once

#include "Ident.h"

#define ZERO_FUN (Fun){NULL, NULL, NULL, NULL, 0}

typedef struct Fun Fun;

typedef struct Fun {
  char *name;
  Fun *next; // Linked list structure (stack)

  Type *ret;     // If this is of type fun, what this function returns
  Ident *params; // An array of params, used for functions
  int paramsLen;
} Fun;

// Variables are held in a stack structure
typedef struct FunStack {
  Fun *tail;
  int len;
} FunStack;

Fun funStackPop(FunStack *s);

void funStackPush(FunStack *s, char *name);

void funStackClear(FunStack *s);
