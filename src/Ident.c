#include "Ident.h"
#include "Panic.h"
#include "String.h"
#include <stdlib.h>

void identStackPush(IdentStack *s, String *name, Type *type) {
  Ident *n = calloc(1, sizeof(Ident));
  if (n == NULL) {
    panic("Couldn't allocated node for stack");
  }

  n->name = name;
  n->type = type;

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
Ident identStackPop(IdentStack *s) {
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

void identStackClear(IdentStack *s) {
  while (s->len > 0) {
    identStackPop(s);
  }
}
