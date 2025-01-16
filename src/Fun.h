#pragma once

#include "Ident.h"
#include "String.h"

#define ZERO_FUN (Fun){NULL, NULL, NULL, NULL, 0}

typedef struct Fun Fun;

typedef struct Fun {
  String *name;
  Fun *next; // Linked list structure (stack)

  Ident *ret;    // If this is of type fun, what this function returns
  Ident *params; // An array of params, used for functions
  int paramsLen;
} Fun;

// Variables are held in a stack structure
typedef struct FunStack {
  Fun *tail;
  int len;
} FunStack;

Fun funStackPop(FunStack *s);

void funStackPush(FunStack *s, String *name);

void funStackClear(FunStack *s);
