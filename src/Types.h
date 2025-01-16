#pragma once

typedef struct Type Type;

#include "Ident.h"
#include "String.h"
#define ZERO_TYPE (Type){NULL, NULL, NULL, 0}

typedef struct Type {
  String *name;
  Type *next; // Linked list structure (stack)

  Ident *props; // An array of props, used for structs
  int propsLen;
} Type;

// Variables are held in a stack structure
typedef struct TypeStack {
  Type *tail;
  int len;
} TypeStack;

void typeStackPush(TypeStack *s, String *name);

// The reason we return the thing and not the pointer to the thing is because we
// free the pointer, so we're returning a copy
Type typeStackPop(TypeStack *s);

void typeStackClear(TypeStack *s);
