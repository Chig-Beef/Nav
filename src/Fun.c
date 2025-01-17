#include "Fun.h"
#include "Panic.h"
#include "String.h"
#include <stdlib.h>

void funStackPush(FunStack *s, String *name) {
  Fun *n = calloc(1, sizeof(Fun));
  if (n == NULL) {
    panic("Couldn't allocated node for stack");
  }

  n->name = name;

  // This will usually be changed by the user after if needed
  n->ret = NULL;
  n->params = NULL;
  n->paramsLen = 0;

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
Fun funStackPop(FunStack *s) {
  if (s->len == 0) {
    return ZERO_FUN;
  }

  --s->len;

  if (s->len == 0) {
    Fun tail = *(s->tail);
    free(s->tail);
    s->tail = NULL;
    return tail;
  }

  Fun tail = *(s->tail);
  free(s->tail);
  s->tail = s->tail->next;
  return tail;
}

void funStackClear(FunStack *s) {
  while (s->len > 0) {
    funStackPop(s);
  }
}
