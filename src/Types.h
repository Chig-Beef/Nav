#pragma once

typedef struct Type Type;

#include "Ident.h"

#define ZERO_TYPE                                                              \
  (Type) { TK_ABS, NULL, NULL, 0, TM_NONE, NULL, NULL }

typedef enum TypeKind {
  TK_ABS,
  TK_COMP,
} TypeKind;

typedef struct Type {
  TypeKind kind;

  // Absolute type properties
  char *name;
  Ident *props; // An array of props, used for structs
  int propsLen;

  // Complex type properties
  TypeModifier mod;
  Type *parent;

  // Stack based
  Type *next;
} Type;

// Variables are held in a stack structure
typedef struct TypeStack {
  Type *tail;
  int len;
} TypeStack;

void typeStackPush(TypeStack *s, TypeKind kind, char *name, TypeModifier mod,
                   Type *parent);

// The reason we return the thing and not the pointer to the thing is because we
// free the pointer, so we're returning a copy
Type typeStackPop(TypeStack *s);

void typeStackClear(TypeStack *s);

char *typeString(Type *t);
