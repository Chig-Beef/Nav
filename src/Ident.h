#pragma once

#include "String.h"
#include "TypeModifier.h"

#define ZERO_IDENT (Ident){NULL, NULL, NULL, TM_NONE, NULL, 0}

typedef struct Ident Ident;

typedef struct Ident {
  String *name;
  Ident *type;      // A type of NULL means that this is a type
  Ident *next;      // Linked list structure (stack)
  TypeModifier mod; // Is this a pointer of a type

  Ident *props; // An array of props, used for structs
  int propsLen;
} Ident;

// Variables are held in a stack structure
typedef struct IdentStack {
  Ident *tail;
  int len;
} IdentStack;

Ident identStackPop(IdentStack *s);

void identStackPush(IdentStack *s, String *name, Ident *type, TypeModifier mod);

void identStackClear(IdentStack *s);
