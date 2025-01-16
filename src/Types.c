#include "Types.h"
#include "Panic.h"
#include <stdlib.h>

TypeKind kind;

void typeStackPush(TypeStack *s, TypeKind kind, String *name, TypeModifier mod,
                   Type *parent) {

  Type *n = malloc(sizeof(Type));
  if (n == NULL) {
    panic("Couldn't allocated node for stack");
  }

  n->kind = kind;
  n->name = name;
  n->mod = mod;
  n->parent = parent;

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
Type typeStackPop(TypeStack *s) {
  if (s->len == 0) {
    return ZERO_TYPE;
  }

  --s->len;

  if (s->len == 0) {
    Type tail = *(s->tail);
    free(s->tail);
    s->tail = NULL;
    return tail;
  }

  Type tail = *(s->tail);
  free(s->tail);
  s->tail = s->tail->next;
  return tail;
}

void typeStackClear(TypeStack *s) {
  while (s->len > 0) {
    typeStackPop(s);
  }
}
