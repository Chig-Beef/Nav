#include "Panic.h"
#include "String.h"
#include <stdlib.h>

#define ZERO_IDENT                                                             \
  (Ident) { NULL, NULL, NULL, TM_NONE, NULL, NULL, NULL }

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

void stackPush(Stack *s, String *name, Ident *type, TypeModifier mod) {
  Ident *n = malloc(sizeof(Ident));
  if (n == NULL) {
    panic("Couldn't allocated node for stack");
  }

  n->name = name;
  n->type = type;
  n->mod = mod;

  // This will usually be changed by the user after if needed
  n->ret = NULL;
  n->params = NULL;
  n->paramsLen = 0;
  n->props = NULL;
  n->propsLen = 0;

  ++s->len;

  if (s->len == 1) {
    s->tail = n;
    return;
  }

  n->next = s->tail;
  s->tail = n;
}

// The reason we return the thing and not the pointer to the thing is because we
// free the pointer, so we're returning a copy
Ident stackPop(Stack *s) {
  if (s->len == 0) {
    return ZERO_IDENT;
  }

  --s->len;

  if (s->len == 0) {
    Ident tail = *(s->tail);
    free(s->tail);
    s->tail = NULL;
    return tail;
  }

  Ident tail = *(s->tail);
  free(s->tail);
  s->tail = s->tail->next;
  return tail;
}

void stackClear(Stack *s) {
  while (s->len > 0) {
    stackPop(s);
  }
}
