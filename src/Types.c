#include "Types.h"
#include "Panic.h"
#include "TypeModifier.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>

NEW_LIST_TYPE_HEADER(char, Char)

char *typeString(Type *t) {
  CharList out;
  if (CharListInit(&out, 1)) {
    panic("Couldn't initialise charlist");
  }

  Type *cur = t;

  while (cur != NULL) {
    switch (cur->mod) {
    case TM_POINTER:
      if (CharListAppend(&out, '^')) {
        panic("Couldn't append to charlist");
      }
      break;

    case TM_ARRAY:
      if (CharListAppend(&out, '[')) {
        panic("Couldn't append to charlist");
      }
      if (CharListAppend(&out, ']')) {
        panic("Couldn't append to charlist");
      }
      break;

    case TM_NONE:
      String *name = cur->name;

      // Null case
      if (name == NULL) {
        if (CharListAppend(&out, 'N')) {
          panic("Couldn't append to charlist");
        }
        if (CharListAppend(&out, 'U')) {
          panic("Couldn't append to charlist");
        }
        if (CharListAppend(&out, 'L')) {
          panic("Couldn't append to charlist");
        }
        if (CharListAppend(&out, 'L')) {
          panic("Couldn't append to charlist");
        }

        break;
      }

      int i = 0;

      while (name->data[i]) {
        if (CharListAppend(&out, name->data[i])) {
          panic("Couldn't append to charlist");
        }
        ++i;
      }

      break;
    }

    // printf("%s\n", out.p);
    cur = cur->parent;
  }

  return out.p;
}

void typeStackPush(TypeStack *s, TypeKind kind, String *name, TypeModifier mod,
                   Type *parent) {

  Type *n = calloc(1, sizeof(Type));
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
