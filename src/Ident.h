#pragma once

#include "String.h"

#define ZERO_IDENT                                                             \
  (Ident) { NULL, NULL, NULL, TM_NONE, NULL, NULL }

typedef enum TypeModifier {
  TM_NONE,
  TM_ARRAY,
  TM_POINTER,
} TypeModifier;

typedef struct Ident Ident;

typedef struct Ident {
  String *name;
  Ident *type;      // A type of NULL means that this is a type
  Ident *next;      // Linked list structure (stack)
  TypeModifier mod; // Is this a pointer of a type

  Ident *ret;    // If this is of type fun, what this function returns
  Ident *params; // An array of params, used for functions
  int paramsLen;

  Ident *props; // An array of props, used for structs
  int propsLen;
} Ident;

// Variables are held in a stack structure
typedef struct Stack {
  Ident *tail;
  int len;
} Stack;

Ident stackPop(Stack *s);

void stackPush(Stack *s, String *name, Ident *type, TypeModifier mod);

void stackClear(Stack *s);
