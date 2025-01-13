#include "Panic.h"
#include "String.h"
#include <stdlib.h>

typedef enum TypeModifier {
  TM_NONE,
  TM_ARRAY,
  TM_POINTER,
} TypeModifier;

typedef struct Ident Ident;

typedef struct Ident {
  String name;
  Ident *type;      // A type of NULL means that this is a type
  Ident *next;      // Linked list structure (stack)
  TypeModifier mod; // Is this a pointer of a type

  Ident *ret; // If this is of type fun, what this function returns

  Ident **params; // An array of params, used for functions or structs
} Ident;

// Variables are held in a stack structure
typedef struct Stack {
  Ident *tail;
  int len;
} Stack;

void stackPush(Stack *s, String name, Ident *type, TypeModifier mod, Ident *ret,
               Ident **params) {

  Ident *n = malloc(sizeof(Ident));
  if (n == NULL) {
    panic("Couldn't allocated node for stack");
  }

  n->name = name;
  n->type = type;
  n->mod = mod;
  n->ret = ret;
  n->params = params;

  ++s->len;

  if (s->len == 1) {
    s->tail = n;
    return;
  }

  n->next = s->tail;
  s->tail = n;
}
