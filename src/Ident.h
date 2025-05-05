#pragma once

typedef struct Ident Ident;

#include "TypeModifier.h"
#include "Types.h"

#define ZERO_IDENT                                                             \
  (Ident) { NULL, NULL, NULL }

typedef struct Ident {
  char *name;
  Type *type;  // A type of NULL means that this is a type
  Ident *next; // Linked list structure (stack)
} Ident;

// Variables are held in a stack structure
typedef struct IdentStack {
  Ident *tail;
  int len;
} IdentStack;

Ident identStackPop(IdentStack *s);

void identStackPush(IdentStack *s, char *name, Type *type);

void identStackClear(IdentStack *s);
