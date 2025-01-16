#pragma once

typedef struct Ident Ident;

#include "String.h"
#include "TypeModifier.h"
#include "Types.h"

#define ZERO_IDENT                                                             \
  (Ident) { NULL, NULL, NULL, TM_NONE }

typedef struct Ident {
  String *name;
  Type *type;       // A type of NULL means that this is a type
  Ident *next;      // Linked list structure (stack)
  TypeModifier mod; // Is this a pointer of a type
} Ident;

// Variables are held in a stack structure
typedef struct IdentStack {
  Ident *tail;
  int len;
} IdentStack;

Ident identStackPop(IdentStack *s);

void identStackPush(IdentStack *s, String *name, Type *type, TypeModifier mod);

void identStackClear(IdentStack *s);
