#include "Types.h"
#include "Panic.h"
#include "String.h"
#include <stdlib.h>

void typeStackPush(TypeStack *s, String *name) {
  Type *n = malloc(sizeof(Type));
  if (n == NULL) {
    panic("Couldn't allocated node for stack");
  }

  n->name = name;

  // This will usually be changed by the user after if needed
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
